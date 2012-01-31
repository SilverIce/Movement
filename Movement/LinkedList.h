
/**
  file:         LinkedList.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"

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

template<typename AbstractNode>
class LinkedListNodeBase
{
    friend class LinkedListNodeBase;
private:
    LinkedListNodeBase(const LinkedListNodeBase&);
    LinkedListNodeBase& operator = (const LinkedListNodeBase&);

protected:

    typedef typename AbstractNode* DerivedNode;

    LinkedListNodeBase() {
        next = prev = 0;
    }

    LinkedListNodeBase(DerivedNode prevNode, DerivedNode nextNode) {
        next = nextNode;
        prev = prevNode;
    }

    DerivedNode next;
    DerivedNode prev;

    bool linked() const { return next && prev;}

    void delink()
    {
        connect(prev, next);
        next = prev = 0;
    }

    static void insert_between(DerivedNode prev, DerivedNode el, DerivedNode next)
    {
        connect(prev, el);
        connect(el, next);
    }

    static void connect(DerivedNode left, DerivedNode right)
    {
        left->next = right;
        right->prev = left;
    }

    // inserts element 'el' before 'me' element
    static void insert_before(DerivedNode me, DerivedNode el)
    {
        connect(me->prev, el);
        connect(el, me);
    }

    // inserts element 'el' after 'me' element
    static void insert_after(DerivedNode me, DerivedNode el)
    {
        connect(me, el);
        connect(el, me->next);
    }
};

template<class T> class LinkedList;

template<class T>
class LinkedListElement : private LinkedListNodeBase<LinkedListElement<T> >
{
    typedef LinkedListNodeBase<LinkedListElement<T> > BasicType;
    typedef LinkedList<T> ListType;
public:

    explicit LinkedListElement() : _container(NULL) {
    }

    explicit LinkedListElement(T& value) : _container(NULL), Value(value) {
    }

    ~LinkedListElement() {
        assert_state( !linked() );
    }

    T Value;

    using BasicType::linked;

    LinkedListElement* Next() {
        return (_container != 0 && next != _container->_first) ? next : 0;
    }

    LinkedListElement* Previous() {
        return (_container != 0 && this != _container->_first) ? prev : 0;
    }

    LinkedList<T>& List() const { return *_container;}

    void delink() {
        if (_container)
            _container->delink(*this);
    }

private:

    friend class ListType;
    friend class BasicType;

    void InsertBetween(LinkedListElement* previousNode, LinkedListElement* nextNode, ListType* list)
    {
        ensureBlankNode();
        BasicType::insert_between(previousNode, this, nextNode);
        _container = list;
    }

    void SelfReference (ListType* list)
    {
        ensureBlankNode();
        next = this;
        prev = this;
        _container = list;
    }

    void Detach()
    {
        BasicType::delink();
        _container = NULL;
    }

    void ensureBlankNode() {
        assert_state(!linked() && !_container);
    }

private:
    ListType * _container;
};

template<class T >
class LinkedList
{
public:

    typedef LinkedListElement<T> element_type;
    typedef T value_type;

    LinkedList() : _first(NULL), _count(0) {
    }

    ~LinkedList() {
        assert_state( empty() );
    }

    element_type* first() {
        return _first;
    }

    element_type* last() {
        return (_first != 0) ? _first->prev : 0;
    }

    void link_first(element_type & node)
    {
        if (!_first)
            node.SelfReference(this);
        else
            node.InsertBetween(_first->prev, _first, this);
        _first = &node;
        ++_count;
    }

    void link_last(element_type & node)
    {
        if (!_first)
        {
            node.SelfReference (this);
            _first = &node;
        }
        else
            node.InsertBetween (_first->prev, _first, this);
        ++_count;
    }

    void link_after(element_type * node, element_type * newNode)
    {
        newNode->InsertBetween (node, node->next, this);
        ++_count;
    }

    void link_before(element_type * node, element_type * newNode)
    {
        newNode->InsertBetween (node->prev, node, this);
        if (node == _first)
            _first = newNode;
        ++_count;
    }

    void delink_first()
    {
        if (_first)
            delink(*_first);
    }

    void delink_last()
    {
        if (_first)
            delink(*_first->prev);
    }

    void delink(element_type & node)
    {
        assert_state(node.linked() && node._container == this);
        --_count;
        if (_count == 0)
            _first = NULL;
        if (&node == _first)
            _first = _first->next;
        node.Detach();
    }

    void clear()
    {
        while(!empty())
            delink_first();
    }

    template<class Func>
    void Iterate(Func functor)
    {
        Visit<Func>(functor);
    }

    template<class Visitor>
    void Visit(Visitor& visitor)
    {
        element_type * i = _first;
        while(i)
        {
            value_type & t = i->Value;
            i = i->Next();
            visitor(t);
        }
    }

    bool empty() const { return _first == NULL;}
    uint32 size() const { return _count;}

private:

    friend class element_type;

    element_type * _first;
    uint32 _count;

    LinkedList& operator = (const LinkedList&);
    LinkedList(const LinkedList&);
};

}
