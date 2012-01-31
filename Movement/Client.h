
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once
#include <vector>
#include "framework/typedefs.h"

class WorldPacket;

namespace Movement
{
    class MovementMessage;
    class UnitMovement;
    class ClientImpl;

    class EXPORT Client
    {
        ClientImpl& m;
        Client(ClientImpl& client) : m(client) {}
        Client(const Client&);
        Client& operator = (const Client&);
        ~Client() {}
    public:

        static Client* create(void * socket);
        void dealloc();

        /** Receives movemet messages from all visible clients */
        void SendMoveMessage(MovementMessage& message) const;
        /** Handles messages from this client */
        void OnMovementMessage(WorldPacket& message);

        static void FillSubscribeList(std::vector<uint16>& opcodes);
    };
}
