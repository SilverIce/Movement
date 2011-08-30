
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once

#include "typedefs.h"

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    class MovementMessage;
    class UnitMovementImpl;
    class RespHandler;

    class ClientImpl
    {
        #pragma region Impl
    private:
        void * m_socket;
        UnitMovementImpl * m_controlled;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        MSTime m_next_sync_time;
        UInt32Counter request_counter;
        //int32 time_skipped;
        typedef std::list<RespHandler*> RespHdlContainer;
        RespHdlContainer m_resp_handlers;

        static MSTime ServerTime() { return MSTime(getMSTime());}
        MSTime ServerToClientTime(const MSTime& server_time) const { return server_time + m_time_diff;}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime());}
        MSTime ClientToServerTime(const MSTime& client_time) const { return client_time - m_time_diff;}

    public:

        UnitMovementImpl* controlled() const { return m_controlled;}
        void SetClientTime(const MSTime& client_time) { m_time_diff = client_time - ServerTime();}

        void QueueState(ClientMoveState& client_state)
        {
            client_state.ms_time = ClientToServerTime(client_state.ms_time);
            m_controlled->_QueueState(client_state);
        }

        void AddRespHandler(RespHandler* req);
        void Kick() {}  // not implemented

        inline void BroadcastMessage(MovementMessage& msg) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, msg);}
        inline void BroadcastMessage(WorldPacket& data) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, data);}
        inline void SendPacket(const WorldPacket& data) const { MaNGOS_API::SendPacket(m_socket, data);}

        void CleanReferences();
        void Dereference(const UnitMovementImpl * m);
        void _OnUpdate();
        #pragma endregion
    public:

        /** Client's lifetime bounded to WorldSession lifetime */
        ClientImpl(HANDLE socket);

        ~ClientImpl()
        {
            CleanReferences();
        }

        std::string ToString() const;

        void LostControl();
        void SetControl(UnitMovementImpl * mov);

        void HandleResponse(WorldPacket& data);

        /** Handles messages from another clients */
        void SendMoveMessage(MovementMessage& msg) const;
        /** Handles messages from that client */
        void HandleOutcomingMessage(WorldPacket& recv_data);

        void HandleMoveTimeSkipped(WorldPacket & recv_data);
    };

    class RespHandler
    {
    protected:
        uint32 m_opcode;
    public:
        uint32 m_reqId;
        MSTime Timeout;

        enum{
            /* Default timeout value, milliseconds */
            DefaultTimeout = 500,
        };

        explicit RespHandler(uint32 _opcode, ClientImpl * client) : m_opcode(_opcode)
        {
            client->AddRespHandler(this);
        }

        bool CanHandle(uint32 opcode) const { return m_opcode == opcode;}
        //virtual void OnTimeout() {}
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) = 0;
    };
}
