
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
    private:
        void * m_socket;
        UnitMovementImpl * m_controlled;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        UInt32Counter request_counter;
        //int32 time_skipped;
        typedef std::list<RespHandler*> RespHdlContainer;
        RespHdlContainer m_resp_handlers;

        static MSTime ServerTime() { return MSTime(Imports.getMSTime());}
        MSTime ServerToClientTime(const MSTime& server_time) const { return server_time + m_time_diff;}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime());}
        MSTime ClientToServerTime(const MSTime& client_time) const { return client_time - m_time_diff;}

    public:

        void assertControlled() const {
            assertInitialized();
            assert_state(m_controlled && m_controlled->client() == this);
        }
        void assertInitialized() const {
            assert_state(m_socket);
            assert_state(commonTasks.hasExecutor());
        }


        TaskTarget_DEV commonTasks;

        UnitMovementImpl* controlled() const { return m_controlled;}
        void SetClientTime(const MSTime& client_time) { m_time_diff = client_time - ServerTime();}

        /** The main 'gate' for movement states that incomes from client. */
        void QueueState(ClientMoveStateChange& client_state);

        uint32 RegisterRespHandler(RespHandler* handler);
        RespHandler* PopRespHandler();
        void Kick() {}  // not implemented

        inline void BroadcastMessage(MovementMessage& msg) const { Imports.BroadcastMoveMessage(&m_controlled->Owner, msg);}
        inline void BroadcastMessage(WorldPacket& data) const { Imports.BroadcastMessage(&m_controlled->Owner, data);}
        inline void SendPacket(const WorldPacket& data) const { Imports.SendPacket(m_socket, data);}

        void CleanReferences();
        void Dereference(const UnitMovementImpl * m);
    public:

        explicit ClientImpl(void * socket);
        ~ClientImpl();

        std::string ToString() const;

        void LostControl();
        void SetControl(UnitMovementImpl& mov);

        void SendMoveMessage(MovementMessage& msg) const;

    public:

        static void OnCommonMoveMessage(ClientImpl& client, WorldPacket& recv_data);
        static void OnMoveTimeSkipped(ClientImpl& client, WorldPacket & recv_data);
        static void OnNotImplementedMessage(ClientImpl& client, WorldPacket& data);
        static void OnSplineDone(ClientImpl& client, WorldPacket& data);
        static void OnNotActiveMover(ClientImpl& client, WorldPacket& data);
        static void OnActiveMover(ClientImpl& client, WorldPacket& data);
    };

    class MoveHandlersBinder
    {
    public:
        typedef void (*Handler)(ClientImpl&, WorldPacket& msg);
        typedef stdext::hash_map<ClientOpcode, Handler> HandlerMap;

        static void InvokeHander(ClientImpl& client, WorldPacket& msg)
        {
            ClientOpcode opcode = (ClientOpcode)msg.GetOpcode();
            HandlerMap::const_iterator it = instance().handlers.find(opcode);
            assert_state_msg(it != instance().handlers.end(), "no handlers for %s", LookupOpcodeName(opcode));
            (it->second) (client, msg);
        }

        static void FillSubscribeList(std::vector<uint16>& opcodes)
        {
            for (HandlerMap::const_iterator it = instance().handlers.begin(); it != instance().handlers.end(); ++it)
                opcodes.push_back(it->first);
        }

    private:

        explicit MoveHandlersBinder();

        static MoveHandlersBinder& instance() {
            static MoveHandlersBinder _instance;
            return _instance;
        }

        HandlerMap handlers;

        void assignHandler(Handler hdl, ClientOpcode opcode) {
            if (opcode == MSG_NULL_ACTION)
                return;
            HandlerMap::const_iterator it = handlers.find(opcode);
            // two normal cases here: handler wasn't assigned yet or same handler for same opcode
            assert_state(it == handlers.end() || it->second == hdl);
            handlers.insert(HandlerMap::value_type(opcode,hdl));
        }

        void assignHandler(Handler hdl, const ClientOpcode * opcodes, uint32 count) {
            for (uint32 i = 0; i < count; ++i)
                assignHandler(hdl, opcodes[i]);
        }
    };

    class RespHandler : public ICallBack
    {
    private:
        ClientImpl* m_client;
        ClientOpcode m_opcode;
        bool m_wasHandled;
    protected:
        uint32 m_requestId;

    private:
        void Execute(TaskExecutor_Args& args) override {
            if (!m_wasHandled)
                log_function("response timeout (opcode: %s)", LookupOpcodeName(m_opcode));
        }

    protected:
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) = 0;

        bool checkRequestId(uint32 requestId) const {
            if (m_requestId != requestId) {
                log_function("wrong request Id: %u expected Id: %u", requestId, m_requestId);
                return false;
            }
            return true;
        }
    public:

        /* Default timeout value is 500 milliseconds */
        static uint32 DefaultTimeout;

        explicit RespHandler(ClientOpcode _opcode, ClientImpl * client) : m_opcode(_opcode), m_client(client), m_wasHandled(false)
        {
            assert_state(m_client);
            addref();
            m_requestId = client->RegisterRespHandler(this);
        }

        virtual ~RespHandler() {
            assert_state(m_client);
            m_client = NULL;
        }

        static void OnResponse(ClientImpl& client, WorldPacket& data)
        {
            RespHandler * handler = client.PopRespHandler();
            if (!handler)
                return;
            if (!handler->OnReply(data))
                log_function("client's response (opcode %s) handler failed", LookupOpcodeName(handler->m_opcode));
            handler->release();
        }

        bool OnReply(WorldPacket& data)
        {
            assert_state(!m_wasHandled);
            if (m_opcode != data.GetOpcode()) {
                log_function("expected reply was: %s, but received instead: %s", LookupOpcodeName(m_opcode), LookupOpcodeName((ClientOpcode)data.GetOpcode()));
                return false;
            }
            m_wasHandled = OnReply(m_client, data);
            return m_wasHandled;
        }
    };

    uint32 RespHandler::DefaultTimeout = 30000;
}
