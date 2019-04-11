/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/

#include "NovusConnection.h"
#include <Networking\ByteBuffer.h>
#include <Networking\Opcode\Opcode.h>
#include <Database\DatabaseConnector.h>
#include <algorithm>
#include <string>
#include <vector>

#include "../Utils/CharacterUtils.h"

enum CharacterResponses
{    
    CHAR_CREATE_IN_PROGRESS                                = 46,
    CHAR_CREATE_SUCCESS                                    = 47,
    CHAR_CREATE_ERROR                                      = 48,
    CHAR_CREATE_FAILED                                     = 49,
    CHAR_CREATE_NAME_IN_USE                                = 50,
    CHAR_CREATE_DISABLED                                   = 51,
    CHAR_CREATE_PVP_TEAMS_VIOLATION                        = 52,
    CHAR_CREATE_SERVER_LIMIT                               = 53,
    CHAR_CREATE_ACCOUNT_LIMIT                              = 54,
    CHAR_CREATE_SERVER_QUEUE                               = 55,
    CHAR_CREATE_ONLY_EXISTING                              = 56,
    CHAR_CREATE_EXPANSION                                  = 57,
    CHAR_CREATE_EXPANSION_CLASS                            = 58,
    CHAR_CREATE_LEVEL_REQUIREMENT                          = 59,
    CHAR_CREATE_UNIQUE_CLASS_LIMIT                         = 60,
    CHAR_CREATE_CHARACTER_IN_GUILD                         = 61,
    CHAR_CREATE_RESTRICTED_RACECLASS                       = 62,
    CHAR_CREATE_CHARACTER_CHOOSE_RACE                      = 63,
    CHAR_CREATE_CHARACTER_ARENA_LEADER                     = 64,
    CHAR_CREATE_CHARACTER_DELETE_MAIL                      = 65,
    CHAR_CREATE_CHARACTER_SWAP_FACTION                     = 66,
    CHAR_CREATE_CHARACTER_RACE_ONLY                        = 67,
    CHAR_CREATE_CHARACTER_GOLD_LIMIT                       = 68,
    CHAR_CREATE_FORCE_LOGIN                                = 69,

    CHAR_DELETE_IN_PROGRESS                                = 70,
    CHAR_DELETE_SUCCESS                                    = 71,
    CHAR_DELETE_FAILED                                     = 72,
    CHAR_DELETE_FAILED_LOCKED_FOR_TRANSFER                 = 73,
    CHAR_DELETE_FAILED_GUILD_LEADER                        = 74,
    CHAR_DELETE_FAILED_ARENA_CAPTAIN                       = 75,
};

robin_hood::unordered_map<u8, NovusMessageHandler> NovusConnection::InitMessageHandlers()
{
    robin_hood::unordered_map<u8, NovusMessageHandler> messageHandlers;

    messageHandlers[NOVUS_CHALLENGE]        = { NOVUSSTATUS_CHALLENGE,    sizeof(sNovusChallenge),  &NovusConnection::HandleCommandChallenge };
    messageHandlers[NOVUS_PROOF]            = { NOVUSSTATUS_PROOF,        1,                        &NovusConnection::HandleCommandProof };
    messageHandlers[NOVUS_FORWARDPACKET]    = { NOVUSSTATUS_AUTHED,       9,                        &NovusConnection::HandleCommandForwardPacket };

    return messageHandlers;
}
robin_hood::unordered_map<u8, NovusMessageHandler> const MessageHandlers = NovusConnection::InitMessageHandlers();

bool NovusConnection::Start()
{
    try
    {
        _socket->connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(_address), _port));

        /* NODE_CHALLENGE */
        Common::ByteBuffer packet(6);
        packet.Write<u8>(0);       // Command
        packet.Write<u8>(0);       // Type
        packet.Write<u16>(335);    // Version
        packet.Write<u16>(12340);  // Build

        AsyncRead();
        Send(packet);
        return true;
    }
    catch (asio::system_error error)
    {
        std::cout << "ERROR: " << error.what() << std::endl;
        return false;
    }
}

