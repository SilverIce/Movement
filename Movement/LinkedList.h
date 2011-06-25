
/**
  file:         LinkedList.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

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

public:
    LinkedListElementBase() : next(0), prev(0) {}

    LinkedListElementBase * next;
    LinkedListElementBase * prev;

    bool linked() const { return next || prev;}

    void delink()
    {
        connect(*prev, *next);
        next = NULL;
        prev = NULL;
    }

    static void insert_between(LinkedListElementBase& prev, LinkedListElementBase& el, LinkedListElementBase& next)
    {
        connect(prev, el);
        connect(el, next);
    }

    static void connect(LinkedListElementBase& left, LinkedListElementBase& right)
    {
        left.next = &right;
        right.prev = &left;
    }

    // inserts element 'el' before 'me' element
    static void insert_before(LinkedListElementBase& me, LinkedListElementBase& el)
    {
        connect(*me.prev, el);
        connect(el, me);
    }

    // inserts element 'el' after 'me' element
    static void insert_after(LinkedListElementBase& me, LinkedListElementBase& el)
    {
        connect(me, el);
        connect(el, *me.next);
    }
};

template<class T> class LinkedList;

template<class T>
class LinkedListElement
{
    LinkedListElement(const LinkedListElement&);
    LinkedListElement& operator = (const LinkedListElement&);

    LinkedListElementBase base;
public:
    typedef typename T value_type;
    typedef LinkedListElement element_type;

    explicit LinkedListElement() {}

    T Value;

    bool linked() const { return base.linked();}

    void delink()
    {
        if (linked())
            base.delink();
    }

    friend class LinkedList<T>;
};

template<class T>
class LinkedList
{
    typedef LinkedListElementBase base_element;
public:

    typedef LinkedListElement<T> element_type;
    typedef typename element_type::value_type value_type;

    LinkedList()
    {
        base_element::connect(first, last);
    }

    void link(element_type & el)
    {
        base_element::insert_before(last, el.base);
    }

    void delink(element_type & el)
    {
        el.delink();
    }

    void delink_all()
    {
        base_element * i = first.next, *i2;
        while( i != &last)
        {
            i2 = i;
            i = i->next;
            i2->delink();
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
        base_element * i = first.next;
        while( i != &last)
        {
            value_type & t = ((element_type*)i)->Value;
            i = i->next;
            visitor(t);
        }
    }

    bool empty() const { return first.next == &last;}

private:
    base_element first;
    base_element last;

    LinkedList& operator = (const LinkedList&);
    LinkedList(const LinkedList&);
};

template<class T>
class MonoDirList
{
public:
    class element 
    {
        element * next;
        friend class MonoDirList;

        static void connect(element& left, element& right)
        {
            left.next = &right;
        }
    public:
        element() : next(NULL) {}
    };

private:

    element first;
    element * last;

    void delink(element& prev, element& delinked)
    {
        prev.next = delinked.next;
        delinked.next = NULL;
        if (last == &delinked)
            last = &prev;
    }

public:

    bool Empty() const { return first.next != NULL;}

    explicit MonoDirList() : last(&first) {}

    void Push(T& t)
    {
        element::connect(*last, t);
        last = &t;
    }

    T* Pop()
    {
        element * i = first.next;
        if (i)
            delink(first, *i);
        return (T*)i;
    }

    template<class Pred> T* remove_if(Pred pred)
    {
        element * i = first.next;
        element * prev = &first;
        while(i != 0)
        {
            if ( pred((T&)*i) )
            {
                delink(*prev, *i);
                return (T*)i;
            }
            prev = i;
            i = i->next;
        }
        return NULL;
    }
};

}
