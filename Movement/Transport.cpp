#include "Transport.h"

#include "framework/gtest.h"
#include "framework/DelayInit.h"
#include "Imports.h"
#include "TaskScheduler.h"
#include "framework/typedefs_p.h"

#include "MovementBase.h"
#include "spline.h"
#include "UpdateFields.h"
#include "MovementCommandMgr.h"

#include <memory>
#include <QtCore/QVector>
#include <QtCore/QTextStream>

namespace Movement
{
    class TransportImpl : public MovingEntity_WOW
    {
        using MovingEntity_WOW::Environment;
    public:

        Tasks::TaskTarget_DEV tasks;

        explicit TransportImpl(const Transport::CreateInfo& info)
        {
            ComponentInit<MovingEntity_WOW>(this);

            Init(ObjectGuid(info.guid), info.object, *info.context);
        }

        ~TransportImpl() {
            CleanReferences();
        }

    private:

        void CleanReferences() {
            UnboardAll();
            MovingEntity_WOW::CleanReferences();
        }

        void UnboardAll() {
            // maybe it's not ok to invoke unboard at such low level.
            while (!BindedEntities().empty()) {
                BindedEntities().first()->Value->as<IPassenger>().Unboard();
            }
            assert_state(BindedEntities().empty());
        }
    };
}

using Movement::uint32;

struct TaxiPathNodeEntry
{
    enum eActionFlag {
        ActionNone      = 0,
        ActionTeleport  = 1,
        ActionStop      = 2,    // transport will pause his movement for TaxiPathNodeEntry.delay seconds
    };

    // 0        m_ID
    uint32    path;                                         // 1        m_PathID
    uint32    index;                                        // 2        m_NodeIndex
    uint32    mapid;                                        // 3        m_ContinentID
    float     x;                                            // 4        m_LocX
    float     y;                                            // 5        m_LocY
    float     z;                                            // 6        m_LocZ
    uint32    actionFlag;                                   // 7        m_flags
    uint32    delay;                                        // 8        m_delay in seconds
    uint32    arrivalEventID;                               // 9        m_arrivalEventID
    uint32    departureEventID;                             // 10       m_departureEventID

    bool actionStop() const { return actionFlag == ActionStop;}
    bool actionTeleport() const { return actionFlag == ActionTeleport;}
    bool noAction() const { return actionFlag == ActionNone;}
};

namespace Movement
{
    /** Object-function that describes how distance changes in respect to time. */
    struct LengthPassedDescr
    {
        uint32 enterStamp;
        uint32 departureTime;
        uint32 timeTotal;

        float accel;
        float velMax;
        float pathLength;
        float initialLength;

        bool beginAccel;
        bool endDecel;

        float timeEndAccel;
        float timeBeginDecel;

        public: LengthPassedDescr() {
            memset(this, 0, sizeof(*this));
        }

        public: void Init(float Velocity, float Accel, float PathLength, float InitialLength, bool BeginAccel, bool EndDecel)
        {
            accel = Accel;
            velMax = Velocity;
            pathLength = PathLength;
            initialLength = InitialLength;

            beginAccel = BeginAccel;
            endDecel = EndDecel;

            InitMoveTime();
        }

