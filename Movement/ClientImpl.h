
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once


namespace Movement
{
    class ByteBuffer;
    class Packet;
    class MovementMessage;
    class UnitMovementImpl;
    class RespHandler;

    class ClientImpl
    {
    private:
        void * m_socket;
        UnitMovementImpl * m_controlled;
        Context &m_context;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        UInt32Counter m_requestCounter;
        //int32 time_skipped;
        typedef QVector<Reference<RespHandler> > RespHdlContainer;
        RespHdlContainer m_resp_handlers;
        ObjectGuid m_firstControlled; // it's always a 

    public:
        static MSTime ServerTime() { return MSTime(Imports.getMSTime());}
        MSTime ServerToClientTime(const MSTime& server_time) const { return server_time + m_time_diff;}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime());}
        MSTime ClientToServerTime(const MSTime& client_time) const { return client_time - m_time_diff;}

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

        // 
        UnitMovementImpl* firstControlledUnit() const {
            UnitMovement *unit = m_context.registry.get<UnitMovement>(m_firstControlled);
            return unit ? &unit->Impl() : nullptr;
        }

        void SetClientTime(const MSTime& client_time) {
            m_time_diff = client_time - ServerTime();
        }

        /** The main 'gate' for movement states that incomes from client. */
        void QueueState(ClientMoveStateChange& client_state, const ObjectGuid& source);

        uint32 RegisterRespHandler(Reference<RespHandler> handler);
        Reference<RespHandler> PopRespHandler();

        void Kick() {}  // not implemented

        void BroadcastMessage(MovementMessage& msg) const {
            assert_state(msg.Packet().getOpcode() != MSG_NULL_ACTION);
            Imports.BroadcastMoveMessage(m_controlled->Owner, msg);
        }

        void BroadcastMessage(const Packet& data) const {
            assert_state(data.getOpcode() != MSG_NULL_ACTION);
            Imports.BroadcastMessage(m_controlled->Owner, data.toPacketData());
        }

        void SendPacket(const Packet& data) const {
            log_debug("server sends: %s", OpcodeName((ClientOpcode)data.getOpcode()));
            assert_state(data.getOpcode() != MSG_NULL_ACTION);
            Imports.SendPacket(m_socket, data.toPacketData());
        }

        void CleanReferences();
        void Dereference(const UnitMovementImpl * m);
    public:

        explicit ClientImpl(void * socket, Context &context)
            :  m_socket(socket)
            , m_controlled(nullptr)
            , m_context(context)
        {
        }

        ~ClientImpl();

        void ToString(QTextStream& st) const;

        void LostControl();
        void SetControl(UnitMovementImpl& mov);

        void SendMoveMessage(MovementMessage& msg) const;

    public:

        static void OnCommonMoveMessage(ClientImpl& client, Packet& recv_data);
        static void OnMoveTimeSkipped(ClientImpl& client, Packet & recv_data);
        static void OnNotImplementedMessage(ClientImpl& client, Packet& data);
        static void OnSplineDone(ClientImpl& client, Packet& data);
        static void OnNotActiveMover(ClientImpl& client, Packet& data);
        static void OnActiveMover(ClientImpl& client, Packet& data);
    };

    class HandlersHolder
    {
    public:
        typedef void (*Handler)(ClientImpl&, Packet& msg);
        typedef QHash<ClientOpcode, Handler> HandlerMap;

    private:
        HandlerMap handlers;

        static Handler getHandler(ClientOpcode opcode) {
            return instance().handlers.value(opcode, nullptr);
        }

    public:

        static HandlersHolder& instance() {
            static HandlersHolder _instance;
            return _instance;
        }

        void InvokeHander(ClientImpl& client, Packet& msg)
        {
            ClientOpcode opcode = (ClientOpcode)msg.getOpcode();
            Handler hdl = getHandler(opcode);
            assert_state_msg(hdl != nullptr, "no handlers for %s", OpcodeName(opcode));
            (*hdl) (client, msg);
            ensureParsed(msg);
        }

        void FillSubscribeList(QVector<uint16>& opcodes)
        {
            assert_state_msg(!instance().handlers.empty(), "bad news, no handlers binded");
            foreach(ClientOpcode opcode, instance().handlers.keys())
                opcodes += (uint16)opcode;
        }

        void ensureParsed(const Packet& msg)
        {
            if (msg.size() != msg.rpos())
            {
                log_write("message %s seems wasn't parsed properly, %u bytes weren't parsed",
                    OpcodeName(ClientOpcode(msg.getOpcode())), uint32(msg.size() - msg.rpos()));
            }
        }

        void assignHandler(Handler hdl, ClientOpcode opcode) {
            if (opcode == MSG_NULL_ACTION)
                return;
            Handler handler = getHandler(opcode);
            // two normal cases here: handler wasn't assigned yet or same handler for same opcode
            assert_state(handler == nullptr || handler == hdl);
            instance().handlers.insert(opcode, hdl);
        }

        void assignHandler(Handler hdl, const ClientOpcode * opcodes, uint32 count) {
            for (uint32 i = 0; i < count; ++i)
                assignHandler(hdl, opcodes[i]);
        }
    };

