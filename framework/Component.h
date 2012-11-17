#pragma once

#include "framework/typedefs.h"

class QString;
class QTextStream;

namespace Movement
{
    typedef void* AspectTypeId;
    class ComponentTree;

    struct EXPORT Component
    {
        private: ComponentTree* m_tree;
        private: AspectTypeId m_typeId;

        private: Component(const Component &);
        private: Component& operator = (const Component &);

        private: Component* _getAspect(AspectTypeId objectTypeId) const;
        private: Component& _as(AspectTypeId objectTypeId) const;
        private: void _ComponentInit(Component* me, AspectTypeId objectTypeId, ComponentTree * tree);
        private: void _ComponentAttach(AspectTypeId objectTypeId, Component * com);

        /** Returns true to show that both components belongs to same entity. */
        public: bool sameTree(const Component& another) const {
            return m_tree == another.m_tree;
        }

        /** Describes component by filling stream with text. */
        public: virtual void toString(QTextStream& stream) const;

        /** Describes all components that attached to the tree  */
        public: QString toStringAll() const;

        /** Performs a cast to given type. Returns a null in case cast failed.*/
        public: template<class T> inline typename T::HasTypeId::ComponentType* getAspect() const {
            return static_cast<typename T::HasTypeId::ComponentType*>( _getAspect(T::HasTypeId::getTypeId()) );
        }

        /** Performs a cast to given type. Asserts that cast never fails.*/
        public: template<class T> inline typename T::HasTypeId::ComponentType& as() const {
            return static_cast<typename T::HasTypeId::ComponentType&>( _as(T::HasTypeId::getTypeId()) );
        }

        public: explicit Component() : m_tree(0), m_typeId(0) {}

        /** Public virtual destructor is needed only in case it required to delete a component
            by having just a pointer to abstract Component. This is not my case. */
        protected: /*virtual*/ ~Component();

        public: template<class MyType> inline void ComponentInit(MyType * me) {
            (void)static_cast<typename MyType::HasTypeId::ComponentType*>(me);
            (void)static_cast<Component*>(me);
            _ComponentInit(me, MyType::HasTypeId::getTypeId(), 0);
        }

        public: template<class T> inline void ComponentAttach(T * object) {
            (void)static_cast<typename T::HasTypeId::ComponentType*>(object);
            (void)static_cast<Component*>(object);
            _ComponentAttach(T::HasTypeId::getTypeId(), object);
        }

        public: void ComponentDetach();
    };

    template<class T, class D> struct class_equality;
    template<class T> struct class_equality<T, T> {};

#define COMPONENT_TYPEID(TYPE) \
    friend struct ::Movement::Component; \
    struct HasTypeId { \
        typedef TYPE ComponentType; \
        EXPORT static ::Movement::AspectTypeId getTypeId() { \
            (void)sizeof(::Movement::class_equality<HasTypeId,typename ComponentType::HasTypeId>); \
            static char dummy; \
            return &dummy; \
        } \
    };
}