        public: void InitMoveTime()
        {
            float ftimeTotal = 0;

            if (!beginAccel && !endDecel)
            {
                ftimeTotal = pathLength / velMax;
                timeEndAccel = 0;
                timeBeginDecel = ftimeTotal;
            }
            else {
                // Length of the path that required to accelerate(or decelerate) from zero to maximum velocity
                float accelS = velMax*velMax / (2*accel);
                static_assert((1 == (int32)true) && (0 == (int32)false), "");
                // 'enoughToAccelerate' is true in case it's possible to accelerate to maximum velocity
                bool enoughToAccelerate = (pathLength >= ((int32)beginAccel+(int32)endDecel)*accelS);
                if (enoughToAccelerate)
                {
                    /** Graphic shows velocity changes. In current example 
                        ship accelerates in begin and decelerates in end
                      V  | velocity
                     ____|_______________ velocity maximum
                         |  /          \
                         | /            \
                     ____|/______________\______time___ t
                    */
                    // Time that needed to accelerate from zero to maximum velocity
                    float accelT = velMax / accel;

                    if (beginAccel) {
                        ftimeTotal += accelT;
                        timeEndAccel = ftimeTotal;
                    } 
                    else
                        ftimeTotal += accelS / velMax;

                    ftimeTotal += (pathLength - 2*accelS) / velMax;

                    if (endDecel) {
                        timeBeginDecel = ftimeTotal;
                        ftimeTotal += accelT;
                    } else {
                        ftimeTotal += accelS / velMax;
                        timeBeginDecel = ftimeTotal;
                    }
                }
                else {
                    /** Graphic shows velocity changes. In current example 
                        ship accelerates in begin and decelerates in end.
                        Ship have no enough time to reach velocity maximum.
                      V  | velocity
                     ____|_____________  velocity maximum
                         |  
                         | /\
                     ____|/__\______________time___ t
                    */ 

                    if (beginAccel && endDecel)
                    {
                        // Time that needed to accelerate velocity from zero to maximum velocity
                        // deceleration and acceleration takes equal time
                        //sqrtf(2 * (spline.lengthTotal()/2) / accel);
                        float accelT = sqrtf(pathLength / accel);
                        // recalculate maximum velocity
                        velMax = accelT * accel;

                        timeEndAccel = timeBeginDecel = accelT;
                        ftimeTotal = 2 * accelT;
                    }
                    else if (beginAccel) {
                        //assert_state(segmentLength >= accelS); //for test
                        ftimeTotal = sqrtf(2 * pathLength / accel);
                        timeEndAccel = timeBeginDecel = ftimeTotal;
                        velMax = ftimeTotal * accel;
                    }
                    else if (endDecel) {
                        //assert_state(segmentLength >= accelS); //for test
                        ftimeTotal = sqrtf(2 * pathLength / accel);
                        timeEndAccel = timeBeginDecel = 0.f;
                        velMax = ftimeTotal * accel;
                    }
                }
            }
            timeTotal = ftimeTotal * 1000;
        }

        public: float pathPassedLength(uint32 timeMs) const
        {
            assert_state(timeMs <= arriveTime());

            float time = (timeMs > departureTime ? timeMs - departureTime : 0) * 0.001f;
            
            float len = 0.f;
            if (time < timeEndAccel)
                len = accel * time * time * 0.5f;

            else if (time < timeBeginDecel)
            {
                if (beginAccel) {
                    len += accel * timeEndAccel * timeEndAccel * 0.5f;  // that calculation can be cached
                    len += (time - timeEndAccel) * velMax;
                } 
                else
                    len += time * velMax;
            }

            else {

                if (beginAccel) {
                    len += accel * timeEndAccel * timeEndAccel * 0.5f;  // that calculation can be cached
                    len += (timeBeginDecel - timeEndAccel) * velMax;
                } 
                else
                    len += timeBeginDecel * velMax;

                //assert_state(endDecel); // assertion fails cause float ops are not precise. disabled assertion causes no harm
                float timeLeft = (time - timeBeginDecel);
                // deceleration: @accel has negative sign
                len += velMax*timeLeft - accel * timeLeft * timeLeft * 0.5f;
            }
            // asserts that recently calculated length value is near or less than total length
            assert_state(G3D::fuzzyLe(len,pathLength));
            return initialLength + len;
        }

        public: uint32 arriveTime() const {
            return (departureTime + timeTotal);
        }

        public: bool isMoving(uint32 time) const {
            assert_state(time <= arriveTime());
            return time > departureTime;
        }

        public: uint32 moveTimeTotal() const { return timeTotal;}
    };

    struct TransportState
    {
        Vector3 position;
        Vector3 der;
        uint32 nodeIdx;
        uint32 mapId;
    };

    class PathSegment
    {
        public: const uint32 enterTime;
        private: uint32 m_timeTotal;
        private: uint32 m_mapId;
        private: Spline<float> m_spline;
        private: QVector<LengthPassedDescr> m_nodes;

        public: uint32 moveTimeTotal() const { return m_timeTotal;}
        public: uint32 mapId() const { return m_mapId;}

