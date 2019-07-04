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

#include "DataStore.h"
#include "BaseSocket.h"

#include <ctime>
#include <iostream>
#include <string>
#include <functional>
#include <asio.hpp>
#include <asio/placeholders.hpp>

class BaseSocket : public std::enable_shared_from_this<BaseSocket>
{
public:
    virtual bool Start() = 0;
    virtual void Close(asio::error_code error)
    {
        _socket->close();
        _isClosed = true;
    }
    virtual void HandleRead() = 0;

    asio::ip::tcp::socket* socket()
    {
        return _socket;
    }

    void Send(DataStore& dataStore)
    {
        if (!dataStore.IsEmpty() || dataStore.IsFull())
        {
            _socket->async_write_some(asio::buffer(dataStore.GetInternalData(), dataStore.WrittenData),
                                      std::bind(&BaseSocket::HandleInternalWrite, this, std::placeholders::_1, std::placeholders::_2));
        }
    }
    bool IsClosed() { return _isClosed; }

protected:
    BaseSocket(asio::ip::tcp::socket* socket) : _receiveBuffer(nullptr, 4096), _sendBuffer(nullptr, 4096), _isClosed(false), _socket(socket) {}

    void AsyncRead()
    {
        if (!_socket->is_open())
            return;

        _receiveBuffer.Reset();
        _socket->async_read_some(asio::buffer(_receiveBuffer.GetWritePointer(), _receiveBuffer.GetRemainingSpace()),
                                 std::bind(&BaseSocket::HandleInternalRead, this, std::placeholders::_1, std::placeholders::_2));
    }
    void HandleInternalRead(asio::error_code error, size_t bytes)
    {
        if (error)
        {
            Close(error);
            return;
        }

        _receiveBuffer.WrittenData += bytes;
        HandleRead();
    }
    void HandleInternalWrite(asio::error_code error, std::size_t transferedBytes)
    {
        if (error)
        {
            Close(error);
        }
    }

    DataStore& GetReceiveBuffer() { return _receiveBuffer; }
    DataStore _receiveBuffer;
    DataStore _sendBuffer;

    bool _isClosed = false;
    asio::ip::tcp::socket* _socket;
};