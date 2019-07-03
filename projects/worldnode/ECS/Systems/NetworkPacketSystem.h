/*
    MIT License

    Copyright (c) 2018-2019 NovusCore

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
#include <entt.hpp>
#include <NovusTypes.h>
#include <Networking/Opcode/Opcode.h>
#include <Utils/DebugHandler.h>
#include <Utils/AtomicLock.h>
#include <Math/Math.h>
#include <Math/Vector2.h>
#include <Cryptography/HMAC.h>
#include <zlib.h>

#include "../../NovusEnums.h"
#include "../../Utils/CharacterUtils.h"
#include "../../DatabaseCache/CharacterDatabaseCache.h"
#include "../../DatabaseCache/DBCDatabaseCache.h"
#include "../../WorldNodeHandler.h"
#include "../../Scripting/PlayerFunctions.h"

#include "../Components/PlayerConnectionComponent.h"
#include "../Components/PlayerFieldDataComponent.h"
#include "../Components/PlayerUpdateDataComponent.h"
#include "../Components/PlayerPositionComponent.h"

#include "../Components/Singletons/SingletonComponent.h"
#include "../Components/Singletons/PlayerDeleteQueueSingleton.h"
#include "../Components/Singletons/CharacterDatabaseCacheSingleton.h"
#include "../Components/Singletons/WorldDatabaseCacheSingleton.h"
#include "../Components/Singletons/DBCDatabaseCacheSingleton.h"
#include "../Components/Singletons/PlayerPacketQueueSingleton.h"
#include "../Components/Singletons/MapSingleton.h"

#include <tracy/Tracy.hpp>

namespace ConnectionSystem
{
    void Update(entt::registry& registry)
    {
        SingletonComponent& singleton = registry.ctx<SingletonComponent>();
        PlayerDeleteQueueSingleton& playerDeleteQueue = registry.ctx<PlayerDeleteQueueSingleton>();
        CharacterDatabaseCacheSingleton& characterDatabase = registry.ctx<CharacterDatabaseCacheSingleton>();
        WorldDatabaseCacheSingleton& worldDatabase = registry.ctx<WorldDatabaseCacheSingleton>();
		DBCDatabaseCacheSingleton& dbcDatabase = registry.ctx<DBCDatabaseCacheSingleton>();
        PlayerPacketQueueSingleton& playerPacketQueue = registry.ctx<PlayerPacketQueueSingleton>();
        WorldNodeHandler& worldNodeHandler = *singleton.worldNodeHandler;
        MapSingleton& mapSingleton = registry.ctx<MapSingleton>();

        LockRead(SingletonComponent);
        LockRead(PlayerDeleteQueueSingleton);
        LockRead(CharacterDatabaseCacheSingleton);

        LockWrite(PlayerConnectionComponent);
        LockWrite(PlayerFieldDataComponent);
        LockWrite(PlayerUpdateDataComponent);
        LockWrite(PlayerPositionComponent);

        auto view = registry.view<PlayerConnectionComponent, PlayerFieldDataComponent, PlayerUpdateDataComponent, PlayerPositionComponent>();
        view.each([&registry, &singleton, &playerDeleteQueue, &characterDatabase, &worldDatabase, &dbcDatabase, &playerPacketQueue, &worldNodeHandler, &mapSingleton](const auto, PlayerConnectionComponent& playerConnection, PlayerFieldDataComponent& clientFieldData, PlayerUpdateDataComponent& playerUpdateData, PlayerPositionComponent& playerPositionData)
            {
                ZoneScopedNC("Connection", tracy::Color::Orange2)

                    for (NetPacket& packet : playerConnection.packets)
                    {
                        ZoneScopedNC("Packet", tracy::Color::Orange2)

                            Opcode opcode = static_cast<Opcode>(packet.opcode);
                        switch (opcode)
                        {
                        case Opcode::CMSG_SET_ACTIVE_MOVER:
                        {
                            ZoneScopedNC("Packet::SetActiveMover", tracy::Color::Orange2)

                                std::shared_ptr<DataStore> timeSync = DataStore::Borrow<4>();
                            timeSync->PutU32(0);

                            playerConnection.socket->SendPacket(timeSync.get(), Opcode::SMSG_TIME_SYNC_REQ);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_LOGOUT_REQUEST:
                        {
                            ZoneScopedNC("Packet::LogoutRequest", tracy::Color::Orange2)

                                ExpiredPlayerData expiredPlayerData;
                            expiredPlayerData.entityId = playerConnection.entityId;
                            expiredPlayerData.accountGuid = playerConnection.accountGuid;
                            expiredPlayerData.characterGuid = playerConnection.characterGuid;
                            playerDeleteQueue.expiredEntityQueue->enqueue(expiredPlayerData);

                            CharacterInfo characterInfo;
                            characterDatabase.cache->GetCharacterInfo(playerConnection.characterGuid, characterInfo);

                            characterInfo.level = clientFieldData.GetFieldValue<u32>(UNIT_FIELD_LEVEL);
                            characterInfo.mapId = playerPositionData.mapId;
                            characterInfo.position = playerPositionData.position;
                            characterInfo.orientation = playerPositionData.orientation;
                            characterInfo.online = 0;
                            characterInfo.UpdateCache(playerConnection.characterGuid);

                            characterDatabase.cache->SaveAndUnloadCharacter(playerConnection.characterGuid);

                            // Here we need to Redirect the client back to Realmserver. The Realmserver will send SMSG_LOGOUT_COMPLETE
                            std::shared_ptr<DataStore> buffer = DataStore::Borrow<30>();
                            i32 ip = 16777343;
                            i16 port = 8001;

                            // 127.0.0.1/1.0.0.127
                            // 2130706433/16777343(https://www.browserling.com/tools/ip-to-dec)
                            buffer->PutI32(ip);
                            buffer->PutI16(port);
                            buffer->PutI32(0); // unk
#pragma warning(push)
#pragma warning(disable: 4312)
                            HMACH hmac(40, playerConnection.socket->sessionKey.BN2BinArray(20).get());
                            hmac.UpdateHash((u8*)& ip, 4);
                            hmac.UpdateHash((u8*)& port, 2);
                            hmac.Finish();
                            buffer->PutBytes(hmac.GetData(), 20);
#pragma warning(pop)
                            playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_REDIRECT_CLIENT);

                            buffer->Reset();
                            buffer->PutU32(1);
                            playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_SUSPEND_COMMS);

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_READY_FOR_ACCOUNT_DATA_TIMES:
                        {
                            /* Packet Structure */
                            // UInt32:  Server Time (time(nullptr))
                            // UInt8:   Unknown Byte Value
                            // UInt32:  Mask for the account data fields

                            std::shared_ptr<DataStore> accountDataTimes = DataStore::Borrow<41>();

                            u32 mask = 0x15;
                            accountDataTimes->PutU32(static_cast<u32>(time(nullptr)));
                            accountDataTimes->PutU8(1); // bitmask blocks count
                            accountDataTimes->PutU32(mask);

                            for (u32 i = 0; i < 8; ++i)
                            {
                                if (mask & (1 << i))
                                {
                                    CharacterData characterData;
                                    if (characterDatabase.cache->GetCharacterData(playerConnection.characterGuid, i, characterData))
                                    {
                                        accountDataTimes->PutU32(characterData.timestamp);
                                    }
                                }
                            }

                            playerConnection.socket->SendPacket(accountDataTimes.get(), Opcode::SMSG_ACCOUNT_DATA_TIMES);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_UPDATE_ACCOUNT_DATA:
                        {
                            packet.handled = true;

                            u32 type, timestamp, decompressedSize;
                            packet.data->GetU32(type);
                            packet.data->GetU32(timestamp);
                            packet.data->GetU32(decompressedSize);

                            if (type > 8)
                            {
                                break;
                            }

                            bool characterDataUpdate = ((1 << type) & CHARACTER_DATA_CACHE_MASK);

                            // This is here temporarily as I'm not certain if the client will UPDATE any AccountData while connected to a WorldNode
                            if (!characterDataUpdate)
                            {
                                NC_LOG_WARNING("Received AccountDataUpdate");
                                break;
                            }

                            // Clear Data
                            if (decompressedSize == 0)
                            {
                                if (characterDataUpdate)
                                {
                                    CharacterData characterData;
                                    if (characterDatabase.cache->GetCharacterData(playerConnection.characterGuid, type, characterData))
                                    {
                                        characterData.timestamp = 0;
                                        characterData.data = "";
                                        characterData.UpdateCache();
                                    }
                                }
                            }
                            else
                            {
                                if (decompressedSize > 0xFFFF)
                                {
                                    break;
                                }


                                std::shared_ptr<DataStore> DataInfo = DataStore::Borrow<1024>();
                                DataInfo->Size = packet.data->Size - packet.data->ReadData;
                                DataInfo->PutBytes(packet.data->GetInternalData() + packet.data->ReadData, DataInfo->Size);

                                uLongf uSize = decompressedSize;
                                u32 pos = static_cast<u32>(DataInfo->ReadData);

                                std::shared_ptr<DataStore> dataInfo = DataStore::Borrow<8192>();
                                if (uncompress(dataInfo->GetInternalData(), &uSize, DataInfo->GetInternalData() + pos, static_cast<uLong>(DataInfo->Size - pos)) != Z_OK)
                                {
                                    break;
                                }

                                std::string finalData = "";
                                dataInfo->GetString(finalData);

                                if (characterDataUpdate)
                                {
                                    CharacterData characterData;
                                    if (characterDatabase.cache->GetCharacterData(playerConnection.characterGuid, type, characterData))
                                    {
                                        characterData.timestamp = timestamp;
                                        characterData.data = finalData;
                                        characterData.UpdateCache();
                                    }
                                }
                            }

                            std::shared_ptr<DataStore> updateAccountDataComplete = DataStore::Borrow<8>();
                            updateAccountDataComplete->PutU32(type);
                            updateAccountDataComplete->PutU32(0);

                            playerConnection.socket->SendPacket(updateAccountDataComplete.get(), Opcode::SMSG_UPDATE_ACCOUNT_DATA_COMPLETE);
                            break;
                        }
                        case Opcode::MSG_MOVE_SET_ALL_SPEED_CHEAT:
                        {
                            f32 speed = 1;
                            packet.data->GetF32(speed);

                            std::shared_ptr<DataStore> speedChange = DataStore::Borrow<12>();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_WALK_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_WALK_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_RUN_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_RUN_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_RUN_BACK_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_RUN_BACK_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_SWIM_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_SWIM_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_SWIM_BACK_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_SWIM_BACK_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_FLIGHT_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_FLIGHT_SPEED_CHANGE);
                            speedChange->Reset();

                            CharacterUtils::BuildSpeedChangePacket(playerConnection.characterGuid, speed, Opcode::SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE, speedChange);
                            playerConnection.socket->SendPacket(speedChange.get(), Opcode::SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE);

                            playerConnection.SendChatNotification("Speed Updated: %f", speed);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_MOVE_START_SWIM_CHEAT:
                        {
                            std::shared_ptr<DataStore> flyMode = DataStore::Borrow<12>();
                            CharacterUtils::BuildFlyModePacket(playerConnection.characterGuid, flyMode);

                            playerConnection.socket->SendPacket(flyMode.get(), Opcode::SMSG_MOVE_SET_CAN_FLY);

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_MOVE_STOP_SWIM_CHEAT:
                        {
                            std::shared_ptr<DataStore> flyMode = DataStore::Borrow<12>();
                            CharacterUtils::BuildFlyModePacket(playerConnection.characterGuid, flyMode);

                            playerConnection.socket->SendPacket(flyMode.get(), Opcode::SMSG_MOVE_UNSET_CAN_FLY);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_QUERY_OBJECT_POSITION:
                        {
                            packet.handled = true;

                            std::shared_ptr<DataStore> objectPosition = DataStore::Borrow<12>();
                            objectPosition->Put<Vector3>(playerPositionData.position);

                            playerPacketQueue.packetQueue->enqueue(PacketQueueData(playerConnection.socket, objectPosition, Opcode::SMSG_QUERY_OBJECT_POSITION));
                            break;
                        }
                        case Opcode::CMSG_LEVEL_CHEAT:
                        {
                            packet.handled = true;
                            u32 level = 0;
                            packet.data->GetU32(level);

                            if (level != clientFieldData.GetFieldValue<u32>(UNIT_FIELD_LEVEL))
                                clientFieldData.SetFieldValue<u32>(UNIT_FIELD_LEVEL, level);
                            break;
                        }
                        case Opcode::CMSG_STANDSTATECHANGE:
                        {
                            ZoneScopedNC("Packet::StandStateChange", tracy::Color::Orange2)

                                u32 standState = 0;
                            packet.data->GetU32(standState);

                            clientFieldData.SetFieldValue<u8>(UNIT_FIELD_BYTES_1, static_cast<u8>(standState));

                            std::shared_ptr<DataStore> standStateChange = DataStore::Borrow<1>();
                            standStateChange->PutU8(static_cast<u8>(standState));

                            playerPacketQueue.packetQueue->enqueue(PacketQueueData(playerConnection.socket, standStateChange, Opcode::SMSG_STANDSTATE_UPDATE));

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_SET_SELECTION:
                        {
                            ZoneScopedNC("Packet::SetSelection", tracy::Color::Orange2)

                                u64 selectedGuid = 0;
                            packet.data->GetGuid(selectedGuid);

                            clientFieldData.SetGuidValue(UNIT_FIELD_TARGET, selectedGuid);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_NAME_QUERY:
                        {
                            ZoneScopedNC("Packet::NameQuery", tracy::Color::Orange2)

                                u64 guid;
                            packet.data->GetU64(guid);

                            std::shared_ptr<DataStore> nameQuery = DataStore::Borrow<30>();
                            nameQuery->PutGuid(guid);

                            CharacterInfo characterInfo;
                            if (characterDatabase.cache->GetCharacterInfo(guid, characterInfo))
                            {
                                nameQuery->PutU8(0); // Name Unknown (0 = false, 1 = true);
                                nameQuery->PutString(characterInfo.name);
                                nameQuery->PutU8(0);
                                nameQuery->PutU8(characterInfo.race);
                                nameQuery->PutU8(characterInfo.gender);
                                nameQuery->PutU8(characterInfo.classId);
                            }
                            else
                            {
                                nameQuery->PutU8(1); // Name Unknown (0 = false, 1 = true);
                                nameQuery->PutString("Unknown");
                                nameQuery->PutU8(0);
                                nameQuery->PutU8(0);
                                nameQuery->PutU8(0);
                                nameQuery->PutU8(0);
                            }
                            nameQuery->PutU8(0);

                            playerConnection.socket->SendPacket(nameQuery.get(), Opcode::SMSG_NAME_QUERY_RESPONSE);
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_ITEM_QUERY_SINGLE:
                        {
                            u32 itemEntry;
                            packet.data->GetU32(itemEntry);
                            std::shared_ptr<DataStore> itemQuery;

                            ItemTemplate itemTemplate;
                            if (!worldDatabase.cache->GetItemTemplate(itemEntry, itemTemplate))
                            {
                                itemQuery = DataStore::Borrow<4>();
                                itemQuery->PutU32(itemEntry | 0x80000000);
                                playerConnection.socket->SendPacket(itemQuery.get(), Opcode::SMSG_ITEM_QUERY_SINGLE_RESPONSE);
                            }
                            else
                            {
                                playerConnection.socket->SendPacket(itemTemplate.GetQuerySinglePacket().get(), Opcode::SMSG_ITEM_QUERY_SINGLE_RESPONSE);
                            }

                            packet.handled = true;
                            break;
                        }
                        /* These packets should be read here, but preferbly handled elsewhere */
                        case Opcode::MSG_MOVE_STOP:
                        case Opcode::MSG_MOVE_STOP_STRAFE:
                        case Opcode::MSG_MOVE_STOP_TURN:
                        case Opcode::MSG_MOVE_STOP_PITCH:
                        case Opcode::MSG_MOVE_START_FORWARD:
                        case Opcode::MSG_MOVE_START_BACKWARD:
                        case Opcode::MSG_MOVE_START_STRAFE_LEFT:
                        case Opcode::MSG_MOVE_START_STRAFE_RIGHT:
                        case Opcode::MSG_MOVE_START_TURN_LEFT:
                        case Opcode::MSG_MOVE_START_TURN_RIGHT:
                        case Opcode::MSG_MOVE_START_PITCH_UP:
                        case Opcode::MSG_MOVE_START_PITCH_DOWN:
                        case Opcode::MSG_MOVE_START_ASCEND:
                        case Opcode::MSG_MOVE_STOP_ASCEND:
                        case Opcode::MSG_MOVE_START_DESCEND:
                        case Opcode::MSG_MOVE_START_SWIM:
                        case Opcode::MSG_MOVE_STOP_SWIM:
                        case Opcode::MSG_MOVE_FALL_LAND:
                        case Opcode::CMSG_MOVE_FALL_RESET:
                        case Opcode::MSG_MOVE_JUMP:
                        case Opcode::MSG_MOVE_SET_FACING:
                        case Opcode::MSG_MOVE_SET_PITCH:
                        case Opcode::MSG_MOVE_SET_RUN_MODE:
                        case Opcode::MSG_MOVE_SET_WALK_MODE:
                        case Opcode::CMSG_MOVE_SET_FLY:
                        case Opcode::CMSG_MOVE_CHNG_TRANSPORT:
                        case Opcode::MSG_MOVE_HEARTBEAT:
                        {
                            ZoneScopedNC("Packet::Passthrough", tracy::Color::Orange2)

                            u64 guid = 0;
                            u32 movementFlags = 0;
                            u16 movementFlagsExtra = 0;
                            u32 gameTime = 0;
                            Vector3 position = Vector3::Zero;
                            f32 orientation = 0;
                            u32 fallTime = 0;

                            u8 opcodeIndex = CharacterUtils::GetLastMovementTimeIndexFromOpcode(opcode);
                            u32 opcodeTime = playerPositionData.lastMovementOpcodeTime[opcodeIndex];

                            packet.data->GetGuid(guid);
                            packet.data->GetU32(movementFlags);
                            packet.data->GetU16(movementFlagsExtra);
                            packet.data->GetU32(gameTime);
                            packet.data->Get<Vector3>(position);
                            packet.data->GetF32(orientation);
                            packet.data->GetU32(fallTime);

                            // Find time offset
                            if (playerPositionData.timeOffsetToServer == INVALID_TIME_OFFSET)
                            {
                                playerPositionData.timeOffsetToServer = singleton.lifeTimeInMS - gameTime;
                            }

                            if (gameTime > opcodeTime)
                            {
                                playerPositionData.lastMovementOpcodeTime[opcodeIndex] = gameTime;

                                PositionUpdateData positionUpdateData;
                                positionUpdateData.opcode = opcode;
                                positionUpdateData.movementFlags = movementFlags;
                                positionUpdateData.movementFlagsExtra = movementFlagsExtra;
                                positionUpdateData.gameTime = static_cast<u32>(singleton.lifeTimeInMS);
                                positionUpdateData.fallTime = fallTime;

                                playerPositionData.position = position;
                                playerPositionData.orientation = orientation;
                                positionUpdateData.position = position;

                                positionUpdateData.orientation = orientation;

                                playerUpdateData.positionUpdateData.push_back(positionUpdateData);
                            }

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_MESSAGECHAT:
                        {
                            ZoneScopedNC("Packet::MessageChat", tracy::Color::Orange2)
                                packet.handled = true;

                            u32 msgType;
                            u32 msgLang;

                            packet.data->GetU32(msgType);
                            packet.data->GetU32(msgLang);

                            if (msgType >= CHAT_MSG_TYPE_MAX)
                            {
                                // Client tried to use invalid type
                                break;
                            }

                            if (msgLang == LANG_UNIVERSAL && msgType != CHAT_MSG_AFK && msgType != CHAT_MSG_DND)
                            {
                                // Client tried to send a message in universal language. (While it not being afk or dnd)
                                break;
                            }

                            if (msgType == CHAT_MSG_AFK || msgType == CHAT_MSG_DND)
                            {
                                // We don't want to send this message to any client.
                                break;
                            }

                            std::string msgOutput;
                            switch (msgType)
                            {
                            case CHAT_MSG_SAY:
                            case CHAT_MSG_YELL:
                            case CHAT_MSG_EMOTE:
                            case CHAT_MSG_TEXT_EMOTE:
                            {
                                packet.data->GetString(msgOutput);
                                break;
                            }

                            default:
                            {
                                worldNodeHandler.PrintMessage("Account(%u), Character(%u) sent unhandled message type %u", playerConnection.accountGuid, playerConnection.characterGuid, msgType);
                                break;
                            }
                            }

                            // Max Message Size is 255
                            if (msgOutput.size() > 255)
                                break;

                            /* Build Packet */
                            ChatUpdateData chatUpdateData;
                            chatUpdateData.chatType = msgType;
                            chatUpdateData.language = msgLang;
                            chatUpdateData.sender = playerConnection.characterGuid;
                            chatUpdateData.message = msgOutput;
                            chatUpdateData.handled = false;
                            playerUpdateData.chatUpdateData.push_back(chatUpdateData);

                            // Call OnPlayerChat script hooks
                            AngelScriptPlayer asPlayer(playerConnection.entityId, &registry);
                            PlayerHooks::CallHook(PlayerHooks::Hooks::HOOK_ONPLAYERCHAT, &asPlayer, msgOutput);
                            break;
                        }
                        case Opcode::CMSG_ATTACKSWING:
                        {
                            ZoneScopedNC("Packet::AttackSwing", tracy::Color::Orange2)

                                u64 attackGuid;
                            packet.data->GetU64(attackGuid);

                            std::shared_ptr<DataStore> attackBuffer = DataStore::Borrow<50>();
                            attackBuffer->PutU64(playerConnection.characterGuid);
                            attackBuffer->PutU64(attackGuid);
                            playerConnection.socket->SendPacket(attackBuffer.get(), Opcode::SMSG_ATTACKSTART);

                            attackBuffer->Reset();
                            attackBuffer->PutU32(0);
                            attackBuffer->PutGuid(playerConnection.characterGuid);
                            attackBuffer->PutGuid(attackGuid);
                            attackBuffer->PutU32(5);
                            attackBuffer->PutU32(0);
                            attackBuffer->PutU8(1);

                            attackBuffer->PutU32(1);
                            attackBuffer->PutF32(5);
                            attackBuffer->PutU32(5);

                            attackBuffer->PutU8(0);
                            attackBuffer->PutU32(0);
                            attackBuffer->PutU32(0);

                            playerConnection.socket->SendPacket(attackBuffer.get(), Opcode::SMSG_ATTACKERSTATEUPDATE);

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_ATTACKSTOP:
                        {
                            ZoneScopedNC("Packet::AttackStop", tracy::Color::Orange2)

                                u64 attackGuid = clientFieldData.GetFieldValue<u64>(UNIT_FIELD_TARGET);

                            std::shared_ptr<DataStore> attackStop = DataStore::Borrow<20>();
                            attackStop->PutGuid(playerConnection.characterGuid);
                            attackStop->PutGuid(attackGuid);
                            attackStop->PutU32(0);

                            playerPacketQueue.packetQueue->enqueue(PacketQueueData(playerConnection.socket, attackStop, Opcode::SMSG_ATTACKSTOP));

                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_SETSHEATHED:
                        {
                            ZoneScopedNC("Packet::SetSheathed", tracy::Color::Orange2)

                                u32 state;
                            packet.data->GetU32(state);

                            clientFieldData.SetFieldValue<u8>(UNIT_FIELD_BYTES_2, static_cast<u8>(state));
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_TEXT_EMOTE:
                        {
                            ZoneScopedNC("Packet::Text_emote", tracy::Color::Orange2)

                            u32 emoteTextId;
                            u32 emoteSoundIndex;
                            u64 targetGuid;

                            packet.data->GetU32(emoteTextId);
                            packet.data->GetU32(emoteSoundIndex);
                            packet.data->GetU64(targetGuid);

							EmoteTextData emoteTextData;
                            if (dbcDatabase.cache->GetEmoteTextData(emoteTextId, emoteTextData))
                            {
                                std::shared_ptr<DataStore> buffer = DataStore::Borrow<36>();
                                /* Play animation packet. */
                                {
                                    //The animation shouldn't play if the player is dead. In the future we should check for that.

                                    buffer->PutU32(emoteTextData.animationId);
                                    buffer->PutU64(playerConnection.characterGuid);

                                    playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_EMOTE);
                                }

                                /* Emote Chat Message Packet. */
                                {
                                    CharacterInfo targetData;
                                    u32 targetNameLength = 0;
                                    if (characterDatabase.cache->GetCharacterInfo(targetGuid, targetData))
                                    {
                                        targetNameLength = static_cast<u32>(targetData.name.size());
                                    }

                                    buffer->Reset();
                                    buffer->PutU64(playerConnection.characterGuid);
                                    buffer->PutU32(emoteTextId);
                                    buffer->PutU32(emoteSoundIndex);
                                    buffer->PutU32(targetNameLength);
                                    if (targetNameLength > 1)
                                    {
                                        buffer->PutString(targetData.name);
                                    }
                                    else
                                    {
                                        buffer->PutU8(0x00);
                                    }

                                    playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_TEXT_EMOTE);
                                }
                            }
                            packet.handled = true;
                            break;
                        }
                        case Opcode::CMSG_CAST_SPELL:
                        {
                            packet.handled = true;

                            u32 spellId = 0, targetFlags = 0;
                            u8 castCount = 0, castFlags = 0;

                            packet.data->GetU8(castCount);
                            packet.data->GetU32(spellId);
                            packet.data->GetU8(castFlags);
                            packet.data->GetU32(targetFlags);

                            // As far as I can tell, the client expects SMSG_SPELL_START followed by SMSG_SPELL_GO.
                            std::shared_ptr<DataStore> buffer = DataStore::Borrow<512>();

                            // Handle blink!
                            if (spellId == 1953)
                            {
                                f32 tempHeight = playerPositionData.position.z;
                                u32 dest = 20;

                                for (u32 i = 0; i < 20; i++)
                                {
                                    f32 newPositionX = playerPositionData.position.x + i * Math::Cos(playerPositionData.orientation);
                                    f32 newPositionY = playerPositionData.position.y + i * Math::Sin(playerPositionData.orientation);
                                    Vector2 newPos(newPositionX, newPositionY);
                                    f32 height = mapSingleton.maps[playerPositionData.mapId].GetHeight(newPos);
                                    f32 deltaHeight = Math::Abs(tempHeight - height);

                                    if (deltaHeight <= 2.0f || (i == 0 && deltaHeight <= 20))
                                    {
                                        dest = i;
                                        tempHeight = height;
                                    }
                                }

                                if (dest == 20)
                                {
                                    buffer->Reset();
                                    buffer->PutU8(castCount);
                                    buffer->PutU32(spellId);
                                    buffer->PutU8(173); // SPELL_FAILED_TRY_AGAIN

                                    playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_CAST_FAILED);
                                    break;
                                }


                                f32 newPositionX = playerPositionData.position.x + dest * Math::Cos(playerPositionData.orientation);
                                f32 newPositionY = playerPositionData.position.y + dest * Math::Sin(playerPositionData.orientation);

                                /*
                                    Adding 2.0f to the final height will solve 90%+ of issues where we fall through the terrain, remove this to fully test blink's capabilities.
                                    This also introduce the bug where after a blink, you might appear a bit over the ground and fall down.
                                */
                                Vector2 newPos(newPositionX, newPositionY);
                                f32 height = mapSingleton.maps[playerPositionData.mapId].GetHeight(newPos);

                                buffer->PutGuid(playerConnection.characterGuid);
                                buffer->PutU32(0); // Teleport Count

                                /* Movement */
                                buffer->PutU32(0);
                                buffer->PutU16(0);
                                buffer->PutU32(static_cast<u32>(singleton.lifeTimeInMS));

                                buffer->PutF32(newPositionX);
                                buffer->PutF32(newPositionY);
                                buffer->PutF32(height);
                                buffer->PutF32(playerPositionData.orientation);

                                buffer->PutU32(targetFlags);

                                playerConnection.socket->SendPacket(buffer.get(), Opcode::MSG_MOVE_TELEPORT_ACK);
                            }

                            buffer->Reset();
                            buffer->PutGuid(playerConnection.characterGuid);
                            buffer->PutGuid(playerConnection.characterGuid);
                            buffer->PutU8(0); // CastCount
                            buffer->PutU32(spellId);
                            buffer->PutU32(0x00000002);
                            buffer->PutU32(0);
                            buffer->PutU32(0);
                            playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_SPELL_START);

                            buffer->Reset();
                            buffer->PutGuid(playerConnection.characterGuid);
                            buffer->PutGuid(playerConnection.characterGuid);
                            buffer->PutU8(0); // CastCount
                            buffer->PutU32(spellId);
                            buffer->PutU32(0x00000100);
                            buffer->PutU32(static_cast<u32>(singleton.lifeTimeInMS));

                            buffer->PutU8(1); // Affected Targets
                            buffer->PutU64(playerConnection.characterGuid); // Target GUID
                            buffer->PutU8(0); // Resisted Targets

                            if (targetFlags == 0) // SELF
                            {
                                targetFlags = 0x02; // UNIT
                            }
                            buffer->PutU32(targetFlags); // Target Flags
                            buffer->PutU8(0); // Target Flags

                            playerConnection.socket->SendPacket(buffer.get(), Opcode::SMSG_SPELL_GO);
                            break;
                        }
                        default:
                        {
                            ZoneScopedNC("Packet::Unhandled", tracy::Color::Orange2)
                            {
                                ZoneScopedNC("Packet::Unhandled::Log", tracy::Color::Orange2)
                                    worldNodeHandler.PrintMessage("Account(%u), Character(%u) sent unhandled opcode %u", playerConnection.accountGuid, playerConnection.characterGuid, opcode);
                            }

                            // Mark all unhandled opcodes as handled to prevent the queue from trying to handle them every tick.
                            packet.handled = true;
                            break;
                        }
                        }
                    }
                /* Cull Movement Data */

                if (playerConnection.packets.size() > 0)
                {
                    ZoneScopedNC("Packet::PacketClear", tracy::Color::Orange2)
                        playerConnection.packets.erase(std::remove_if(playerConnection.packets.begin(), playerConnection.packets.end(), [](NetPacket& packet)
                            {
                                return packet.handled;
                            }), playerConnection.packets.end());
                }
            });
    }
}
