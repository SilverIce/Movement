
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

template<class DerivedType>
class LinkedListNodeBase
{
private:
    LinkedListNodeBase(const LinkedListNodeBase&);
    LinkedListNodeBase& operator = (const LinkedListNodeBase&);

protected:

    LinkedListNodeBase() {
        next = prev = 0;
    }

    DerivedType* next;
    DerivedType* prev;

    bool linked() const { return next && prev;}

    void delink()
    {
        connect(prev, next);
        next = prev = 0;
    }

    static void insert_between(DerivedType* prev, DerivedType* el, DerivedType* next)
    {
        connect(prev, el);
        connect(el, next);
    }

    static void connect(DerivedType* left, DerivedType* right)
    {
        left->next = right;
        right->prev = left;
    }

    // inserts element 'el' before 'me' element
    static void insert_before(DerivedType* me, DerivedType* el)
    {
        connect(me->prev, el);
        connect(el, me);
    }

    // inserts element 'el' after 'me' element
    static void insert_after(DerivedType* me, DerivedType* el)
    {
        connect(me, el);
        connect(el, me->next);
    }
};

template<class,class> class LinkedList;

template<class T>
class LinkedListElement : private LinkedListNodeBase<LinkedListElement<T> >
{
    typedef LinkedListNodeBase<LinkedListElement<T> > BasicType;
    typedef LinkedList<T, LinkedListElement<T> > ListType;
public:

    typedef T value_type;

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

    ListType& List() const { return *_container;}

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
        insert_between(previousNode, this, nextNode);
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

    void ensureLinkedWith(ListType * list) {
        assert_state(linked() && _container == list);
    }

private:
    ListType * _container;
};

template<class T, class NodeType = LinkedListElement<T> >
class LinkedList
{
public:

    typedef NodeType element_type;
    typedef typename NodeType::value_type value_type;

    LinkedList() : _first(NULL), _count(0) {
    }

    ~LinkedList() {
        assert_state( empty() );
    }

    element_type* first() const {
        return _first;
    }

    element_type* last() const {
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
        node.ensureLinkedWith(this);
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
            i = i->Next();
            visitor(t);
        }
    }

    bool empty() const { return _first == NULL;}
    uint32 size() const { return _count;}

private:

    friend typename NodeType;

    element_type * _first;
    uint32 _count;

    LinkedList& operator = (const LinkedList&);
    LinkedList(const LinkedList&);
};
}
