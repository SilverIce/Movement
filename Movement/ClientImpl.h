
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
        void SetClientTime(const MSTime& client_time) { m_time_diff = client_time - ServerTime();}

        /** The main 'gate' for movement states that incomes from client. */
        void QueueState(ClientMoveStateChange& client_state, const ObjectGuid& source);

        uint32 RegisterRespHandler(RespHandler* handler);
        RespHandler* PopRespHandler();
        void Kick() {}  // not implemented

        void BroadcastMessage(MovementMessage& msg) const {
            assert_state(msg.Packet().GetOpcode() != MSG_NULL_ACTION);
            Imports.BroadcastMoveMessage(m_controlled->Owner, msg);
        }

        void BroadcastMessage(WorldPacket& data) const {
            assert_state(data.GetOpcode() != MSG_NULL_ACTION);
            Imports.BroadcastMessage(m_controlled->Owner, data);
        }

        void SendPacket(const WorldPacket& data) const {
            assert_state(data.GetOpcode() != MSG_NULL_ACTION);
            Imports.SendPacket(m_socket, data);
        }

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
            assert_state_msg(it != instance().handlers.end(), "no handlers for %s", OpcodeName(opcode));
            (it->second) (client, msg);
            ensureParsed(msg);
        }

        static void FillSubscribeList(std::vector<uint16>& opcodes)
        {
            for (HandlerMap::const_iterator it = instance().handlers.begin(); it != instance().handlers.end(); ++it)
                opcodes.push_back(it->first);
        }

        static void ensureParsed(const WorldPacket& msg)
        {
            if (msg.size() != msg.rpos())
            {
                log_write("message %s seems wasn't parsed properly, %u bytes weren't parsed",
                    OpcodeName(ClientOpcode(msg.GetOpcode())), uint32(msg.size() - msg.rpos()));
            }
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
        ClientOpcode m_targetOpcode;
        bool m_replyReceived;
        MSTime m_TimeoutLaunchTime;
    protected:
        uint32 m_requestId;

    private:
        void Execute(TaskExecutor_Args& args) override {
            if (!m_replyReceived) {
                /** Currently this problem is caused by some unaccounted wow-client's code technical details:
                    wow-client ignores requests while in busy state(for ex. during teleporting).*/
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
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) = 0;

        bool checkRequestId(uint32 requestId) const {
            if (m_requestId != requestId) {
                /** Currently this problem is caused by some unaccounted wow-client's code technical details:
                    wow-client ignores request packets while in busy state(for ex. during teleporting).
                    When client is not busy and able send responses, servers sends some new request and client send a reply, 
                    but since there is still old unreplyed response handler queued*/
                log_function("can not handle response %s - wrong request Id %u, expected request %u",
                    OpcodeName(m_targetOpcode), requestId, m_requestId);
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

        static void OnResponse(ClientImpl& client, WorldPacket& data)
        {
            Reference<RespHandler> handler = client.PopRespHandler();
            if (!handler.pointer()) {
                log_function("can not handle response %s - no response handlers queued", OpcodeName((ClientOpcode)data.GetOpcode()));
                return;
            }

            assert_state(!handler->m_replyReceived);
            assert_state(handler->m_client == &client);

            handler->m_replyReceived = true;
            if (handler->m_targetOpcode != data.GetOpcode()) {
                log_function("can not handle response %s - expected response is %s",
                    OpcodeName((ClientOpcode)data.GetOpcode()), OpcodeName(handler->m_targetOpcode));
                return;
            }

            handler->OnReply(&client, data);
        }
    };

    uint32 RespHandler::DefaultTimeout = 30000;
}
