#pragma once

#include "ByteBuffer.h"
#include "typedefs.h"

namespace Movement
{
    class PackedGuid;
    struct PackedGuidReader;
    class ObjectGuid
    {
        uint64 _value;
        friend void operator >> (ByteBuffer& buf, ObjectGuid& guid);
    public:
        explicit ObjectGuid() { SetRawValue(0);}
        explicit ObjectGuid(const uint64& value) { SetRawValue(value);}

        void SetRawValue(uint64 raw) { _value = raw;}
        uint64 GetRawValue() const { return _value;}

        PackedGuidReader ReadAsPacked();
        PackedGuid WriteAsPacked() const;

        bool operator == (const ObjectGuid& other) const {
            return _value == other._value;
        }
    };

    inline void operator << (ByteBuffer& buf, const ObjectGuid& guid)
    {
        buf << guid.GetRawValue();
    }

    inline void operator >> (ByteBuffer& buf, ObjectGuid& guid)
    {
        buf >> guid._value;
    }

    struct PackedGuidReader
    {
        ObjectGuid * m_guid;
        explicit PackedGuidReader(ObjectGuid* guid) : m_guid(guid) {}
    };

    class PackedGuid
    {
        friend void operator << (ByteBuffer& buf, const PackedGuid & guid);

    public:
        explicit PackedGuid() { Set(0); }
        explicit PackedGuid(ObjectGuid const& guid) { Set(guid.GetRawValue()); }

        void Set(uint64 const& _guid)
        {
            uint64 guid = _guid;
            _byteCount = 0;
            _mask = 0;
            for (uint8 byteIdx = 0; guid != 0;)
            {
                if (guid & 0xFF)
                {
                    _mask |= (1 << byteIdx);
                    _packed[_byteCount] = (uint8)(guid & 0xFF);
                    ++_byteCount;
                }
                guid >>= 8;
                ++byteIdx;
            }
        }

        uint64 Get() const
        {
            uint64 guid = 0;
            uint8 count = 0;
            for (uint8 byteIdx = 0; byteIdx < 8; ++byteIdx)
            {
                if (_mask & (1 << byteIdx))
                    guid |= uint64(_packed[count++]) << (byteIdx * 8);
            }
            return guid;
        }

        static ObjectGuid Read(ByteBuffer& buffer)
        {
            uint8 _mask = buffer.read<uint8>();
            uint64 guid = 0;
            for (uint8 byteIdx = 0; byteIdx < 8; ++byteIdx)
            {
                if (_mask & (1 << byteIdx))
                    guid |= uint64(buffer.read<uint8>()) << (byteIdx * 8);
            }
            return ObjectGuid(guid);
        }

    private:
        uint8 _mask;
        uint8 _packed[8];
        uint8 _byteCount;
    };

    inline PackedGuidReader ObjectGuid::ReadAsPacked() { return PackedGuidReader(this); }
    inline PackedGuid ObjectGuid::WriteAsPacked() const { return PackedGuid(*this);}

    inline void operator << (ByteBuffer& buf, const PackedGuid & guid)
    {
        buf << guid._mask;
        buf.append(&guid._packed[0], guid._byteCount);
    }

    inline void operator >> (ByteBuffer& buf, const PackedGuidReader& reader)
    {
        *reader.m_guid = PackedGuid::Read(buf);
    }
}