        public: PathSegment(const Transport::MotionInfo& info, uint32& nodeItr, uint32 EnterTime)
            : enterTime(EnterTime)
        {
            m_timeTotal = 0;

            assert_state(info.nodes);
            assert_state(info.nodesSize > 1);
            assert_state(nodeItr < info.nodesSize);

            m_mapId = info.nodes[nodeItr].mapid;

            const uint32 first = nodeItr;
            initSpline(info, first, nodeItr);
            initLengthDescriptors(info, first);
        }

        private: void initSpline(const Transport::MotionInfo &info, uint32 first, uint32 &nodeItr) 
        {
            const TaxiPathNodeEntry * const nodes = info.nodes;
            SplineBase::ControlArray points;
            while (true) {
                const TaxiPathNodeEntry& node = nodes[nodeItr];
                points.push_back( (Vector3&)node.x );

                ++nodeItr;

                if (nodeItr == info.nodesSize || nodes[nodeItr].mapid != node.mapid)
                    break;
            }
            // now nodeItr is invalid or points to node that on different map

            m_spline.initCustom(&points[0], points.size(), SplineBase::ModeCatmullrom);

            struct LengthInit {
                const Transport::MotionInfo& info;
                int32 firstDbcIdx;

                float operator()(const Spline<float>& s, int32 segmentIdx) {
                    if (info.nodes[segmentIdx + firstDbcIdx + 1].actionTeleport())
                        return 0;
                    else
                        return s.segmentLength(segmentIdx, SplineBase::LengthPrecisionWoWClient);
                }
            };
            LengthInit init = {info, first};
            m_spline.initLengthsPart(init);
        }

        private: void initLengthDescriptors(const Transport::MotionInfo &info, uint32 first) 
        {
            for (int32 segmentIdx = 0; segmentIdx < m_spline.last(); )
            {
                // it is 'splineIdx + 1' because first and last nodes are used just as storage for spline control points 
                // and are take no any participation in path length calculation
                const TaxiPathNodeEntry& node = info.nodes[first + segmentIdx + 1];

                if (node.actionTeleport()) {
                    ++segmentIdx;
                    continue;
                }
                
                bool beginAccel = node.actionStop();
                bool endDeccel;
                int32 segmentIdxBegin = segmentIdx;
                const float initialLength = m_spline.length(segmentIdx);
                while(true) {
                    ++segmentIdx;
                    const TaxiPathNodeEntry& nodeNext = info.nodes[first + segmentIdx + 1];

                    if ((segmentIdx+1 > m_spline.last()) || !nodeNext.noAction()) {
                        endDeccel = nodeNext.actionStop();
                        break;
                    }
                }
                float segmLength = m_spline.lengthBetween(segmentIdxBegin, segmentIdx);

                LengthPassedDescr taxiNode;
                taxiNode.enterStamp = m_timeTotal;
                m_timeTotal += (node.actionStop() ? node.delay : 0) * 1000;
                taxiNode.departureTime = m_timeTotal;
                taxiNode.Init(info.velocity, info.acceleration, segmLength, initialLength, beginAccel, endDeccel);
                m_timeTotal += taxiNode.moveTimeTotal();

                m_nodes.push_back(taxiNode);
            }
            
            const LengthPassedDescr& lastDescr = m_nodes.back();
            assert_state( G3D::fuzzyEq(m_spline.lengthTotal(), lastDescr.initialLength+lastDescr.pathLength) );
        }

        private: const LengthPassedDescr& getDescr(uint32 timeRelative) const
        {
            struct comparer {
                bool operator () (const uint32& time, const LengthPassedDescr& segment) {
                    return time < segment.enterStamp;
                }
            };
            const LengthPassedDescr& descr = *(qUpperBound(m_nodes.begin(),m_nodes.end(),timeRelative,comparer()) - 1);
            assert_state(descr.enterStamp <= timeRelative);
            return descr;
        }

