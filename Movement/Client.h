
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once
#include "framework/typedefs.h"

class WorldPacket;
template<typename> class QVector;

template<class> class QVector;

namespace Movement
{
    class MovementMessage;

    /** Represents an object that controls UnitMovement objects.
        The main purpose - handle incoming movement packets
        (packets, that change controlled object state, for example, position).
    */
    class EXPORT Client
    {
        friend struct ClientMemoryLayout;
        Client(const Client&);
        Client& operator = (const Client&);
        Client() {}
        ~Client() {}
    public:

        static Client* create(void * socket);
        void dealloc();

        /** Receives movement messages from all visible clients */
        void SendMoveMessage(MovementMessage& message) const;
        /** Handles messages from this client */
        void OnMovementMessage(WorldPacket& message);

        static void FillSubscribeList(QVector<uint16>& opcodes);
    };
}
