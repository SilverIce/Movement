
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once
#include <vector>

class WorldPacket;

namespace Movement
{
    class MovementMessage;
    class UnitMovement;
    class ClientImpl;

    class Client
    {
        ClientImpl& m;
        Client(ClientImpl& client) : m(client) {}
        Client(const Client&);
        Client& operator = (const Client&);
    public:

        static Client* create(void * socket);
        ~Client();

        void LostControl();
        void SetControl(UnitMovement * mov);

        /** Receives movemet messages from all visible clients */
        void SendMoveMessage(MovementMessage& message) const;
        /** Handles messages from this client */
        void OnMovementMessage(WorldPacket& message);

        static void FillSubscribeList(std::vector<uint16>& opcodes);
    };
}
