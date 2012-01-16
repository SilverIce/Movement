#include "gtest/gtest.h"

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
        EXPECT_TRUE( f.ToString().find("AllowSwimFlyTransition") != std::string::npos );
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
        EXPECT_TRUE( f.ToString().find("Done") != std::string::npos );
        EXPECT_TRUE( sizeof(f.raw) == sizeof(MoveSplineFlag) );
    }

    void testClientMoveStateSerialization(const ClientMoveState& stateIn)
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
        CMP(ClientMoveState::transport_position);
        CMP(ClientMoveState::transport_seat);
        CMP(ClientMoveState::transport_time);
        CMP(ClientMoveState::transport_time2);
        CMP(ClientMoveState::world_position);
        #undef CMP
    }

    TEST(ClientMoveState, serialization)
    {
        ClientMoveState stateIn;

        stateIn.world_position = Location(10, 20, 30, 1);
        stateIn.fallTime = 400;
        stateIn.ms_time = 20000;
        testClientMoveStateSerialization(stateIn);

        stateIn.moveFlags.ontransport = true;
        stateIn.transport_position = Location(1, 2, 3, 1);
        stateIn.transport_seat = 1;
        testClientMoveStateSerialization(stateIn);

        stateIn.moveFlags.falling = true;
        stateIn.jump_horizontalVelocity = 9.f;
        stateIn.jump_verticalVelocity = 8.f;
        testClientMoveStateSerialization(stateIn);

        stateIn.moveFlags.spline_elevation = true;
        stateIn.spline_elevation = 2.f;
        testClientMoveStateSerialization(stateIn);

        stateIn.moveFlags.allow_pitching = true;
        stateIn.pitchAngle = 0.5f;
        testClientMoveStateSerialization(stateIn);
    }

    TEST(MoveSpline, MoveSplineInitArgs)
    {
        MoveSplineInitArgs ar;
        EXPECT_TRUE(!ar.Validate());

        ar.path.resize(2);
        ar.velocity = 10.f;
        EXPECT_TRUE(ar.Validate());

        extern float VelocityLimit;
        ar.velocity = VelocityLimit + 1;
        EXPECT_TRUE(!ar.Validate());

        ar.velocity = -1.f;
        EXPECT_TRUE(!ar.Validate());
    }

    TEST(MoveSpline, basic1)
    {
        Vector3 path[] = {
            Vector3(),
            Vector3(10,10,10),
        };

        MoveSpline * mov = NULL;
        MoveSplineInitArgs ar;
        ar.path.assign(path, path + CountOf(path));
        ar.velocity = 10.f;
        EXPECT_TRUE(ar.Validate());

        EXPECT_TRUE(MoveSpline::Initialize(mov,ar));
        EXPECT_TRUE(mov->Duration() > 0);
        EXPECT_TRUE(mov->timePassed() == 0);

        Vector3 pos = mov->FinalDestination();
        EXPECT_TRUE(pos.fuzzyEq(path[1]));

        pos = mov->ComputePosition();
        EXPECT_TRUE( pos.fuzzyEq(path[0]) );

        mov->updateState(mov->Duration());
        EXPECT_TRUE(mov->Arrived());
        EXPECT_TRUE(mov->Duration() == mov->timePassed());
        EXPECT_TRUE( mov->ComputePosition().fuzzyEq(path[1]) );
        delete mov;
    }

    struct MoveSplineInitArgs_Default : public MoveSplineInitArgs
    {
        explicit MoveSplineInitArgs_Default()
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

            path.assign(nodes, nodes + CountOf(nodes));
            velocity = 14.f;
            EXPECT_TRUE( Validate() );
        }
    };

    struct MoveSplineInitArgs_Cyclic : public MoveSplineInitArgs_Default
    {
        explicit MoveSplineInitArgs_Cyclic()
        {
            flags.cyclic = true;
            flags.catmullrom = true;
            EXPECT_TRUE( Validate() );
        }
    };

    TEST(MoveSpline, basic2)
    {
        MoveSpline mov;
        MoveSplineInitArgs_Default arg;
        mov.Initialize(arg);

        struct UpdateResultHandler
        {
            explicit UpdateResultHandler(MoveSpline& spl) : spline(spl) {
                point = spline.currentPathPointIdx();
                prevResult = MoveSpline::Result_None;
                receiveCounter = 0;
            }

            MoveSpline& spline;
            int32 point;
            MoveSpline::UpdateResult prevResult;
            int32 receiveCounter;

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
                    EXPECT_TRUE( spline.Duration() == spline.timePassed() );
                }

                prevResult = res;
            }
        };

        UpdateResultHandler hdl(mov);

        while (!mov.Arrived())
        {
            int32 diffTime = mov.next_timestamp() - mov.timePassed();
            EXPECT_TRUE( diffTime >= 0);

            mov.updateState(diffTime, hdl);

            Location loc = mov.ComputePosition();
            EXPECT_TRUE( loc.isFinite() );
            EXPECT_TRUE( arg.path[mov.currentPathPointIdx()].fuzzyEq( loc ) );
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
