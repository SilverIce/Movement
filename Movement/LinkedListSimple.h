
/**
  file:         LinkedList.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"

namespace Movement
{
    template<class> class LinkedListSimple;

    template<class T>
    class LinkedListElementSimple
    {
        friend class LinkedListSimple<T>;

        LinkedListElementSimple * next;
        LinkedListElementSimple * prev;
    public:

        typedef T value_type;

        explicit LinkedListElementSimple() : next(0), prev(0) {
        }

        explicit LinkedListElementSimple(const T& value) : next(0), prev(0), Value(value) {
        }

        ~LinkedListElementSimple() {
            assert_state( !linked() );
        }

        T Value;

        LinkedListElementSimple* Prev() const {
            return prev;
        }
        LinkedListElementSimple* Next() const {
            return next;
        }

        bool linked() const {
            return next || prev;
        }
    };

    template<class T>
    class LinkedListSimple
    {
    public:

        typedef LinkedListElementSimple<T> element_type;
        typedef typename element_type::value_type value_type;

        LinkedListSimple() : _first(0) {
        }

        ~LinkedListSimple() {
            assert_state( empty() );
        }

        element_type* first() const {
            return _first;
        }

        void link_first(element_type & node)
        {
            assert_state(!node.linked());
            if (_first) {
                _first->prev = &node;
                node.next = _first;
            }
            _first = &node;
        }

        void link_before(element_type& node, element_type & newNode)
        {
            assert_state(!newNode.linked());
            assert_state(!empty());

            element_type& next = node;
            element_type* prev = node.prev;
            next.prev = &newNode;
            newNode.next = &next;
            if (prev) {
                prev->next = &newNode;
                newNode.prev = prev;
            }
            
            if (_first == &node)
                _first = &newNode;
        }

        void link_after(element_type& node, element_type & newNode)
        {
            assert_state(!newNode.linked());
            assert_state(!empty());

            element_type& prev = node;
            element_type* next = node.next;
            prev.next = &newNode;
            newNode.prev = &prev;
            if (next) {
                next->prev = &newNode;
                newNode.next = next;
            }
        }

        void delink_first()
        {
            if (_first)
                delink(*_first);
        }

        void delink(element_type & node)
        {
            assert_state(!empty());
            element_type* next = node.next;
            element_type* prev = node.prev;
            if (prev)
                prev->next = next;
            if (next)
                next->prev = prev;
            node.next = 0;
            node.prev = 0;
            if (&node == _first)
                _first = next;
        }

        void clear()
        {
            while(!empty())
                delink_first();
        }

        template<class Func>
        void Iterate(Func functor) const
        {
            Visit<Func>(functor);
        }

        template<class Visitor>
        void Visit(Visitor& visitor) const
        {
            element_type * i = _first;
            while(i)
            {
                value_type & t = i->Value;
                i = i->next;
                visitor(t);
            }
        }

        bool empty() const { return _first == 0;}

        uint32 count() const {
            uint32 _count = 0;
            element_type * node = first();
            while(node) {
                ++_count;
                node = node->next;
            }
            return _count;
        }

    private:

        friend typename element_type;

        element_type * _first;

        LinkedListSimple& operator = (const LinkedListSimple&);
        LinkedListSimple(const LinkedListSimple&);
    };
}
