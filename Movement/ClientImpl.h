
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once

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
        MoveUpdater * m_updater;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        UInt32Counter request_counter;
        //int32 time_skipped;
        typedef std::list<RespHandler*> RespHdlContainer;
        RespHdlContainer m_resp_handlers;

        static MSTime ServerTime() { return MSTime(MaNGOS_API::getMSTime());}
        MSTime ServerToClientTime(const MSTime& server_time) const { return server_time + m_time_diff;}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime());}
        MSTime ClientToServerTime(const MSTime& client_time) const { return client_time - m_time_diff;}

    public:

        TaskTarget commonTasks;

        MoveUpdater& Updater() const { return *m_updater;}
        UnitMovementImpl* controlled() const { return m_controlled;}
        void SetClientTime(const MSTime& client_time) { m_time_diff = client_time - ServerTime();}

        void QueueState(ClientMoveStateChange& client_state)
        {
            struct ApplyStateTask {
                UnitMovementImpl * owner;
                ClientMoveStateChange state;

                ApplyStateTask(UnitMovementImpl * own, const ClientMoveStateChange& client_state)
                    : state(client_state), owner(own) {}

                STATIC_EXEC(ApplyStateTask, TaskExecutor_Args&){
                    owner->ApplyState(state.state);
                    if (state.floatValueType != Parameter_End)
                        owner->SetParameter(state.floatValueType, state.floatValue);
                }
            };
            client_state.state.ms_time = ClientToServerTime(client_state.state.ms_time);
            m_controlled->Updater().AddTask(new ApplyStateTask(m_controlled,client_state),
                client_state.state.ms_time, m_controlled->commonTasks);
        }

        void RegisterRespHandler(RespHandler* handler);
        void UnregisterRespHandler(RespHandler* handler);
        void Kick() {}  // not implemented

        inline void BroadcastMessage(MovementMessage& msg) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, msg);}
        inline void BroadcastMessage(WorldPacket& data) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, data);}
        inline void SendPacket(const WorldPacket& data) const { MaNGOS_API::SendPacket(m_socket, data);}

        void CleanReferences();
        void Dereference(const UnitMovementImpl * m);
        #pragma endregion
    public:

        /** Client's lifetime bounded to WorldSession lifetime */
        ClientImpl(HANDLE socket);
        ~ClientImpl();

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
    private:
        ClientImpl* m_client;
        uint32 m_opcode;
        bool m_wasHandled;
    protected:
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) = 0;

        bool checkRequestId(uint32 requestId) const {
            if (requestId != m_reqId) {
                log_function("wrong request Id: %u expected Id: %u", requestId, m_reqId);
                return false;
            }
            return true;
        }
    public:
        uint32 m_reqId;

        enum{
            /* Default timeout value, milliseconds */
            DefaultTimeout = 500,
        };

        explicit RespHandler(uint32 _opcode, ClientImpl * client) : m_opcode(_opcode), m_client(client), m_wasHandled(false)
        {
            client->RegisterRespHandler(this);
        }

        ~RespHandler() {
            m_client->UnregisterRespHandler(this);
        }

        bool OnReply(WorldPacket& data)
        {
            if (m_opcode != data.GetOpcode()) {
                log_function("expected reply was: %s, but received instead: %s", LookupOpcodeName(m_opcode), LookupOpcodeName(data.GetOpcode()));
                return false;
            }
            m_wasHandled = OnReply(m_client, data);
            return m_wasHandled;
        }

        STATIC_EXEC(RespHandler, TaskExecutor_Args& args){
            if (!m_wasHandled) {
                log_function("Kick client due to response(opcode: %s) timeout", LookupOpcodeName(m_opcode));
                m_client->Kick();
            }
        }
    };
}
