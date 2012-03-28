#pragma once

#include "framework/typedefs.h"

namespace Movement
{
    typedef size_t AspectTypeId;
    class ComponentTree;

    struct EXPORT Component
    {
        private: ComponentTree* m_tree;
        private: void* m_this;
        private: AspectTypeId m_typeId;

        private: void* _getAspect(AspectTypeId objectTypeId) const;
        private: void _ComponentInit(void * me, AspectTypeId objectTypeId, ComponentTree * tree);
        private: void _ComponentAttach(void * object, AspectTypeId objectTypeId, Component * com);

        public: ComponentTree* Tree() const {
            return m_tree;
        }

        public: template<class T> T* getAspect() const {
            return (T*)_getAspect(T::getTypeId());
        }

        public: template<class T> T* as() const {
            return (T*)_getAspect(T::getTypeId());
        }

        public: explicit Component() : m_tree(0), m_this(0), m_typeId(0) {}

        public: virtual ~Component();

        public: template<class MyType> void ComponentInit(MyType * me) {
            _ComponentInit(me, MyType::getTypeId(), 0);
        }

        public: template<class T> void ComponentAttach(T * object) {
            _ComponentAttach(object, T::getTypeId(), object);
        }

        public: void ComponentDetach();
    };

#define COMPONENT_TYPEID \
    friend struct Component; \
    static AspectTypeId getTypeId() { \
        static char dummy; \
        return (size_t)&dummy; \
    }
}
