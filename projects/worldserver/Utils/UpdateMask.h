#pragma once
#include <NovusTypes.h>
#include <Networking/ByteBuffer.h>
#include <bitset>

#define CLIENT_UPDATE_MASK_BITS 32
template <size_t size>
class UpdateMask
{
public:
    UpdateMask(u32 valuesCount)
    {
        Reset();

        _fieldCount = valuesCount;
        _blockCount = (valuesCount + CLIENT_UPDATE_MASK_BITS - 1) / CLIENT_UPDATE_MASK_BITS;
    }

    void SetBit(u32 index)
    {
        _bits.set(index);
    }
    void UnsetBit(u32 index)
    {
        _bits.reset(index);
    }
    bool IsSet(u32 index)
    {
        return _bits.test(index);
    }
    void Reset()
    {
        _bits.reset();
    }
    bool Any()
    {
        return _bits.any();
    }

    void AddTo(Common::ByteBuffer& buffer)
    {
        u32 maskPart = 0;
		u32 j = 0;
        for (u32 i = 0; i < GetBlocks() * 32; i++)
        {
            if (IsSet(i))
                maskPart |= 1 << j;

            if (++j == 32)
            {
                buffer.Write<u32>(maskPart);
                maskPart = 0;
				j = 0;
            }
        }
    }

    u32 GetFields() { return _fieldCount; }
    u32 GetBlocks() { return _blockCount; }

private:
    u32 _fieldCount;
    u32 _blockCount;
    std::bitset<size> _bits;
};