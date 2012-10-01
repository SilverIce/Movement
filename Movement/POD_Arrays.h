#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include "framework/typedefs_p.h"

namespace Tasks { namespace detail
{
    struct MemBlock 
    {
        char * _data;
        size_t _size;
        size_t _capacity;

        MemBlock() {
            _data = NULL;
            _size = 0;
            _capacity = 0;
        }

        ~MemBlock() {
            delete _data;
            _data = NULL;
            _size = 0;
            _capacity = 0;
        }

        size_t size() const {
            return _size;
        }

        bool empty() const {
            return _size == 0;
        }

        size_t capacity() const {
            return _capacity;
        }

        char& operator [] (size_t index) {
            _assertInRange(index);
            return *(_data + index);
        }

        const char& operator [] (size_t index) const {
            _assertInRange(index);
            return *(_data + index);
        }

        void resize(size_t size) {
            reserve(size);
            _size = size;
        }

        void clear() {
            _size = 0;
        }

        void reserve(size_t capacity) {
            if (capacity > _capacity) {
                char * mem = (char*)operator new(capacity);
                if (_data != nullptr)
                    memcpy(mem, _data, _size);
                setElements(mem, capacity);
            }
        }

        void push(const char * lowBound, size_t size) {
            push(lowBound - _data, size);
        }

        void push(size_t lowBound, size_t size) {
            _assertInRange(lowBound);
            if ((_size + size) > _capacity)
            {
                size_t newCapacity = std::max<size_t>((_size + size) * 3 / 2, 1);
                char * mem = (char*)operator new(newCapacity);

                memcpy(mem, _data, lowBound);       // copy left part
                memcpy(mem + lowBound + size, _data + lowBound, _size - lowBound);

                setElements(mem, newCapacity);
            }
            else if (lowBound == _size) {
                // push_back case
            }
            else {
                memmove(_data + lowBound + size, _data + lowBound, _size - lowBound);
            }

            _size += size;
        }

        /*static void memmove(void * dst, const void * src, size_t _Size)
        {
            if (dst > src)
            {
                size_t wnd = dst - src;
                dst = dst + _Size;
                src = src + _Size;

                do {
                    dst -= wnd;
                    scr -= wnd;
                    memcpy(dst, src, wnd);
                } while()
            }
            

        }*/

        void insert(const char * lowBound, const char* data, size_t dataSize) {
            _assertNotInRange(_data);
            insert(lowBound - _data, data, dataSize);
        }

        void insert(size_t lowBound, const char* data, size_t dataSize) {
            push(lowBound, dataSize);
            memcpy(_data + lowBound, data, dataSize);
        }

        void erase(size_t lowBound, size_t count) {
            _assertInRange(lowBound);
            size_t hiBound = lowBound + count;
            if (hiBound < _size)
                memmove(_data + lowBound, _data + hiBound, _size - hiBound);
            _size -= count;
        }

        void setElements(char * elems, size_t capacity) {
            delete _data;
            _data = elems;
            _capacity = capacity;
        }

        void swap(MemBlock& other) {
            std::swap(_data, other._data);
            std::swap(_size, other._size);
            std::swap(_capacity, other._capacity);
        }

        inline void _assertInRange(const char *  at) const {
            assert_state(at >= _data && at <= (_data + _size));
        }

        inline void _assertNotInRange(const char *  at) const {
            assert_state(at < _data || at > (_data + _size));
        }

        inline void _assertInRange(size_t at) const {
            assert_state(at <= _size);
        }
    };

    /** Efficient container desinged specially for POD types.
        It never deallocates memory when clear, erase methods called*/
    template<class T>
    struct POD_Array
    {
    private:
        MemBlock block;

    public:

        typedef T* iterator;
        typedef const T* const_iterator;
        typedef T& reference;
        typedef const T& const_reference;

        template<class Iterator>
        struct ReverseIterator 
        {
            Iterator ptr;

            explicit ReverseIterator(Iterator it) : ptr(it) {}

            void operator ++() { --ptr;}
            void operator --() { ++ptr;}

            reference operator *() { return *ptr;}
            const_reference operator *() const { return *ptr;}

            bool operator == (const ReverseIterator& it) const {
                return ptr == it.ptr;
            }

