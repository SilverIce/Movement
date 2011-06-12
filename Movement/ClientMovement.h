
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#include "typedefs.h"
#include "MaNGOS_API.h"
#include "UnitMovement.h"

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    class MovementMessage;
    class UnitMovement;

    class Client
    {
        #pragma region Impl
    private:
        HANDLE m_socket;
        UnitMovement * m_controlled;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        MSTime m_last_sync_time;
        UInt32Counter ack_counter;
        UInt32Counter sync_counter;
        uint32 last_recvd_ack;
        //int32 time_skipped;

        static MSTime ServerTime() { return MSTime(getMSTime());}
        MSTime ServerToClientTime(const MSTime& server_time) const { return server_time + m_time_diff;}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime());}
        MSTime ClientToServerTime(const MSTime& client_time) const { return client_time - m_time_diff;}

        inline void BroadcastMessage(MovementMessage& msg) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, msg);}
        inline void BroadcastMessage(WorldPacket& data) const { MaNGOS_API::BroadcastMessage(&m_controlled->Owner, data);}
        inline void SendPacket(const WorldPacket& data) const { MaNGOS_API::SendPacket(m_socket, data);}

        void CleanReferences();
        #pragma endregion
    public:

        /** Client's lifetime bounded to WorldSession lifetime */
        Client(HANDLE socket);

        ~Client()
        {
            CleanReferences();
        }

        std::string ToString() const;

        void Dereference(const UnitMovement * m);
        void LostControl();
        void SetControl(UnitMovement * mov);

        void _OnUpdate();

        /** Handles messages from another clients */
        void HandleIncomingMessage(MovementMessage& msg) const;
        /** Handles messages from that client */
        void HandleOutcomingMessage(WorldPacket& recv_data);

        void HandleTimeSyncResp(WorldPacket& recv_data);

        void HandleMoveTimeSkipped(WorldPacket & recv_data);
    };
}
