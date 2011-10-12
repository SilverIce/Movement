#pragma once

#include <memory>
#include <algorithm>

namespace Tasks
{
#   ifdef  TASKSCHEDULER_DEBUGGING
#       define pod_assert(expr) mov_assert(expr)
#   else
#       define pod_assert(expr)
#   endif

    /** Efficient container desinged specially for POD types.
        It never deallocates memory when clear, erase methods called*/
    template<class T>
    struct POD_Array
    {
    private:
        T * _elements;
        size_t _count;
        size_t _capacity;

    public:

        typedef T* iterator;
        typedef T const* const_iterator;
        typedef T& reference;
        typedef const T& const_reference;

        struct reverse_iterator
        {
            T * ptr;

            explicit reverse_iterator(iterator it) : ptr(it) {}

            void operator ++() { --ptr;}
            void operator --() { ++ptr;}

            reference operator *() { return *ptr;}

            bool operator == (const reverse_iterator& it) const {
                return ptr == it.ptr;
            }

            bool operator != (const reverse_iterator& it) const {
                return ptr != it.ptr;
            }

            reverse_iterator operator + (size_t difference) const {
                reverse_iterator it(ptr - difference);
                return it;
            }
            reverse_iterator operator - (size_t difference) const {
                reverse_iterator it(ptr + difference);
                return it;
            }
        };

        enum{
            elSize = sizeof(T),
        };

        POD_Array(size_t capacity = 0) {
            _elements = NULL;
            _count = 0;
            _capacity = 0;
            reserve(std::max<size_t>(3,capacity));
        }

        ~POD_Array() {
            delete _elements;
            _elements = NULL;
            _count = 0;
            _capacity = 0;
        }

        bool empty() const { return _count == 0;}
        size_t size() const { return _count;}
        size_t capacity() const { return _capacity;}
        iterator begin() { return _elements;}
        iterator end() { return _elements+_count;}
        const_iterator begin() const { return _elements;}
        const_iterator end() const { return _elements+_count;}
        reverse_iterator rbegin() { return reverse_iterator(end()-1);}
        reverse_iterator rend() { return reverse_iterator(begin()-1);}

        reference front() { return *begin();}
        reference back() { return *(end()-1);}
        const_reference front() const { return *begin();}
        const_reference back() const { return *(end()-1);}

        reference operator[] (size_t i) { return *(begin()+i);}
        const_reference operator[] (size_t i) const { return *(begin()+i);}

        void clear() { resize(0); }

        void push_back(const T& value) {
            insert(end(), value);
        }

        void insert(const_iterator at, const T& value)
        {
            _assertInRange(at);
            if (_capacity > _count) {
                if (at != end())
                    memmove((iterator)(at + 1), at, (end() - at) * elSize);  // move right block -->
                memcpy((iterator)at, &value, elSize);
                ++_count;
            }
            else {
                size_t newCapacity = _capacity * 3 / 2;
                T * mem = allocate(newCapacity);
                if (at != begin())
                    memcpy(mem, begin(), (at - begin()) * elSize);       // copy left part
                memcpy(mem + (at - begin()), &value, elSize);
                if (at != end())
                    memcpy(mem + (at - begin())+1, at, (end() - at) * elSize); // copy right part

                setElements(mem, newCapacity);
                ++_count;
            }
        }

        void assign(const_iterator beg, const_iterator end) {
            _assertNotInRange(beg);
            _assertNotInRange(end);
            pod_assert(beg <= end);
            size_t copySize = end - beg;
            if (copySize > _capacity) {
                T * mem = allocate(copySize);
                setElements(mem, copySize);
            }
            _count = copySize;
            memcpy(_elements, beg, copySize * elSize);
        }

        void reserve(size_t newCapacity) {
            if (newCapacity > _capacity) {
                T * mem = allocate(newCapacity);
                memcpy(mem, begin(), size() * elSize);       // copy left part
                setElements(mem, newCapacity);
            }
        }

        void resize(size_t newSize) {
            if (newSize > _capacity)
                reserve(newSize);
            _count = newSize;
        }

        void pop_back() {
            erase(end());
        }

        void erase(const_iterator at) {
            pod_assert(!empty());
            _assertInRange(at);
            if (at != end())
                memmove((iterator)at, at + 1, (end() - at) * elSize);
            --_count;
        }

        void erase(const_iterator first, const_iterator last) {
            pod_assert(!empty());
            _assertInRange(first);
            _assertInRange(last);
            pod_assert(first <= last);
            if (last != end())
                memmove((iterator)first, last, (end() - last) * elSize);
            _count -= (last - first);
        }

        void swap(POD_Array& other) {
            std::swap(_elements, other._elements);
            std::swap(_count, other._count);
            std::swap(_capacity, other._capacity);
        }

    private:
        POD_Array(const POD_Array&);
        POD_Array& operator = (const POD_Array&);

        static T* allocate(size_t elementCount) {
            return (T*)operator new((elementCount+1) * elSize);
        }

        void setElements(T * elems, size_t elementCount) {
            delete _elements;
            _elements = elems;
            _capacity = elementCount;
        }

        inline void _assertInRange(const_iterator at) const {
            pod_assert(at >= begin() && at <= end());
        }

        inline void _assertNotInRange(const_iterator at) const {
            pod_assert(at < begin() || at > end());
        }
    };

}
