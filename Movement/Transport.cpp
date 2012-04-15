#include "Transport.h"

#include "framework/gtest.h"
#include "Imports.h"
#include "TaskScheduler.h"

#include "MovementCommandMgr.h"
#include "framework/typedefs_p.h"
#include "UpdateFields.h"
#include "spline.h"
#include <memory>
#include "MovementBase.h"

namespace Movement
{
    using std::endl;

    class TransportImpl : public MovingEntity_WOW
    {
        using MovingEntity_WOW::Environment;
        using MovingEntity_WOW::SetEnvironment;
        using MovingEntity_WOW::BindedEntities;
    public:

        Tasks::TaskTarget_DEV tasks;

        explicit TransportImpl(const Transport::CreateInfo& info)
        {
            ComponentInit<MovingEntity_WOW>(this);

            Guid.SetRawValue(info.guid);
            Owner = info.object;
            tasks.SetExecutor(info.executor);
        }

        ~TransportImpl() {
            CleanReferences();
        }

    private:

        void CleanReferences() {
            UnboardAll();
            tasks.Unregister();
        }

        void UnboardAll() {
            struct {
                void operator()(Component * passenger) {
                    passenger->as<IPassenger>().Unboard();
                }
            } unboarder;
            BindedEntities().Visit(unboarder);
            assert_state(BindedEntities().empty());
        }
    };
}

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
    /** Object-function that calculates distance passed by transport. */
    struct LengthPassedDescr
    {
        uint32 enterStamp;
        uint32 departureStamp;
        uint32 timeTotal;

        float accel;
        float velMax;
        float segmentLength;
        float initialLength;

        bool beginAccel;
        bool endDecel;

        float timeEndAccel;
        float timeBeginDecel;

        public: LengthPassedDescr() {
            memset(this, 0, sizeof(*this));
        }

        public: void Init(float velocity, float Accel, float segmLength, float InitLength, bool BeginAccel, bool EndDecel)
        {
            accel = Accel;
            velMax = velocity;
            segmentLength = segmLength;
            initialLength = InitLength;

            beginAccel = BeginAccel;
            endDecel = EndDecel;

            InitMoveTime();
        }

        private: void InitMoveTime()
        {
            float ftimeTotal = 0;

            if (!beginAccel && !endDecel)
            {
                ftimeTotal = segmentLength / velMax;
                timeEndAccel = 0;
                timeBeginDecel = ftimeTotal;
            }
            else {
                // Length of the path that needed to accelerate from zero to maximum velocity
                float accelS = velMax*velMax / (2*accel);
                // Is true in case in current path segment transport able to accelerate to his maximum velocity
                bool enoughToAccelerate = (segmentLength >= ((int32)beginAccel+(int32)endDecel)*accelS);
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

                    ftimeTotal += (segmentLength - 2*accelS) / velMax;

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

                    // unsure that this part of code works properly
                    //assert_state(false);

                    if (beginAccel && endDecel)
                    {
                        // Time that needed to accelerate velocity from zero to maximum velocity
                        // deceleration and acceleration takes equal time
                        //sqrtf(2 * (spline.lengthTotal()/2) / accel);
                        float accelT = sqrtf(segmentLength / accel);
                        // recalculate maximum velocity
                        velMax = accelT * accel;

                        timeEndAccel = timeBeginDecel = accelT;
                        ftimeTotal = 2 * accelT;
                    }
                    else if (beginAccel) {
                        //assert_state(segmentLength >= accelS); //for test
                        ftimeTotal = sqrtf(2 * segmentLength / accel);
                        timeEndAccel = timeBeginDecel = ftimeTotal;
                        velMax = ftimeTotal * accel;
                    }
                    else if (endDecel) {
                        //assert_state(segmentLength >= accelS); //for test
                        ftimeTotal = sqrtf(2 * segmentLength / accel);
                        timeEndAccel = timeBeginDecel = 0.f;
                        velMax = ftimeTotal * accel;
                    }
                }
            }
            timeTotal = ftimeTotal * 1000;
        }

        public: float pathPassedLen(uint32 mstime) const
        {
            assert_state(mstime <= (departureStamp + timeTotal));

            float time = (mstime > departureStamp ? mstime - departureStamp : 0) * 0.001f;
            
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

                assert_state(endDecel);
                float timeLeft = (time - timeBeginDecel);
                // deceleration: @accel has negative sign
                len += velMax*timeLeft - accel * timeLeft * timeLeft * 0.5f;
            }
            // asserts that recently calculated length value is near or less than total length
            assert_state(G3D::fuzzyLe(len,segmentLength));
            return initialLength + len;
        }

        public: bool isMoving(uint32 mstime) const {
            assert_state(mstime <= (departureStamp + timeTotal));
            return mstime > departureStamp;
        }

        public: uint32 moveTimeTotal() const { return timeTotal;}
    };

    struct TransportState
    {
        Vector3 position;
        Vector3 der;
        uint32 nodeIdx;
        uint32 timePassed;
        bool isMoving;
    };

    class PathSegment
    {
        Spline<float> spline;

        std::vector<LengthPassedDescr> m_nodes;

        uint32 m_timeTotal;
        bool m_cyclic;

        public: PathSegment(const Transport::MotionInfo& info, int32& nodeItr)
        {
            m_timeTotal = 0;
            m_cyclic = false;

            assert_state(info.nodes);
            assert_state(info.nodesSize > 1);
            assert_state(nodeItr < (int32)info.nodesSize);

            const int32 first = nodeItr;
            bool cyclic;
            const TaxiPathNodeEntry * const nodes = info.nodes;
            {
                SplineBase::ControlArray points;
                while (true) {
                    const TaxiPathNodeEntry& node = nodes[nodeItr];
                    points.push_back( (Vector3&)node.x );

                    ++nodeItr;

                    if (nodeItr == info.nodesSize || nodes[nodeItr].mapid != node.mapid)
                        break;
                }
                // now nodeItr is invalid or points to node that on different map

                cyclic = (points.size() == info.nodesSize/* && !beginAccel && !endDeccel*/);
                assert_state(points.size() > 1);
                m_cyclic = cyclic;
                if (!cyclic)
                    spline.initSpline(&points[0], points.size(), SplineBase::ModeCatmullrom);
                else
                    spline.initCyclicSpline(&points[0], points.size(), SplineBase::ModeCatmullrom, 0);

                struct LengthInit {
                    const Transport::MotionInfo& info;
                    int32 firstDbcIdx;
                    float lengthSumm;

                    float operator()(Spline<float>& s, int32 splineIdx) {
                        if (info.nodes[splineIdx - s.first() + firstDbcIdx].actionTeleport())
                            return lengthSumm;
                        else
                            return (lengthSumm += s.segmentLength(splineIdx, SplineBase::LengthPrecisionWoWClient));
                    }
                };
                LengthInit init = {info, first, 0};
                spline.initLengths(init);
            }

            {
                uint32 time = 0;
                int32 endIdx = nodeItr + (int32)cyclic;
                for (int32 nodeIdx = first; (nodeIdx+1) < endIdx; )
                {
                    const TaxiPathNodeEntry& node = nodes[(nodeIdx) % info.nodesSize];

                    if (node.actionTeleport()) {
                        // pathLengthTotal, time variables are not grown here
                        ++nodeIdx;
                    }
                    else {
                        int32 splineIdxBegin = spline.first() + nodeIdx - first;
                        bool beginAccel = node.actionStop();
                        bool endDeccel;
                        const float initialLength = spline.length(splineIdxBegin);

                        while(true) {
                            ++nodeIdx;
                            const TaxiPathNodeEntry& nodeNext = nodes[nodeIdx % info.nodesSize];
                            if (!((nodeIdx+1) < endIdx) || !nodeNext.noAction()) {
                                endDeccel = nodeNext.actionStop();
                                break;
                            }
                        }
                        int32 splineIdxEnd = spline.first() + nodeIdx - first;

                        float segmLength = spline.lengthBetween(splineIdxBegin,splineIdxEnd);

                        LengthPassedDescr taxiNode;
                        taxiNode.enterStamp = time;
                        time += (node.actionStop() ? node.delay : 0) * 1000;
                        taxiNode.departureStamp = time;
                        taxiNode.Init(info.velocity, info.acceleration, segmLength, initialLength, beginAccel, endDeccel);
                        time += taxiNode.moveTimeTotal();

                        m_nodes.push_back(taxiNode);
                    }
                }
                m_timeTotal = time;
            }

            const LengthPassedDescr& lastDescr = m_nodes.back();
            assert_state( G3D::fuzzyEq(spline.lengthTotal(), lastDescr.initialLength+lastDescr.segmentLength) );
        }

        static bool fuzzyInRange(float value, float lo, float hi) {
            const float eps = 0.01f;
            return (value > lo - eps) && (value < hi + eps);
        }

        private: float timeToLengthCoeff(uint32 time) const
        {
            const LengthPassedDescr& descr = getDescr(time);
            float dist = descr.pathPassedLen(time);
            assert_state(dist <= spline.lengthTotal());
            return dist / spline.lengthTotal();
        }

        private: const LengthPassedDescr& getDescr(uint32 time) const {
            assert_state(time <= moveTimeTotal());
            int32 idx = 0;
            while(true) {
                // terminate cycle only if next node timestamp > time or there is no next node
                if (idx == (m_nodes.size()-1) || m_nodes[idx+1].enterStamp > time)
                    break;
                ++idx;
            }
            assert_state(m_nodes[idx].enterStamp <= time);
            return m_nodes[idx];
        }

        public: Vector3 evaluatePosition(uint32 time) const {
            return spline.evaluatePosition(timeToLengthCoeff(time));
        }

        public: Vector3 evaluateDerivative(uint32 time) const {
            Vector3 dir = -spline.evaluateDerivative(timeToLengthCoeff(time));
            // TODO: ensure that Spline::evaluateDerivative works properly.
            // something wrong: currently we have to inverse direction vector to fix it.
            return dir;
        }

        public: uint32 moveTimeTotal() const { return m_timeTotal;}

        public: bool isCyclic() const { return m_cyclic;}

        public: TransportState computeState(uint32 time) const
        {
            assert_state(time < m_timeTotal);

            const LengthPassedDescr& desc = getDescr(time);
            int32 splineIdx;
            float u;
            spline.computeIndex(desc.pathPassedLen(time) / spline.lengthTotal(), splineIdx, u);

            TransportState state;
            state.position = spline.evaluatePosition(splineIdx, u);
            state.der = -spline.evaluateDerivative(splineIdx, u);
            state.nodeIdx = splineIdx - spline.first();
            state.timePassed = time;
            state.isMoving = desc.isMoving(time);
            return state;
        }
    };

    /** Controls transport motion */
    class MOTransportMover : public Tasks::ICallBack, private Component
    {
        COMPONENT_TYPEID;
        TransportImpl* m_controlled;
        std::auto_ptr<PathSegment> m_segment;
        uint32 m_pathId;

        TransportState m_state;
    public:

        int32 timeMod;

        explicit MOTransportMover(TransportImpl& controlled, const Transport::MotionInfo& info)
        {
            m_controlled = &controlled;
            m_pathId = info.nodes->path;
            timeMod = 0;
            int32 firstIdx = 0;
            m_segment.reset(new PathSegment(info, firstIdx));
            controlled.ComponentAttach(this);
            float oldPeriod = (float)Imports.GetUIntValue(m_controlled->Owner,GAMEOBJECT_LEVEL) * 0.001f;
            Imports.SetUIntValue(m_controlled->Owner, GAMEOBJECT_LEVEL, m_segment->moveTimeTotal());

            log_debug("transport mover initialized");
            log_debug("    new period %f seconds, old %f. cyclic=%i",
                m_segment->moveTimeTotal()/1000.f, oldPeriod, (int)m_segment->isCyclic());
        }

        ~MOTransportMover() {
            m_controlled = nullptr;
        }

        /** If true, enables transport position visualization by spawning marks.
            Disabled by default, because such visualization causes client crash.*/
        static volatile bool spawnMarks;

        void Execute(Tasks::TaskExecutor_Args& args) override
        {
            Tasks::RescheduleTaskWithDelay(args, 500);

            uint32 time = (timeMod + args.now.time) % m_segment->moveTimeTotal();

            m_state = m_segment->computeState(time);
            m_controlled->RelativePosition(m_state.position);
            m_controlled->SetRotationFromTangentLine(m_state.der);

            // spawn mark each 2.5 sec to show ship server-side location
            if (args.execTickCount % (2500/100) && spawnMarks)
                Imports.SpawnMark(m_controlled->Owner, m_controlled->GlobalPosition());
        }

        std::string toString() const override
        {
            std::ostringstream st;
            st << endl << "path Id " << m_pathId;
            st << endl << "node Id " << m_state.nodeIdx;
            st << endl << "period (sec) " << m_segment->moveTimeTotal()*0.001f;
            st << endl << "passed (sec) " << m_state.timePassed*0.001f;
            st << endl << "movetime mod (sec) " << timeMod*0.001f;
            st << endl << "isMoving = " << m_state.isMoving;
            return st.str();
        }
    };

    volatile bool MOTransportMover::spawnMarks = false;

    Transport::Transport(const Transport::CreateInfo& info) {
        m = new TransportImpl(info);
        m->tasks.AddTask(new MOTransportMover(*m, info.motion), 0);
    }

    Transport::~Transport() {
        delete m;
        m = nullptr;
    }
}