        public: TransportState computeState(uint32 time) const
        {
            time = relativeTime(time);
            const LengthPassedDescr& desc = getDescr(time);
            int32 splineIdx;
            float u;
            m_spline.computeIndex(desc.pathPassedLength(time) / m_spline.lengthTotal(), splineIdx, u);

            TransportState state;
            state.position = m_spline.evaluatePosition(splineIdx, u);
            state.der = -m_spline.evaluateDerivative(splineIdx, u);
            state.nodeIdx = splineIdx;
            state.mapId = m_mapId;
            return state;
        }

        public: void movingState(QTextStream& st, uint32 time) const
        {
            time = relativeTime(time);
            TransportState state = computeState(time);
            st << endl << "map Id " << m_mapId;

            const LengthPassedDescr& desc = getDescr(time);
            st << endl << "beginStop " << desc.beginAccel << " endStop " << desc.endDecel;
            st << endl << "timeToNextDesc " << 0.001f*(desc.arriveTime() - time);
            st << endl << "isMoving = " << desc.isMoving(time);
         }

        private: uint32 relativeTime(uint32 time) const {
            assert_state(enterTime <= time && time <= (enterTime + moveTimeTotal()));
            return time - enterTime;
        }
    };

    /** Controls transport motion */
    class MOTransportMover : private Component
    {
        COMPONENT_TYPEID(MOTransportMover);
        TransportImpl m_controlled;
        Vector3 m_initialLocation;
        Transport* m_publicFace;

        uint32 m_pathId;
        uint32 m_currentTime;
        uint32 m_period;
        uint32 m_mapId;
        QVector<PathSegment*> m_segments;

        Tasks::TaskTarget_DEV m_positionUpdateTask;

    public:
        int32 timeModDbg;

        uint32 timeLine() const { return m_currentTime;}
        uint32 period() const { return m_period; }
        uint32 mapId() const { return m_mapId;}
        TransportImpl& controlled() { return m_controlled;}

        explicit MOTransportMover(const Transport::CreateInfo& info, Transport* publicFace) : m_controlled(info)
        {
            m_pathId = info.motion.nodes->path;
            m_period = 0;
            timeModDbg = 0;
            m_currentTime = 0;
            m_mapId = (uint32)-1;
            m_publicFace = publicFace;
            m_initialLocation = Vector3(info.initialLocation.x, info.initialLocation.y, info.initialLocation.z);

            uint32 firstIdx = 0;
            while (firstIdx < info.motion.nodesSize) {
                PathSegment* segment = new PathSegment(info.motion, firstIdx, m_period);
                m_period += segment->moveTimeTotal();
                m_segments.append(segment);
            }
            assert_state(!m_segments.empty());
            float oldPeriod = 0.001f * Imports.GetUIntValue(m_controlled.Owner,GAMEOBJECT_LEVEL);
            Imports.SetUIntValue(m_controlled.Owner,GAMEOBJECT_LEVEL, m_period);

            m_controlled.ComponentAttach(this);
            m_positionUpdateTask.SetExecutor(m_controlled.context->executor);
            m_positionUpdateTask.AddTask(newTask(this,&MOTransportMover::updatePositionCallback), 0);

            updateState(info.context->executor.Time().time);

            log_debug("transport mover initialized");
            log_debug("    %u new period %f seconds, old %f. accelRate %f velocity %f",
                info.motion.nodes->path, m_period/1000.f, oldPeriod, info.motion.acceleration, info.motion.velocity);
        }

        ~MOTransportMover() {
            m_positionUpdateTask.Unregister();
            ComponentDetach();

            qDeleteAll(m_segments);
        }

        /** If true, enables transport position visualization by spawning marks.
            Disabled by default, because such visualization causes client crash.*/
        static volatile bool spawnMarks;

        void updatePositionCallback(Tasks::TaskExecutor_Args& args)
        {
            Tasks::RescheduleTaskWithDelay(args, 500);
            updateState(args.now.time);
            // spawn mark each 2.5 sec to show ship server-side location
            if (args.execTickCount % (2500/100) && spawnMarks)
                Imports.SpawnMark(m_controlled.Owner, m_controlled.GlobalPosition());
        }

