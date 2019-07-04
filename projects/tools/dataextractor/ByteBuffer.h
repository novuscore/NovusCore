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

#include <NovusTypes.h>
#include <vector>
#include <cassert>
#include <cstring>
#include <string>

class ByteBuffer
{
public:
    ByteBuffer() : _readPos(0), _writePos(0)
    {
        _bufferData.reserve(4096);
    }
    ByteBuffer(size_t reserveSize) : _readPos(0), _writePos(0)
    {
        _bufferData.reserve(reserveSize);
    }
    virtual ~ByteBuffer() {}

    template <typename T>
    void Read(T& destination)
    {
        destination = *(reinterpret_cast<T const*>(&_bufferData[_readPos]));
        _readPos += sizeof(T);
    }
    template <typename T>
    T ReadAt(size_t position)
    {
        return *(reinterpret_cast<T const*>(&_bufferData[position]));
    }
    void Read(void* destination, size_t length)
    {
        memcpy(destination, &_bufferData[_readPos], length);
        _readPos += length;
    }
    void Read(std::string& value)
    {
        value.clear();
        while (_readPos < size()) // prevent crash at wrong string format in packet
        {
            char c = Read();
            if (c == 0)
                break;
            value += c;
        }
    }
    char Read()
    {
        char r = Read(_readPos);
        _readPos += sizeof(char);
        return r;
    }
    char Read(size_t position)
    {
        char val = *(reinterpret_cast<char const*>(&_bufferData[position]));
        return val;
    }

    void WriteString(const std::string& value)
    {
        if (size_t len = value.length())
            Append((u8 const*)value.c_str(), len);
        Write<u8>(0);
    }
    void Write(void const* data, std::size_t size)
    {
        if (size)
        {
            std::memcpy(&_bufferData[_writePos], data, size);
            WriteBytes(size);
        }
    }
    template <typename T>
    void Write(T const value)
    {
        Append(reinterpret_cast<const u8*>(&value), sizeof(value));
    }
    template <typename T>
    void WriteAt(T const value, size_t position)
    {
        assert(_bufferData.size() > position);
        std::memcpy(&_bufferData[position], reinterpret_cast<const u8*>(&value), sizeof(T));
    }

    template <typename T>
    void Replace(size_t position, T Value)
    {
        _replace(position, reinterpret_cast<u8*>(&Value), sizeof(Value));
    }
    void _replace(size_t position, u8 const* src, size_t content)
    {
        std::memcpy(&_bufferData[position], src, content);
    }

    void Append(ByteBuffer const& buffer)
    {
        if (buffer._writePos)
            Append(buffer.data(), buffer._writePos);
    }

    void Append(u8 const* value, size_t size)
    {
        size_t const newSize = _writePos + size;
        if (_bufferData.capacity() < newSize)
        {
            if (newSize < 100)
                _bufferData.reserve(300);
            else if (newSize < 750)
                _bufferData.reserve(2500);
            else if (newSize < 6000)
                _bufferData.reserve(10000);
            else
                _bufferData.reserve(400000);
        }

        if (_bufferData.size() < newSize)
            _bufferData.resize(newSize);

        std::memcpy(&_bufferData[_writePos], value, size);
        _writePos = newSize;
    }
    void ResetPos()
    {
        _readPos = 0;
        _writePos = 0;
    }

    void Clean()
    {
        _readPos = 0;
        _writePos = 0;
        _bufferData.clear();
    }
    void Resize(size_t newSize)
    {
        _bufferData.resize(newSize);
    }
    u8* data()
    {
        return _bufferData.data();
    }
    u8 const* data() const
    {
        return _bufferData.data();
    }

    void WriteBytes(size_t size)
    {
        _writePos += size;
    }

    void ReadBytes(size_t size)
    {
        _readPos += size;
    }

    void CleanBuffer()
    {
        if (_readPos)
        {
            if (_readPos != _writePos)
                memmove(GetDataPointer(), GetReadPointer(), GetActualSize());
            _writePos -= _readPos;
            _readPos = 0;
        }
    }

    void RecalculateSize()
    {
        if (GetSpaceLeft() == 0)
        {
            _bufferData.resize(static_cast<u64>(_bufferData.size() * 1.5f));
        }
    }

    u8* GetDataPointer() { return _bufferData.data(); }
    u8* GetReadPointer() { return _bufferData.data() + _readPos; }
    u8* GetWritePointer() { return _bufferData.data() + _writePos; }
    u32 GetActualSize() { return static_cast<u32>(_writePos - _readPos); }
    u32 GetSpaceLeft() { return static_cast<u32>(_bufferData.size() - _writePos); }
    u32 size() const { return static_cast<u32>(_bufferData.size()); }
    bool empty() const { return _bufferData.empty(); }

    size_t _readPos, _writePos;

private:
    std::vector<u8> _bufferData;
};