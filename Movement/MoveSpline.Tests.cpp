#include "MoveSpline.h"

namespace Movement
{
    void testMoveSplineFlag()
    {
        MoveSplineFlag::eFlags Flag = MoveSplineFlag::Done;
        MoveSplineFlag f(Flag);
        check( f.hasFlag(Flag) );
        check( f & Flag );
        check( f.raw == Flag );
        check( f.done );
        check( f.ToString().find("Done") != std::string::npos );
        check( sizeof(f.raw) == sizeof(MoveSplineFlag) );
    }

    void testMoveSplineInitArgs()
    {
        MoveSplineInitArgs ar;
        check(!ar.Validate());

        ar.path.resize(2);
        ar.velocity = 10.f;
        check(ar.Validate());

        extern float terminalVelocity;
        ar.velocity = terminalVelocity + 1;
        check(!ar.Validate());

        ar.velocity = -1.f;
        check(!ar.Validate());
    }

    void testMoveSpline()
    {
        Vector3 path[] = {
            Vector3(),
            Vector3(10,10,10),
        };

        MoveSpline * mov = NULL;
        MoveSplineInitArgs ar;
        ar.path.assign(path, path + CountOf(path));
        ar.path.push_back(Vector3(10,10,0));
        ar.velocity = 10.f;
        check(ar.Validate());

        check(MoveSpline::Initialize(mov,ar));
        check(mov->Duration() > 0);
        check(mov->FinalDestination().fuzzyEq(path[1]));
        check( mov->ComputePosition(Location()).fuzzyEq(path[0]) );

        mov->updateState(mov->Duration());
        check(mov->Finalized());
        check( mov->ComputePosition(Location()).fuzzyEq(path[1]) );
        delete mov;
    }

    void MoveSplineTests()
    {
        testMoveSplineFlag();
        testMoveSplineInitArgs();
        testMoveSpline();
    }
}
