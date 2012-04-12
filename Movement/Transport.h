#pragma once

#include "framework/typedefs.h"
#include "framework/Component.h"

class WorldObject;
struct TaxiPathNodeEntry;

namespace Tasks {
    class ITaskExecutor;
}

namespace Movement
{
    /** Holds the info that helps unboard, delink passenger from the transport. */
    struct IPassenger : Component
    {
        virtual void Unboard() = 0;
        COMPONENT_TYPEID;
    };

    class EXPORT Transport
    {
        class TransportImpl* m;

        Transport& operator = (const Transport&);
        Transport(const Transport&);
    public:
        struct MotionInfo
        {
            const TaxiPathNodeEntry * nodes;
            uint32 nodesSize;

            float velocity;
            float acceleration;
        };

        struct CreateInfo
        {
            WorldObject* object;
            Tasks::ITaskExecutor& executor;
            uint64 guid;
            MotionInfo motion;
        };

        explicit Transport(const Transport::CreateInfo& );
        ~Transport();
    };
}
