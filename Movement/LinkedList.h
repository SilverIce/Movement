#pragma once

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

namespace Movement{

template<class T> class LinkedList;

template<class T>
struct LinkedListElement
{
    explicit LinkedListElement() : next(0), prev(0), m_obj(0) {}

    T& ref() { return *m_obj;}
    const T& ref() const { return *m_obj;}

    T& operator *() { return *m_obj;}
    const T& operator *() const { return *m_obj;}

    operator bool () const { return m_obj;}

private:
    friend class LinkedList<T>;

    T * m_obj;

    LinkedListElement * next;
    LinkedListElement * prev;
};

template<class T>
class LinkedList
{
public:

    typedef LinkedListElement<T> element_type;
    typedef LinkedListElement<T>* iterator;

    LinkedList() : m_size(0)
    {
         first.next = &last;
         last.prev = &first;
    }

    void link(element_type & el, T& obj)
    {
        element_type * prev = last.prev;
        element_type * next = &last;

        prev->next = &el;
        next->prev = &el;

        el.prev = prev;
        el.next = next;
        el.m_obj = &obj;

        ++m_size;
    }

    void delink(element_type & el)
    {
        element_type * next = el.next;
        element_type * prev = el.prev;

        prev->next = next;
        next->prev = prev;

        el.next = el.prev = NULL;
        el.m_obj = NULL;

        --m_size;
    }

    void delink_all()
    {
        element_type * i = first.next;
        element_type * end = &last;
        while( i != &last)
            delink(*i);
    }

    template<class Func>
    void Iterate(Func functor)
    {
        element_type * i = first.next;
        element_type * end = &last;
        while( i != &last)
        {
            T & t = i->ref();
            i = i->next;
            functor(t);
        }
    }

    template<class Visitor>
    void Visit(Visitor& visitor)
    {
        element_type * i = first.next;
        element_type * end = &last;
        while( i != &last)
        {
            T & t = i->ref();
            i = i->next;
            visitor(t);
        }
    }

    bool empty() const { return m_size == 0;}

    size_t size() const { return m_size;}

    const element_type* begin() const { return first.next; }
    element_type* begin() { return first.next; }

    const element_type* end() const { return &last; }
    element_type* end() { return &last; }

private:
    element_type first;
    element_type last;
    size_t m_size;
};

}
