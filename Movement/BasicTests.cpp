#include "typedefs_p.h"
#include "MSTime.h"

namespace Movement
{
    static_assert(true, "");
    static_assert(sizeof(uint8) == 1, "");
    static_assert(sizeof(int8) == 1, "");
    static_assert(sizeof(uint16) == 2, "");
    static_assert(sizeof(int16) == 2, "");
    static_assert(sizeof(uint32) == 4, "");
    static_assert(sizeof(int32) == 4, "");
    static_assert(sizeof(uint64) == 8, "");
    static_assert(sizeof(int64) == 8, "");

    void testMSTime()
    {
        const MSTime t2(2), t4(4);

        check( (t4 - t2) == MSTime(2) );
        check( (t4 + t2) == MSTime(6) );
        check( t4 > t2 && t4 >= t2 );
        check( t2 < t4 && t2 <= t4 );
        check( t2 != t4 );
        check( (t2 + MSTime(2)) == t4 );
        check( (t2 - t4) == (2 + (0xFFFFFFFF - 4)) );
    }

    extern void UnitMovementTests();

    void RunTests()
    {
        testMSTime();

        UnitMovementTests();
    }
}
