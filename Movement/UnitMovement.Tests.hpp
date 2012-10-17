#include "framework/gtest.h"

namespace Movement
{
    TEST(UnitMovement, UnitMoveFlag)
    {
        UnitMoveFlag::eUnitMoveFlags Flag = UnitMoveFlag::AllowSwimFlyTransition;
        UnitMoveFlag f(Flag);
        EXPECT_TRUE( f.hasFlag(Flag) );
        EXPECT_TRUE( (f & Flag) != 0 );
        EXPECT_TRUE( f.raw == Flag );
        EXPECT_TRUE( f.allowSwimFlyTransition );
        EXPECT_TRUE( f.toString() == "AllowSwimFlyTransition" );
        EXPECT_TRUE( sizeof(f.raw) == sizeof(UnitMoveFlag) );

        {
            ByteBuffer buf;
            UnitMoveFlag flagIn(UnitMoveFlag::Forward|UnitMoveFlag::AllowSwimFlyTransition);
            buf << flagIn;
            UnitMoveFlag flagOut;
            buf >> flagOut;
            EXPECT_TRUE( flagIn == flagOut );
        }
    }

    TEST(MoveSpline, MoveSplineFlag)
    {
        MoveSplineFlag::eFlags Flag = MoveSplineFlag::Done;
        MoveSplineFlag f(Flag);
        EXPECT_TRUE( f.hasFlag(Flag) );
        EXPECT_TRUE( (f & Flag) != 0 );
        EXPECT_TRUE( f.raw == Flag );
        EXPECT_TRUE( f.done );
        EXPECT_TRUE( f.toString() == "Done" );
        EXPECT_TRUE( sizeof(f.raw) == sizeof(MoveSplineFlag) );
    }

    void testClientMoveStateSerialization(testing::State& testState, const ClientMoveState& stateIn)
    {
        ByteBuffer buf;
        PacketBuilder::WriteClientStatus(stateIn, buf);
        ClientMoveState stateOut;
        PacketBuilder::ReadClientStatus(stateOut, buf);

        #define CMP(field) EXPECT_TRUE(stateIn.field == stateOut.field);
        CMP(ClientMoveState::fallTime);
        CMP(ClientMoveState::jump_directionX);
        CMP(ClientMoveState::jump_directionY);
        CMP(ClientMoveState::jump_horizontalVelocity);
        CMP(ClientMoveState::jump_verticalVelocity);
        CMP(ClientMoveState::jump_verticalVelocity);
        CMP(ClientMoveState::moveFlags);
        CMP(ClientMoveState::ms_time);
        CMP(ClientMoveState::pitchAngle);
        CMP(ClientMoveState::spline_elevation);
        CMP(ClientMoveState::transport_guid);
        CMP(ClientMoveState::relativePosition);
        CMP(ClientMoveState::transport_seat);
        CMP(ClientMoveState::transport_time);
        CMP(ClientMoveState::transport_time2);
        CMP(ClientMoveState::globalPosition);
        #undef CMP
    }

    TEST(ClientMoveState, serialization)
    {
        ClientMoveState stateIn;

        stateIn.globalPosition = Vector4(10, 20, 30, 1);
        stateIn.fallTime = 400;
        stateIn.ms_time = 20000;
        testClientMoveStateSerialization(testState,stateIn);

        stateIn.moveFlags.ontransport = true;
        stateIn.relativePosition = Vector4(1, 2, 3, 1);
        stateIn.transport_seat = 1;
        testClientMoveStateSerialization(testState,stateIn);

        stateIn.moveFlags.falling = true;
        stateIn.jump_horizontalVelocity = 9.f;
        stateIn.jump_verticalVelocity = 8.f;
        testClientMoveStateSerialization(testState,stateIn);

        stateIn.moveFlags.spline_elevation = true;
        stateIn.spline_elevation = 2.f;
        testClientMoveStateSerialization(testState,stateIn);

        stateIn.moveFlags.allow_pitching = true;
        stateIn.pitchAngle = 0.5f;
        testClientMoveStateSerialization(testState,stateIn);
    }

    TEST(MoveSpline, MoveSplineInitArgs)
    {
        MoveSplineInitArgs ar;
        EXPECT_TRUE(!ar.Validate());

        ar.path.resize(2);
        ar.velocity = 10.f;
        EXPECT_TRUE(ar.Validate());

       /* extern float VelocityLimit;
        ar.velocity = VelocityLimit + 1;
        EXPECT_TRUE(!ar.Validate());
*/

        ar.velocity = -1.f;
        EXPECT_TRUE(!ar.Validate());
    }

