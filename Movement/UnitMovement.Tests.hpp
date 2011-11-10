#include "MoveSpline.h"
#include "gtest/gtest.h"

namespace Movement
{
    TEST(UnitMovement, UnitMoveFlag)
    {
        UnitMoveFlag::eUnitMoveFlags Flag = UnitMoveFlag::AllowSwimFlyTransition;
        UnitMoveFlag f(Flag);
        check( f.hasFlag(Flag) );
        check( (f & Flag) != 0 );
        check( f.raw == Flag );
        check( f.allowSwimFlyTransition );
        check( f.ToString().find("AllowSwimFlyTransition") != std::string::npos );
        check( sizeof(f.raw) == sizeof(UnitMoveFlag) );
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

    TEST(MoveSpline, MoveSplineInitArgs)
    {
        MoveSplineInitArgs ar;
        check(!ar.Validate());

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

        Location pos = mov->FinalDestination();
        EXPECT_TRUE(pos.fuzzyEq(path[1]));

        pos = mov->ComputePosition();
        EXPECT_TRUE( pos.fuzzyEq(path[0]) );

        mov->updateState(mov->Duration());
        EXPECT_TRUE(mov->Finalized());
        EXPECT_TRUE( mov->ComputePosition().fuzzyEq(path[1]) );
        delete mov;
    }

    void initCyclicMoveSpline(MoveSpline& mov, MoveSplineInitArgs& arg)
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

        arg = MoveSplineInitArgs();
        arg.path.assign(nodes, nodes + CountOf(nodes));
        arg.velocity = 14.f;
        arg.flags.cyclic = true;
        arg.flags.catmullrom = true;
        check( arg.Validate() );

        mov.Initialize(arg);
        check(mov.isCyclic());
        check(mov.Duration() > 0);
    }

    TEST(MoveSpline, basic2)
    {
        MoveSpline mov;
        MoveSplineInitArgs arg;
        initCyclicMoveSpline(mov, arg);

        int32 testTime = mov.Duration() * 2;
        while(testTime > 0)
        {
            int32 next = mov.next_timestamp() - mov.timePassed();
            EXPECT_TRUE( next >= 0);
            testTime -= next;

            struct UpdateResultHandler {
                std::vector<MoveSpline::UpdateResult> results;
                void operator()(MoveSpline::UpdateResult res) {
                    results.push_back(res);
                }
            };

            UpdateResultHandler hdl;
            mov.updateState(next, hdl);

            EXPECT_TRUE( hdl.results.size() == 1);
            EXPECT_TRUE( hdl.results.back() != MoveSpline::Result_Arrived); // never possible since it's infinite, cyclic spline
            Location loc = mov.ComputePosition();
            EXPECT_TRUE( loc.isFinite() );
            EXPECT_TRUE( arg.path[mov.currentPathIdx()].fuzzyEq( loc ) );
        }

        testTime = mov.Duration() * 2;
        while(testTime > 0)
        {
            int32 next = 100;
            testTime -= next;
            mov.updateState(next);
            Location loc = mov.ComputePosition();
            EXPECT_TRUE( loc.isFinite() );
            int32 pathIdx = mov.currentPathIdx();
            EXPECT_TRUE( pathIdx < (int32)arg.path.size() );
            EXPECT_TRUE( pathIdx >= 0 );

            Vector3 vertex = arg.path[mov.currentPathIdx()];
            EXPECT_TRUE( vertex.isFinite() );
        }
    }
}