#define ASSIGN_HANDLER(MessageHanger, ... ) { \
        ::Movement::ClientOpcode opcodes[] = {__VA_ARGS__}; \
        ::Movement::HandlersHolder::instance().assignHandler(MessageHanger, opcodes, CountOf(opcodes)); \
    }

    class RespHandler : public ICallBack
    {
    private:
        ClientImpl* m_client;
        ClientOpcode m_targetOpcode;
        MSTime m_TimeoutLaunchTime;
        uint32 m_requestId;
        bool m_replyReceived;
    protected:

        uint32 requestId() const { return m_requestId;}

    private:
        void Execute(TaskExecutor_Args& args) override {
            if (!m_replyReceived) {
                /** Currently this problem is caused by some unaccounted wow-client's code technical details:
                    wow-client ignores requests while in busy state(for ex. during teleport).*/
                log_function("response handler %s timed out. timeout is %u seconds",
                    OpcodeName(m_targetOpcode), (args.now - m_TimeoutLaunchTime).time/1000 );
            }
        }

        void LaunchTimeoutCheck(MSTime timeout)
        {
            m_TimeoutLaunchTime = Imports.getMSTime();
            m_client->commonTasks.AddTask(this, m_TimeoutLaunchTime + timeout);
        }

    protected:
        virtual bool OnReply(ClientImpl * client, Packet& data) = 0;

        bool checkRequestId(uint32 RequestId) const {
            if (requestId() != RequestId) {
                /** Currently this problem is caused by some unaccounted wow-client's code technical details:
                    wow-client ignores request packets while in busy state(for ex. during teleporting).
                    When client is not busy and able send responses, server sends some new request and client sends a reply, 
                    but since there is still old unreplyed response handler queued*/
                log_function("can not handle response %s - wrong request Id %u, expected request %u",
                    OpcodeName(m_targetOpcode), RequestId, m_requestId);
                //return false;
            }
            return true;
        }

        /* Default timeout, in milliseconds */
        static uint32 DefaultTimeout;

        explicit RespHandler(ClientOpcode targetOpcode, ClientImpl * client) {
            assert_state(client);
            assert_state(targetOpcode != MSG_NULL_ACTION);
            m_targetOpcode = targetOpcode;
            m_client = client;
            m_replyReceived = false;
            m_requestId = client->RegisterRespHandler(Reference<RespHandler>(this));
            LaunchTimeoutCheck(DefaultTimeout);
        }

        virtual ~RespHandler() {
            assert_state(m_client);
            m_client = NULL;
        }

    public:

        static void OnResponse(ClientImpl& client, Packet& data)
        {
            Reference<RespHandler> handler = client.PopRespHandler();
            if (!handler.pointer()) {
                log_function("can not handle response %s - no response handlers queued", OpcodeName((ClientOpcode)data.getOpcode()));
                return;
            }

            assert_state(!handler->m_replyReceived);
            assert_state(handler->m_client == &client);

            handler->m_replyReceived = true;
            if (handler->m_targetOpcode != data.getOpcode()) {
                log_function("can not handle response %s - expected response is %s",
                    OpcodeName((ClientOpcode)data.getOpcode()), OpcodeName(handler->m_targetOpcode));
                return;
            }

            handler->OnReply(&client, data);
        }
    };

    uint32 RespHandler::DefaultTimeout = 30000;
}