    template<class T, size_t N> void assign(QVector<T>& vec, const T (&values)[N]) {
        vec.resize(N);
        memcpy(&vec[0], &values, N * sizeof(T));
    }

    TEST(MoveSpline, basic1)
    {
        Vector3 path[] = {
            Vector3(),
            Vector3(10,10,10),
        };

        MoveSpline mov;
        MoveSplineInitArgs ar;
        assign(ar.path, path);
        ar.velocity = 10.f;
        EXPECT_TRUE(ar.Validate());

        mov.Initialize(ar);
        EXPECT_TRUE(mov.timeTotal() > 0);
        EXPECT_TRUE(mov.timePassed() == 0);

        Vector3 pos = mov.FinalDestination();
        EXPECT_TRUE(pos.fuzzyEq(path[1]));

        pos = mov.ComputePosition().xyz();
        EXPECT_TRUE( pos.fuzzyEq(path[0]) );

        mov.updateState(mov.timeTotal());
        EXPECT_TRUE(mov.Arrived());
        EXPECT_TRUE(mov.timeTotal() == mov.timePassed());
        EXPECT_TRUE( mov.ComputePosition().xyz().fuzzyEq(path[1]) );
    }

    struct MoveSplineInitArgs_Default : public MoveSplineInitArgs
    {
        explicit MoveSplineInitArgs_Default(testing::State& testState)
        {
            Vector3 nodes[] = {
                Vector3(-4000.046f,    985.8019f,    61.02531f),
                Vector3(-3981.982f,    1017.846f,    58.96975f),
                Vector3(-3949.962f,    1033.053f,    56.85864f),
                Vector3(-3918.825f,    1014.746f,    58.33086f),
                Vector3(-3900.323f,    984.7424f,    60.60864f),
                Vector3(-3918.999f,    953.8466f,    58.96975f),
                Vector3(-3950.793f,    934.2088f,    58.96975f),
                Vector3(-3982.866f,    950.2649f,    58.96975f),
            };

            assign(path, nodes);
            velocity = 14.f;
            EXPECT_TRUE( Validate() );
        }
    };

    struct MoveSplineInitArgs_Cyclic : public MoveSplineInitArgs_Default
    {
        explicit MoveSplineInitArgs_Cyclic(testing::State& testState) : MoveSplineInitArgs_Default(testState)
        {
            flags.cyclic = true;
            flags.catmullrom = true;
            EXPECT_TRUE( Validate() );
        }
    };

    TEST(MoveSpline, basic2)
    {
        MoveSpline mov;
        MoveSplineInitArgs_Default arg(testState);
        mov.Initialize(arg);

        struct UpdateResultHandler
        {
            explicit UpdateResultHandler(MoveSpline& spl, testing::State& state) : spline(spl), testState(state) {
                point = spline.currentPathPointIdx();
                prevResult = MoveSpline::Result_None;
                receiveCounter = 0;
            }

            MoveSpline& spline;
            int32 point;
            MoveSpline::UpdateResult prevResult;
            int32 receiveCounter;
            testing::State& testState;

            void operator()(MoveSpline::UpdateResult res)
            {
                if (res == MoveSpline::Result_None)
                    return;

                int32 currPoint = spline.currentPathPointIdx();
                int32 difference = currPoint - point;
                EXPECT_TRUE( difference == 1 );
                EXPECT_TRUE( currPoint == (++receiveCounter) );
                point = currPoint;

                if (res == MoveSpline::Result_Arrived)
                {
                    EXPECT_TRUE( prevResult != MoveSpline::Result_Arrived); // receive Result_Arrived only once
                    EXPECT_TRUE( spline.Arrived() );
                    EXPECT_TRUE( spline.timeTotal() == spline.timePassed() );
                }

                prevResult = res;
            }
        };

        UpdateResultHandler hdl(mov, testState);

        while (!mov.Arrived())
        {
            int32 diffTime = mov.timeInNextPoint() - mov.timePassed();
            EXPECT_TRUE( diffTime >= 0);

            mov.updateState(diffTime, hdl);

            Vector4 loc = mov.ComputePosition();
            EXPECT_TRUE( loc.isFinite() );
            EXPECT_TRUE( arg.path[mov.currentPathPointIdx()].fuzzyEq( loc.xyz() ) );
        }
    }

    TEST(MovementMessage, basic)
    {
        MovementMessage msg(NULL);
        EXPECT_TRUE( msg.OrigTime() == 0 );
        EXPECT_TRUE( msg.Source() == NULL );

        ClientMoveState state;
        state.ms_time = 1989;
        msg << state;
        EXPECT_TRUE( msg.OrigTime() == state.ms_time );
    }
}