            bool operator != (const ReverseIterator& it) const {
                return ptr != it.ptr;
            }

            ReverseIterator operator + (size_t difference) const {
                ReverseIterator it(ptr - difference);
                return it;
            }
            ReverseIterator operator - (size_t difference) const {
                ReverseIterator it(ptr + difference);
                return it;
            }
        };
        typedef ReverseIterator<iterator> reverse_iterator;
        typedef ReverseIterator<const_iterator> const_reverse_iterator;

        enum{
            elSize = sizeof(T),
        };

        explicit POD_Array(size_t size = 0) {
            resize(size);
        }

        POD_Array(const POD_Array& other) {
            assign(other.begin(), other.end());
        }

        explicit POD_Array(iterator beg, iterator end) {
            assign(beg, end);
        }

        explicit POD_Array(size_t count, const_reference value) {
            block.resize(count * elSize);
            //std::fill_n(begin(), count, value);
        }

        /*template<class Itr> explicit POD_Array(Itr beg, Itr end) {
            resize(end - beg);
            std::copy(beg, end, begin());
        }*/

        ~POD_Array() {
        }

        bool empty() const { return block._size == 0;}
        size_t size() const { return block._size / elSize;}
        size_t capacity() const { return block._capacity / elSize;}
        iterator begin() { return (iterator)block._data;}
        iterator end() { return begin() + size();}
        const_iterator begin() const { return (const_iterator)block._data;}
        const_iterator end() const { return begin() + size();}
        reverse_iterator rbegin() { return reverse_iterator(end()-1);}
        reverse_iterator rend() { return reverse_iterator(begin()-1);}
        const_reverse_iterator rbegin() const { return const_reverse_iterator(end()-1);}
        const_reverse_iterator rend() const { return const_reverse_iterator(begin()-1);}

        reference front() { return *begin();}
        reference back() { return *(end()-1);}
        reference at(size_t pos) { return *(begin()+pos);}
        const_reference at(size_t pos) const { return *(begin()+pos);}
        const_reference front() const { return *begin();}
        const_reference back() const { return *(end()-1);}

        reference operator[] (size_t i) { return *(begin()+i);}
        const_reference operator[] (size_t i) const { return *(begin()+i);}

        void clear() { block.clear();}

        void push_back(const T& value) {
            insert(end(), value);
        }

        iterator insert(const_iterator at, const T& value)
        {
            _assertInRange(at);
            size_t insertPos = at - begin();
            block.insert(insertPos * elSize, (const char*)&value, elSize);
            return begin() + insertPos;
        }

     /*   void insert(const_iterator at, size_t n, const T& value) {
            reserve(size() + n);
            //std::fill_n();
        }*/

#pragma warning (disable:4996)
        template<class Itr>
        void insert(const_iterator at, Itr beg, Itr end) {
            block.push((at - begin())*elSize, (end-beg)*elSize);
            std::copy(beg, end, begin());
        }
#pragma warning (default:4996)

        void assign(const_iterator beg, const_iterator end) {
            _assertNotInRange(beg);
            _assertNotInRange(end);
            assert_state(beg <= end);

            block.clear();
            block.insert((size_t)0, (const char *)beg, (end - beg) * elSize);
        }

        void reserve(size_t newCapacity) {
            block.reserve(newCapacity * elSize);
        }

        void resize(size_t newSize) {
            block.resize(newSize * elSize);
        }

        void pop_back() {
            erase(end());
        }

        iterator erase(const_iterator at) {
            assert_state(!empty());
            _assertInRange(at);
            block.erase((at - begin())*elSize, elSize);
            return (iterator)at;
        }

        void erase(const_iterator first, const_iterator last) {
            _assertInRange(first);
            _assertInRange(last);
            assert_state(first <= last);
            block.erase((first - begin())*elSize, (last - first) * elSize);
        }

        void swap(POD_Array& other) {
            block.swap(other.block);
        }

        void operator = (const POD_Array& other) {
            assign(other.begin(), other.end());
        }

    private:

        inline void _assertInRange(const_iterator at) const {
            assert_state(at >= begin() && at <= end());
        }

        inline void _assertNotInRange(const_iterator at) const {
            assert_state(at < begin() || at > end() || at == 0);
        }
    };
}
}
