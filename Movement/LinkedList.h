#pragma once

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

namespace Movement
{
class LinkedListElementBase
{
private:
    LinkedListElementBase(const LinkedListElementBase&);
    LinkedListElementBase& operator = (const LinkedListElementBase&);

protected:

    LinkedListElementBase() : next(0), prev(0) {}

    LinkedListElementBase * next;
    LinkedListElementBase * prev;

    bool linked() const { return next && prev;}

    void delink()
    {
        next->prev = prev;
        prev->next = next;

        next = NULL;
        prev = NULL;
    }

    static void insert_between(LinkedListElementBase& prev, LinkedListElementBase& el, LinkedListElementBase& next)
    {
        el.next = &next;
        el.prev = &prev;

        prev.next = &el;
        next.prev = &el;
    }

    static void connect(LinkedListElementBase& el_1, LinkedListElementBase& el_2)
    {
        el_1.next = &el_2;
        el_2.prev = &el_1;
    }

    // inserts element 'el' before 'me' element
    static void insert_before(LinkedListElementBase& me, LinkedListElementBase& el)
    {
        insert_between(*me.prev, el, me);
    }

    // inserts element 'el' after 'me' element
    static void insert_after(LinkedListElementBase& me, LinkedListElementBase& el)
    {
        insert_between(me, el, *me.next);
    }
};

template<class T> class LinkedList;

template<class T>
struct LinkedListElement : public LinkedListElementBase
{
    typedef typename T value_type;
    typedef LinkedListElement element_type;

    explicit LinkedListElement() {}

    ~LinkedListElement()
    {
        // delink();
    }

    T Value;

    T& operator *() { return Value;}
    const T& operator *() const { return Value;}

    operator bool () const { return linked();}

    void delink()
    {
        if (linked())
        {
            //Value.clean();
            LinkedListElementBase::delink();
        }
    }

private:

    friend class LinkedList<T>;

    element_type* Next() { return (element_type*)next;}
    element_type* Prev() { return (element_type*)prev;}
};

template<class T>
class LinkedList
{
public:

    typedef LinkedListElement<T> element_type;
    typedef typename element_type::value_type value_type;

    LinkedList()
    {
        element_type::connect(first, last);
    }

    void link(element_type & el)
    {
        element_type::insert_before(last, el);
    }

    void delink(element_type & el)
    {
        el.delink();
    }

    void delink_all()
    {
        element_type * i = first.Next(), * i2;
        element_type * end = &last;
        while( i != end)
        {
            i2 = i;
            i = i->Next();
            delink(*i2);
        }
    }

    template<class Func>
    void Iterate(Func functor)
    {
        Visit<Func>(functor);
    }

    template<class Visitor>
    void Visit(Visitor& visitor)
    {
        element_type * i = first.Next();
        element_type * end = &last;
        while( i != end)
        {
            value_type & t = i->Value;
            i = i->Next();
            visitor(t);
        }
    }

    bool empty() const { return first.Next() == &last;}

private:
    element_type first;
    element_type last;

    LinkedList& operator = (LinkedList&);
    LinkedList(LinkedList&);
};

}