        void updateState(uint32 time)
        {
            m_currentTime = time % m_period;
            TransportState state = currentSegment().computeState(m_currentTime);
            m_controlled.RelativePosition(m_initialLocation + state.position);
            m_controlled.SetRotationFromTangentLine(state.der);
            if (m_mapId != state.mapId) {
                m_mapId = state.mapId;
                Transport::OnMapChanged(*m_publicFace, controlled().Owner);
            }
        }

        const PathSegment& currentSegment() const {
            struct comparer {
                bool operator () (const uint32& time, const PathSegment* segment) {
                    return time < segment->enterTime;
                }
            };
            const PathSegment& segm = **(qUpperBound(m_segments.constBegin(),m_segments.constEnd(),m_currentTime,comparer()) - 1);
            assert_state(segm.enterTime <= m_currentTime);
            return segm;
        }

        void toString(QTextStream& st) const override
        {
            st << endl << "path Id " << m_pathId;
            st << endl << "movetime mod " << timeModDbg*0.001f;
            st << endl << "period " << m_period*0.001f;
            st << endl << "passed " << m_currentTime*0.001f;
            st << endl << "map Id " << m_mapId;
            currentSegment().movingState(st, m_currentTime);
        }
    };

    volatile bool MOTransportMover::spawnMarks = false;

    Transport::Transport(const Transport::CreateInfo& info) : m(nullptr) {
        m = new MOTransportMover(info, this);
    }

    Transport::~Transport() {
        delete m;
        m = nullptr;
    }

    uint32 Transport::TimeLine() {
        return m->timeLine();
    }

    uint32 Transport::MapId() {
        return m->mapId();
    }

    const Vector3& Transport::Position() {
        return m->controlled().GlobalPosition();
    }

    QVector<Component*> Transport::Passengers() {
        QVector<Component*> passengers;
        auto first = m->controlled().BindedEntities().first();
        while (first) {
            passengers.append(first->Value);
            first = first->Next();
        }
        return passengers;
    }

    QByteArray Transport::WriteCreate() {
        assert_state(false); // not implemented yet
        return QByteArray();
    }

    static void OnMapChanged_DoNothing(Transport&, WorldObject*) {}
    void (*Transport::OnMapChanged)(Transport&, WorldObject*) = &OnMapChanged_DoNothing;
}

namespace Movement
{
#   define STR(x) #x

    DECLARE_COMMAND_NODE(TransportCommandNode, "transport", MovementCommand);

    static MOTransportMover* extractMover(CommandInvoker& invoker)
    {
        MOTransportMover * mover = nullptr;
        if (MovingEntity_WOW * transp = invoker.com.as<MovingEntity_WOW>().Environment()) {
            mover = transp->getAspect<MOTransportMover>();
            if (!mover)
                invoker.output << endl << "Transport-target has no " STR(MOTransportMover) " component";
        } else
            invoker.output << endl << "Invoker is not boarded";
        return mover;
    }

    struct ToggleTransportPathPoints : TransportCommandNode // public MovementCommand
    {
        explicit ToggleTransportPathPoints() {
            Init("visualize|vis");
            Description = "Enables or disables transport position visualization. Command affects all transports.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            MOTransportMover::spawnMarks = !MOTransportMover::spawnMarks;
        }
    };
    DELAYED_INIT(ToggleTransportPathPoints, ToggleTransportPathPoints);

    struct SetTransportTimeMod : public TransportCommandNode
    {
        explicit SetTransportTimeMod() {
            Init("movetimemod|timemod");
            Description = "Modifies transport movement progress. Command argument is time in seconds. "
                "Negative time value moves it back, positive - forward.";
        }

        void Invoke(StringReader& command, CommandInvoker& inv) override {
            if (MOTransportMover * mover = extractMover(inv))
                mover->timeModDbg = command.readFloat() * 1000;
        }
    };
    DELAYED_INIT(SetTransportTimeMod, SetTransportTimeMod);

    struct PrintTransportInfoCommand : public TransportCommandNode
    {
        explicit PrintTransportInfoCommand() {
            Init("info");
            Description = "Prints transport info.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            if (MovingEntity_WOW* target = invoker.com.as<MovingEntity_WOW>().Environment())
                invoker.output << endl << "Transport info: " << target->toStringAll();
            else
                invoker.output << endl << "Invoker is not boarded";
        }
    };
    DELAYED_INIT(PrintTransportInfoCommand, PrintTransportInfoCommand);
}

