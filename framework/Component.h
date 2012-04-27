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
        private: void* m_this;
        private: AspectTypeId m_typeId;

        private: Component(const Component &);
        private: Component& operator = (const Component &);

        private: void* _getAspect(AspectTypeId objectTypeId) const;
        private: void* _as(AspectTypeId objectTypeId) const;
        private: void _ComponentInit(void * me, AspectTypeId objectTypeId, ComponentTree * tree);
        private: void _ComponentAttach(void * object, AspectTypeId objectTypeId, Component * com);

        /** Returns true to show that both components belongs to same entity. */
        public: bool sameTree(const Component& another) const {
            return m_tree == another.m_tree;
        }

        public: virtual void toString(QTextStream& st) const;

        /** Describes all components that attached to the tree  */
        public: QString toStringAll() const;

        /** Performs a cast to given type. Returns a null in case cast failed.*/
        public: template<class T> inline typename T::HasTypeId::ComponentType* getAspect() const {
            return (typename T::HasTypeId::ComponentType*)_getAspect(T::HasTypeId::getTypeId());
        }

        /** Performs a cast to given type. Asserts that cast never fails.*/
        public: template<class T> inline typename T::HasTypeId::ComponentType& as() const {
            return *(typename T::HasTypeId::ComponentType*)_as(T::HasTypeId::getTypeId());
        }

        public: explicit Component() : m_tree(0), m_this(0), m_typeId(0) {}

        public: virtual ~Component();

        public: template<class MyType> inline void ComponentInit(MyType * me) {
            _ComponentInit(static_cast<typename MyType::HasTypeId::ComponentType*>(me), MyType::HasTypeId::getTypeId(), 0);
        }

        public: template<class T> inline void ComponentAttach(T * object) {
            _ComponentAttach(static_cast<typename T::HasTypeId::ComponentType*>(object), T::HasTypeId::getTypeId(), object);
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