void NovusConnection::HandleRead()
{
    Common::ByteBuffer& buffer = GetByteBuffer();

    bool isDecrypted = false;
    while (buffer.GetActualSize())
    {
        // Decrypt data post CHALLENGE Status
        if (!isDecrypted && (_status == NOVUSSTATUS_PROOF || _status == NOVUSSTATUS_AUTHED))
        {
            _crypto->Decrypt(buffer.GetReadPointer(), buffer.GetActualSize());
            isDecrypted = true;
        }

        u8 command = buffer.GetDataPointer()[0];

        auto itr = MessageHandlers.find(command);
        if (itr == MessageHandlers.end())
        {
            std::cout << "Received HandleRead with no MessageHandler to respond." << std::endl;
            buffer.Clean();
            break;
        }

        if (_status != itr->second.status)
        {
            Close(asio::error::shut_down);
            return;
        }

        if (command == NOVUS_FORWARDPACKET)
        {
            // Check if we should read header
            if (_headerBuffer.GetSpaceLeft() > 0)
            {
                size_t headerSize = std::min(buffer.GetActualSize(), _headerBuffer.GetSpaceLeft());
                _headerBuffer.Write(buffer.GetReadPointer(), headerSize);
                buffer.ReadBytes(headerSize);

                if (_headerBuffer.GetSpaceLeft() > 0)
                {
                    // Wait until we have the entire header
                    assert(buffer.GetActualSize() == 0);
                    break;
                }

                /* Read Header */
                NovusHeader* header = reinterpret_cast<NovusHeader*>(_headerBuffer.GetReadPointer());
                _packetBuffer.Resize(header->size);
                _packetBuffer.ResetPos();
            }

            // We have a header, now check the packet data
            if (_packetBuffer.GetSpaceLeft() > 0)
            {
                std::size_t packetSize = std::min(buffer.GetActualSize(), _packetBuffer.GetSpaceLeft());
                _packetBuffer.Write(buffer.GetReadPointer(), packetSize);
                buffer.ReadBytes(packetSize);

                if (_packetBuffer.GetSpaceLeft() > 0)
                {
                    // Wait until we have all of the packet data
                    assert(buffer.GetActualSize() == 0);
                    break;
                }
            }

            if (!HandleCommandForwardPacket())
            {
                Close(asio::error::shut_down);
                return;
            }
            _headerBuffer.ResetPos();
        }
        else
        {
            u16 size = static_cast<u16>(itr->second.packetSize);
            if (buffer.GetActualSize() < size)
                break;

            if (!(*this.*itr->second.handler)())
            {
                Close(asio::error::shut_down);
                return;
            }

            buffer.ReadBytes(size);
        }
    }

    AsyncRead();
}

bool NovusConnection::HandleCommandChallenge()
{
    _status = NOVUSSTATUS_CLOSED;
    sNovusChallenge* novusChallenge = reinterpret_cast<sNovusChallenge*>(GetByteBuffer().GetReadPointer());

    _key->Bin2BN(novusChallenge->K, 32);
    _crypto->SetupClient(_key);

    /* Send fancy encrypted packet here */
    Common::ByteBuffer packet;
    packet.Write<u8>(NOVUS_PROOF); // RELAY_PROOF
    _crypto->Encrypt(packet.GetReadPointer(), packet.GetActualSize());
    _status = NOVUSSTATUS_PROOF;

    Send(packet);
    return true;
}
bool NovusConnection::HandleCommandProof()
{
    _status = NOVUSSTATUS_AUTHED;

    return true;
}