namespace Movement
{
    TEST_REGISTER(LoadTaxiNodes);
    void LoadTaxiNodes(testing::State& testState)
    {
        const TaxiPathNodeEntry nodes[] = {
            {1094,0,571,7559.285645,1858.847656,644.990051,0,0,0,0},
            {1094,1,571,7495.156250,1804.520020,644.990051,0,0,0,0},
            {1094,2,571,7408.253418,1736.899902,636.212830,0,0,0,0},
            {1094,3,571,7287.811523,1662.633057,644.990051,0,0,0,0},
            {1094,4,571,7156.391602,1594.091553,644.990051,0,0,0,0},
            {1094,5,571,7035.677246,1512.187134,644.990051,2,5,0,0},
            {1094,6,571,7096.642578,1412.777588,629.185364,0,0,0,0},
            {1094,7,571,7243.689941,1458.626221,608.824402,0,0,0,0},
            {1094,8,571,7398.887207,1559.812744,595.243347,2,5,0,0},
            {1094,9,571,7705.005371,1751.556274,597.215637,0,0,0,0},
            {1094,10,571,7861.870605,1856.443970,620.241089,0,0,0,0},
            {1094,11,571,7810.094727,1962.217041,640.380249,2,5,0,0},
            {1094,12,571,7691.859375,1936.717896,641.684753,0,0,0,0},
            {1094,13,571,7560.709473,1858.655151,644.990051,0,0,0,0},
            {1094,14,571,7497.058105,1804.973389,644.990051,0,0,0,0},
            {1094,15,571,7409.123535,1737.521484,636.074219,0,0,0,0},
        };

        Transport::MotionInfo info;
        info.nodes = nodes;
        info.nodesSize = CountOf(nodes);
        info.acceleration = 1;
        info.velocity = 2;

        uint32 firstNode = 0;
        PathSegment segm(info, firstNode, 0);

        log_console("new period is %f, proper is 1051.388062", segm.moveTimeTotal()*0.001f);

        Vector3 prevPos = segm.computeState(0).position;

        for (uint32 time = 0; time <= segm.moveTimeTotal()*10; time += (segm.moveTimeTotal()/100)) {
            TransportState& st = segm.computeState(time % segm.moveTimeTotal());
            EXPECT_TRUE( st.position.isFinite() );
            EXPECT_TRUE( st.der.isFinite() );

            float dist = (st.position - prevPos).length();
            prevPos = st.position;
            // expects that position changes are enough 'smooth'
            //EXPECT_TRUE( dist < 80 );
        }
    }

    TEST_REGISTER(LengthPassedDescrTest);
    void LengthPassedDescrTest(testing::State& testState)
    {
        using G3D::fuzzyEq;

        {
            LengthPassedDescr desc;
            desc.accel = 0;
            desc.velMax = 10;
            desc.pathLength = 60;
            desc.beginAccel = desc.endDecel = false;
            desc.InitMoveTime();

            EXPECT_TRUE(desc.moveTimeTotal() == uint32(1000 * desc.pathLength / desc.velMax));
            EXPECT_TRUE(fuzzyEq(desc.pathPassedLength(desc.moveTimeTotal()), desc.pathLength));
        }
        {
            LengthPassedDescr desc;
            desc.accel = 2;
            desc.velMax = 10;
            desc.pathLength = 60;
            desc.beginAccel = true;
            desc.endDecel = false;
            desc.InitMoveTime();

            float accelS = desc.velMax * desc.velMax * 0.5f / desc.accel;
            float moveTime = 0.f;
            if (accelS >= desc.pathLength)
                moveTime = sqrtf(2*desc.pathLength/desc.accel);
            else
                moveTime = desc.velMax / desc.accel + (desc.pathLength - accelS) / desc.velMax;
            EXPECT_TRUE(fuzzyEq(0.001f * desc.moveTimeTotal(), moveTime));
        }
    }
}
