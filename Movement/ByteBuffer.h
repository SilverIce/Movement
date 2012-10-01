#pragma once

#include "framework/typedefs.h"
#include <assert.h>
#include <vector>
#include "POD_Arrays.h"

namespace Movement
{
    using Tasks::detail::MemBlock;

    template<class T>
    struct Unused
    {
    };

    class ByteBuffer
    {
    public:

        ByteBuffer() : w_pos(0), r_pos(0) {}

        explicit ByteBuffer(size_t size) : w_pos(0), r_pos(0) {
            buffer.reserve(size);
        }

        size_t rpos() const { return r_pos;}
        size_t wpos() const { return w_pos;}

        void rpos(size_t value) { r_pos = value;}

        void wpos(size_t value) { w_pos = value;}

        const char* contents() const { return &buffer[0];}

#define DECLARE_READ_WRITE(TYPE, NAME) \
        ByteBuffer& operator << (const TYPE& value) { \
            append((const char*)&value, sizeof(TYPE)); \
            return *this; \
        } \
        ByteBuffer& operator >> (TYPE& value) { \
            read((char*)&value, sizeof(TYPE)); \
            return *this; \
        } \
        TYPE read##NAME() { \
            TYPE value; \
            read((char*)&value, sizeof(TYPE)); \
            return value; \
        } \
        void write##NAME(const TYPE& value) { \
            append((const char*)&value, sizeof(TYPE)); \
        }

#define DECLARE_READ_WRITE_SIGNED_UNSIGNED(TYPE, NAME) \
        DECLARE_READ_WRITE(TYPE, NAME) \
        DECLARE_READ_WRITE(u##TYPE, U##NAME)

        DECLARE_READ_WRITE_SIGNED_UNSIGNED(int8, Int8);
        DECLARE_READ_WRITE_SIGNED_UNSIGNED(int16, Int16);
        DECLARE_READ_WRITE_SIGNED_UNSIGNED(int32, Int32);
        DECLARE_READ_WRITE_SIGNED_UNSIGNED(int64, Int64);
        DECLARE_READ_WRITE(float, Single);
        DECLARE_READ_WRITE(double, Double);

        template<class T> void put(size_t pos, const T& value)
        {
            put(pos, (const char*)&value, sizeof T);
        }

        void put(size_t pos, const char *src, size_t cnt)
        {
            buffer.copy(pos, src, cnt);
        }

        template<class T> void append (const T * array, size_t count)
        {
            append((const char*)array, count * (sizeof T));
        }

        void append (const char* src, size_t count)
        {
            if (count == 0)
                return;
            buffer.insert(w_pos, src, count);
            w_pos += count;
        }

        template<typename T> ByteBuffer& operator >> (Unused<T>)
        {
            read_skip<T>();
            return *this;
        }

        template <typename T> T read()
        {
            T r = read<T>(r_pos);
            r_pos += sizeof(T);
            return r;
        }

        template <typename T> T read(size_t pos) const
        {
            if (pos + sizeof(T) > size())
                assert(false);
                //throw ByteBufferException(false, pos, sizeof(T), size());
            T val = *((T const*)&buffer[pos]);
            return val;
        }

        void read(char *dest, size_t len)
        {
            if (r_pos  + len > size())
                assert(false);
                //throw ByteBufferException(false, _rpos, len, size());
            memcpy(dest, &buffer[r_pos], len);
            r_pos += len;
        }

        bool empty() const { return buffer.empty();}

        template<class T> void read_skip()
        {
            if (r_pos  + (sizeof T) > size())
                assert(false);
            r_pos += sizeof T;
        }

        size_t size() const { return buffer.size();}
    private:

        size_t w_pos;
        size_t r_pos;
    protected:
        MemBlock buffer;
    };
}
