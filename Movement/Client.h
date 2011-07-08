
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once

#include <string>

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

        /** Client's lifetime bounded to WorldSession lifetime */
        static Client* create(void * socket);
        ~Client();

        ClientImpl& Impl() { return m;}

        std::string ToString() const;

        void LostControl();
        void SetControl(UnitMovement * mov);

        void HandleResponse(WorldPacket& data);

        /** Handles messages from another clients */
        void SendMoveMessage(MovementMessage& msg) const;
        /** Handles messages from that client */
        void HandleOutcomingMessage(WorldPacket& recv_data);

        void HandleMoveTimeSkipped(WorldPacket & recv_data);
    };
}
