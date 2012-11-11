
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once
#include "framework/typedefs.h"

template<typename> class QVector;

namespace Movement
{
    class Context;
    class MovementMessage;
    struct PacketData;

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

        static Client* create(void * socket, Context &context);
        void dealloc();

        /** Receives movement messages from all visible clients */
        void SendMoveMessage(MovementMessage& message) const;
        /** Handles messages from this client */
        void OnMovementMessage(const PacketData& message);

        static void FillSubscribeList(QVector<uint16>& opcodes);
    };

    struct PacketData
    {
        const char *bytes;
        uint32 byteCount;
        uint16 opcode;

        PacketData(uint16 Opcode, const char* Bytes, uint32 ByteCount)
            : bytes(Bytes), byteCount(ByteCount), opcode(Opcode)
        {}
    };
}