namespace Movement
{
    struct ToggleTransportPathPoints : public MovementCommand
    {
        explicit ToggleTransportPathPoints() {
            Init("TransportVisualize|TranspVis");
            Description = "Enables or disables transport position visualization. Command affects all transports.";
        }

        void Invoke(StringReader& /*command*/, CommandInvoker& /*invoker*/) override {
            MOTransportMover::spawnMarks = !MOTransportMover::spawnMarks;
        }
    };
    DELAYED_INIT(ToggleTransportPathPoints, ToggleTransportPathPoints);

#   define STR(x) #x

    struct SetTransportTimeMod : public MovementCommand
    {
        explicit SetTransportTimeMod() {
            Init("TransportTimeMod|TranspMod");
            Description = "Modifies transport movement progress. Command argument is time in milliseconds. "
                "Negative time value moves it back, positive - forward.";
        }

        void Invoke(StringReader& command, CommandInvoker& inv) override {
            if (MovingEntity_Revolvable2 * transp = inv.com.as<MovingEntity_WOW>().Environment()) {
                if (MOTransportMover * mover = transp->getAspect<MOTransportMover>())
                    mover->timeMod = command.readInt();
                else
                    inv.output << endl << "Transport-target has no " STR(MOTransportMover) " component";
            }
            else
                inv.output << endl << "Invoker is not boarded";
        }
    };
    DELAYED_INIT(SetTransportTimeMod, SetTransportTimeMod);
}