bool NovusConnection::HandleCommandForwardPacket()
{
    NovusHeader* header = reinterpret_cast<NovusHeader*>(_headerBuffer.GetReadPointer());
    //std::cout << "Received opcode: 0x" << std::hex << std::uppercase << header->opcode << std::endl;

    switch (static_cast<Common::Opcode>(header->opcode))
    {
        case Common::Opcode::CMSG_READY_FOR_ACCOUNT_DATA_TIMES:
        {
            /* Packet Structure */
            // UInt32:  Server Time (time(nullptr))
            // UInt8:   Unknown Byte Value
            // UInt32:  Mask for the account data fields

            Common::ByteBuffer accountDataForwardPacket;
            Common::ByteBuffer accountDataTimes;
            NovusHeader packetHeader;
            packetHeader.command = NOVUS_FORWARDPACKET;
            packetHeader.account = header->account;
            packetHeader.opcode = Common::Opcode::SMSG_ACCOUNT_DATA_TIMES;

            u32 mask = 0x15;
            accountDataTimes.Write<u32>(static_cast<u32>(time(nullptr)));
            accountDataTimes.Write<u8>(1); // bitmask blocks count
            accountDataTimes.Write<u32>(mask); // PER_CHARACTER_CACHE_MASK

            for (u32 i = 0; i < 8; ++i)
            {
                if (mask & (1 << i))
                    accountDataTimes.Write<u32>(0);
            }

            packetHeader.size = static_cast<u16>(accountDataTimes.GetActualSize());
            packetHeader.AddTo(accountDataForwardPacket);
            accountDataForwardPacket.Append(accountDataTimes);

            SendPacket(accountDataForwardPacket);
            break;
        }
        case Common::Opcode::CMSG_UPDATE_ACCOUNT_DATA:
        {
            u32 type, timestamp, decompressedSize;
            _packetBuffer.Read(&type, 4);
            _packetBuffer.Read(&timestamp, 4);
            _packetBuffer.Read(&decompressedSize, 4);

            if (type > 8)
            {
                std::cout << "Bad Type." << std::endl;
                break;
            }

            NovusHeader packetHeader;
            packetHeader.command = NOVUS_FORWARDPACKET;
            packetHeader.account = header->account;
            packetHeader.opcode = Common::Opcode::SMSG_UPDATE_ACCOUNT_DATA_COMPLETE;
            packetHeader.size = 8;
            Common::ByteBuffer updateAccountDataComplete(9 + 4 + 4);
            packetHeader.AddTo(updateAccountDataComplete);

            updateAccountDataComplete.Write<u32>(type);
            updateAccountDataComplete.Write<u32>(0);
            SendPacket(updateAccountDataComplete);
            break;
        }
        case Common::Opcode::CMSG_REALM_SPLIT:
        {
            Common::ByteBuffer forwardPacket;
            Common::ByteBuffer realmSplit;

            std::string split_date = "01/01/01";
            u32 unk = 0;
            _packetBuffer.Read<u32>(unk);

            NovusHeader packetHeader;
            packetHeader.command = NOVUS_FORWARDPACKET;
            packetHeader.account = header->account;
            packetHeader.opcode = Common::Opcode::SMSG_REALM_SPLIT;

            realmSplit.Write<u32>(unk);
            realmSplit.Write<u32>(0x0); // split states: 0x0 realm normal, 0x1 realm split, 0x2 realm split pending
            realmSplit.WriteString(split_date);

            packetHeader.size = realmSplit.GetActualSize();
            packetHeader.AddTo(forwardPacket);
            forwardPacket.Append(realmSplit);

            SendPacket(forwardPacket);
            break;
        }
        case Common::Opcode::CMSG_CHAR_ENUM:
        {
            PreparedStatement stmt("SELECT characters.guid, characters.name, characters.race, characters.class, characters.gender, character_visual_data.skin, character_visual_data.face, character_visual_data.hair_style, character_visual_data.hair_color, character_visual_data.facial_style, characters.level, characters.zoneId, characters.mapId, characters.coordinate_x, characters.coordinate_y, characters.coordinate_z FROM characters INNER JOIN character_visual_data ON characters.guid=character_visual_data.guid WHERE characters.account={u};");
            stmt.Bind(header->account);
            DatabaseConnector::QueryAsync(DATABASE_TYPE::CHARSERVER, stmt, [this, header](amy::result_set& results, DatabaseConnector& connector)
            {
                Common::ByteBuffer forwardPacket;
                Common::ByteBuffer charEnum;

                NovusHeader packetHeader;
                packetHeader.command = NOVUS_FORWARDPACKET;
                packetHeader.account = header->account;
                packetHeader.opcode = Common::Opcode::SMSG_CHAR_ENUM;

                // Number of characters
                charEnum.Write<u8>(static_cast<u8>(results.affected_rows()));

                /* Template for loading a character */
                for (auto& row : results)
                {
                    charEnum.Write<u64>(row[0].GetU64()); // Guid
                    charEnum.WriteString(row[1].GetString()); // Name
                    charEnum.Write<u8>(row[2].GetU8()); // Race
                    charEnum.Write<u8>(row[3].GetU8()); // Class
                    charEnum.Write<u8>(row[4].GetU8()); // Gender

                    charEnum.Write<u8>(row[5].GetU8()); // Skin
                    charEnum.Write<u8>(row[6].GetU8()); // Face
                    charEnum.Write<u8>(row[7].GetU8()); // Hairstyle
                    charEnum.Write<u8>(row[8].GetU8()); // Haircolor
                    charEnum.Write<u8>(row[9].GetU8()); // Facialstyle

                    charEnum.Write<u8>(row[10].GetU8()); // Level
                    charEnum.Write<u32>(row[11].GetU16()); // Zone Id
                    charEnum.Write<u32>(row[12].GetU16()); // Map Id

                    charEnum.Write<f32>(row[13].GetF32()); // X
                    charEnum.Write<f32>(row[14].GetF32()); // Y
                    charEnum.Write<f32>(row[15].GetF32()); // Z

                    charEnum.Write<u32>(0); // Guild Id

                    charEnum.Write<u32>(0); // Character Flags
                    charEnum.Write<u32>(0); // characterCustomize Flag

                    charEnum.Write<u8>(1); // First Login (Here we should probably do a playerTime check to determin if its the player's first login)

                    charEnum.Write<u32>(0); // Pet Display Id (Lich King: 22234)
                    charEnum.Write<u32>(0);  // Pet Level
                    charEnum.Write<u32>(0);  // Pet Family

                    u32 equipmentDataNull = 0;
                    for (i32 i = 0; i < 23; ++i)
                    {
                        charEnum.Write<u32>(equipmentDataNull);
                        charEnum.Write<u8>(0);
                        charEnum.Write<u32>(equipmentDataNull);
                    }
                }

                packetHeader.size = charEnum.GetActualSize();
                packetHeader.AddTo(forwardPacket);
                forwardPacket.Append(charEnum);

                SendPacket(forwardPacket);
            });
            break;
        }
        case Common::Opcode::CMSG_CHAR_CREATE:
        {
            cCharacterCreateData* createData = new cCharacterCreateData();
            createData->Read(_packetBuffer);

            PreparedStatement stmt("SELECT name FROM characters WHERE name={s};");
            stmt.Bind(createData->charName);
            DatabaseConnector::QueryAsync(DATABASE_TYPE::CHARSERVER, stmt, [this, header, createData](amy::result_set& results, DatabaseConnector& connector)
            {
                Common::ByteBuffer forwardPacket;

                NovusHeader packetHeader;
                packetHeader.command = NOVUS_FORWARDPACKET;
                packetHeader.account = header->account;
                packetHeader.opcode = Common::Opcode::SMSG_CHAR_CREATE;
                packetHeader.size = 1;
                packetHeader.AddTo(forwardPacket);

                if (results.affected_rows() > 0)
                {
                    forwardPacket.Write<u8>(CHAR_CREATE_NAME_IN_USE);
                    SendPacket(forwardPacket);
                    delete createData;
                    return;
                }

                /* Convert name to proper format */
                std::transform(createData->charName.begin(), createData->charName.end(), createData->charName.begin(), ::tolower);
                createData->charName[0] = std::toupper(createData->charName[0]);

                CharacterUtils::SpawnPosition spawnPosition;
                if (!CharacterUtils::BuildGetDefaultSpawn(_cache.GetDefaultSpawnStorageData(), createData->charRace, createData->charClass, spawnPosition))
                {
                    forwardPacket.Write<u8>(CHAR_CREATE_DISABLED);
                    SendPacket(forwardPacket);
                    delete createData;
                    return;
                }

                PreparedStatement characterBaseData("INSERT INTO characters(account, name, race, gender, class, mapId, zoneId, coordinate_x, coordinate_y, coordinate_z, orientation) VALUES({u}, {s}, {u}, {u}, {u}, {u}, {u}, {f}, {f}, {f}, {f});");
                characterBaseData.Bind(header->account);
                characterBaseData.Bind(createData->charName);
                characterBaseData.Bind(createData->charRace);
                characterBaseData.Bind(createData->charGender);
                characterBaseData.Bind(createData->charClass);
                characterBaseData.Bind(spawnPosition.mapId);
                characterBaseData.Bind(spawnPosition.zoneId);
                characterBaseData.Bind(spawnPosition.coordinate_x);
                characterBaseData.Bind(spawnPosition.coordinate_y);
                characterBaseData.Bind(spawnPosition.coordinate_z);
                characterBaseData.Bind(spawnPosition.orientation);
                connector.Execute(characterBaseData);

                // This needs to be non-async as we rely on LAST_INSERT_ID() to retrieve the character's guid
                amy::result_set guidResult;
                connector.Query("SELECT LAST_INSERT_ID();", guidResult);
                u64 characterGuid = guidResult[0][0].as<amy::sql_bigint_unsigned>();

                PreparedStatement characterVisualData("INSERT INTO character_visual_data(guid, skin, face, facial_style, hair_style, hair_color) VALUES({u}, {u}, {u}, {u}, {u}, {u});");
                characterVisualData.Bind(characterGuid);
                characterVisualData.Bind(createData->charSkin);
                characterVisualData.Bind(createData->charFace);
                characterVisualData.Bind(createData->charFacialStyle);
                characterVisualData.Bind(createData->charHairStyle);
                characterVisualData.Bind(createData->charHairColor);
                connector.ExecuteAsync(characterVisualData);

                // Baseline Skills
                std::string skillSql;
                if (CharacterUtils::BuildDefaultSkillSQL(_cache.GetDefaultSkillStorageData(), characterGuid, createData->charRace, createData->charClass, skillSql))
                {
                    connector.ExecuteAsync(skillSql);
                }

                // Baseline Spells
                std::string spellSql;
                if (CharacterUtils::BuildDefaultSpellSQL(_cache.GetDefaultSpellStorageData(), characterGuid, createData->charRace, createData->charClass, spellSql))
                {
                    connector.ExecuteAsync(spellSql);
                }

                DatabaseConnector::Borrow(DATABASE_TYPE::AUTHSERVER, [this, header](std::shared_ptr<DatabaseConnector>& connector)
                {
                    PreparedStatement realmCharacterCount("INSERT INTO realm_characters(account, realmid, characters) VALUES({u}, {u}, 1) ON DUPLICATE KEY UPDATE characters = characters + 1;");
                    realmCharacterCount.Bind(header->account);
                    realmCharacterCount.Bind(_realmId);
                    connector->Execute(realmCharacterCount);
                });

                forwardPacket.Write<u8>(CHAR_CREATE_SUCCESS);
                SendPacket(forwardPacket);

                delete createData;
            });

            break;
        }
        case Common::Opcode::CMSG_CHAR_DELETE:
        {
            u64 guid = 0;
            _packetBuffer.Read<u64>(guid);

            PreparedStatement stmt("SELECT account FROM characters WHERE guid={u};");
            stmt.Bind(guid);
            DatabaseConnector::QueryAsync(DATABASE_TYPE::CHARSERVER, stmt, [this, header, guid](amy::result_set& results, DatabaseConnector& connector)
            {
                Common::ByteBuffer forwardPacket;
                NovusHeader packetHeader;
                packetHeader.command = NOVUS_FORWARDPACKET;
                packetHeader.account = header->account;
                packetHeader.opcode = Common::Opcode::SMSG_CHAR_DELETE;
                packetHeader.size = 1;
                packetHeader.AddTo(forwardPacket);

                // Char doesn't exist
                if (results.affected_rows() == 0)
                {
                    forwardPacket.Write<u8>(CHAR_DELETE_FAILED);
                    SendPacket(forwardPacket);
                    return;
                }

                // Prevent deleting other player's characters
                u64 characterAccountGuid = results[0][0].as<amy::sql_bigint_unsigned>();
                if (header->account != characterAccountGuid)
                {
                    forwardPacket.Write<u8>(CHAR_DELETE_FAILED);
                    SendPacket(forwardPacket);
                    return;
                }

                PreparedStatement characterBaseData("DELETE FROM characters WHERE guid={u};");
                characterBaseData.Bind(guid);

                PreparedStatement characterVisualData("DELETE FROM character_visual_data WHERE guid={u};");
                characterVisualData.Bind(guid);

                PreparedStatement charcaterSkillStorage("DELETE FROM character_skill_storage WHERE guid={u};");
                charcaterSkillStorage.Bind(guid);

                PreparedStatement charcaterSpellStorage("DELETE FROM character_spell_storage WHERE guid={u};");
                charcaterSpellStorage.Bind(guid);

                connector.Execute(characterBaseData);
                connector.Execute(characterVisualData);
                connector.Execute(charcaterSkillStorage);
                connector.Execute(charcaterSpellStorage);

                DatabaseConnector::Borrow(DATABASE_TYPE::AUTHSERVER, [this, header](std::shared_ptr<DatabaseConnector>& connector)
                {
                    PreparedStatement realmCharacterCount("UPDATE realm_characters SET characters=characters-1 WHERE account={u} and realmid={u};");
                    realmCharacterCount.Bind(header->account);
                    realmCharacterCount.Bind(_realmId);
                    connector->Execute(realmCharacterCount);
                });

                forwardPacket.Write<u8>(CHAR_DELETE_SUCCESS); 
                SendPacket(forwardPacket);
            });
            break;
        }     
        default:
            std::cout << "Could not handled opcode: 0x" << std::hex << std::uppercase << header->opcode << std::endl;
            break;
    }

    return true;
}

void NovusConnection::SendPacket(Common::ByteBuffer& packet)
{
    _crypto->Encrypt(packet.GetReadPointer(), packet.GetActualSize());
    Send(packet);
}