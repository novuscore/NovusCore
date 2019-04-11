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
#pragma once

#include <asio\ip\tcp.hpp>
#include <Networking\BaseSocket.h>
#include <Networking\TcpServer.h>
#include <Cryptography\BigNumber.h>
#include <Cryptography\StreamCrypto.h>
#include <robin_hood.h>
#include <NovusTypes.h>

#include "../DatabaseCache/CharacterDatabaseCache.h"

enum NovusCommand
{
    NOVUS_CHALLENGE         = 0x00,
    NOVUS_PROOF             = 0x01,
    NOVUS_FORWARDPACKET      = 0x02
};
enum NovusStatus
{
    NOVUSSTATUS_CHALLENGE   = 0x0,
    NOVUSSTATUS_PROOF       = 0x1,
    NOVUSSTATUS_AUTHED      = 0x2,
    NOVUSSTATUS_CLOSED      = 0x3
};

#pragma pack(push, 1)
struct sNovusChallenge
{
    u8 command;
    u8 K[32];
};

struct NovusHeader
{
    u8     command;
    u32    account;
    u16    opcode;
    u16    size;

    void Read(Common::ByteBuffer& buffer)
    {
        buffer.Read<u8>(command);
        buffer.Read<u32>(account);
        buffer.Read<u16>(opcode);
        buffer.Read<u16>(size);
    }

    void AddTo(Common::ByteBuffer& buffer)
    {
        buffer.Append(reinterpret_cast<u8*>(this), sizeof(NovusHeader));
    }
};

struct cCharacterCreateData
{
    std::string charName;
    u8 charRace;
    u8 charClass;
    u8 charGender;
    u8 charSkin;
    u8 charFace;
    u8 charHairStyle;
    u8 charHairColor;
    u8 charFacialStyle;
    u8 charOutfitId;

    void Read(Common::ByteBuffer& buffer)
    {
        buffer.Read(charName);
        buffer.Read<u8>(charRace);
        buffer.Read<u8>(charClass);
        buffer.Read<u8>(charGender);
        buffer.Read<u8>(charSkin);
        buffer.Read<u8>(charFace);
        buffer.Read<u8>(charHairStyle);
        buffer.Read<u8>(charHairColor);
        buffer.Read<u8>(charFacialStyle);
        buffer.Read<u8>(charOutfitId);
    }
};

class NovusConnection;
struct NovusMessageHandler
{
    NovusStatus status;
    size_t packetSize;
    bool (NovusConnection::*handler)();
};
#pragma pack(pop)

class NovusConnection : Common::BaseSocket
{
public:
    static robin_hood::unordered_map<u8, NovusMessageHandler> InitMessageHandlers();

    NovusConnection(asio::ip::tcp::socket* socket, std::string address, u16 port, u8 realmId, CharacterDatabaseCache& cache) : Common::BaseSocket(socket), _status(NOVUSSTATUS_CHALLENGE), _crypto(), _address(address), _port(port), _realmId(realmId), _cache(cache), _headerBuffer(), _packetBuffer()
    { 
        _crypto = new StreamCrypto();
        _key = new BigNumber();

        _headerBuffer.Resize(sizeof(NovusHeader));
    }

    bool Start() override;
    void HandleRead() override;

    bool HandleCommandChallenge();
    bool HandleCommandProof();
    bool HandleCommandForwardPacket();

    void SendPacket(Common::ByteBuffer& packet);

    NovusStatus _status;
private:
    std::string _address;
    u16 _port;
    u8 _realmId;

    StreamCrypto* _crypto;
    BigNumber* _key;

    Common::ByteBuffer _headerBuffer;
    Common::ByteBuffer _packetBuffer;

    CharacterDatabaseCache& _cache;
};