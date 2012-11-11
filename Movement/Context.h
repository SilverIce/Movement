#pragma once

#include <QtCore/QHash>
#include "framework/typedefs_p.h"
#include "TaskScheduler.h"
#include "ObjectGuid.h"

namespace Movement
{
    struct Component;

    class EntityRegistry
    {
        typedef QHash<uint64, Component*> Registry;
        Registry entityRegistry;

    public:
        void add(const ObjectGuid& guid, Component& object) {
            Registry::iterator it(entityRegistry.find(guid.GetRawValue()));
            assert_state( it == entityRegistry.end() );
            entityRegistry.insert(guid.GetRawValue(), &object);
        }

        void remove(const ObjectGuid& guid) {
            entityRegistry.remove(guid.GetRawValue());
        }

        template<class T> T* get(const ObjectGuid& guid) const {
            Component *component = get(guid);
            return component ? component->getAspect<T>() : nullptr;
        }

        Component* get(const ObjectGuid& guid) const {
            return entityRegistry.value(guid.GetRawValue());
        }
    };

    class Context
    {
    public:
        Tasks::TaskExecutor executor;
        EntityRegistry registry;
    };

    EXPORT inline Context* Context_create() { return new Context();}
    EXPORT inline void Context_update(Context* context, uint32 time) { context->executor.Execute(time);}
    EXPORT inline void Context_destroy(Context* context) { delete context;}
}
