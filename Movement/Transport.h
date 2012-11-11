#pragma once

#include "framework/typedefs.h"
#include "framework/Component.h"
#include "movement/Location.h"

class WorldObject;
struct TaxiPathNodeEntry;

template<class> class QVector;
class QByteArray;

namespace Tasks {
    class ITaskExecutor;
}

namespace Movement
{
    /** For internal use only! Holds the info that helps unboard, delink passenger from the transport. */
    struct IPassenger : Component
    {
        virtual void Unboard() = 0;
        COMPONENT_TYPEID(IPassenger);
    };

    class Context;

    class EXPORT Transport
    {
        class MOTransportMover* m;

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
            Context* context;
            uint64 guid;
            MotionInfo motion;
            Location initialLocation;
        };

        explicit Transport(const Transport::CreateInfo& );
        ~Transport();

        uint32 timeLine();

        uint32 mapId();

        const Vector3& Position();

        // special position to append it into create packets
        const Vector3& initialPosition();

        QVector<Component*> Passengers();

        QByteArray WriteCreate();

        static void (*OnMapChanged)(Transport& transport, WorldObject* owner);
    };
}
