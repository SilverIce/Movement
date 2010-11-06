
#include <stddef.h>
#include <stdio.h>
#include <conio.h>

#include "client_structures.h"

#define CHECK_OFFSET(offset, s, m)\
    if ( offset != offsetof(s,m) )\
    {\
        printf("\noffsetcheck %s failed\n", # offset);\
        printf("real address %u != %u member adress\n\n", offset, offsetof(s,m));\
    }\
    else\
        printf("offsetcheck %s succeded\n", # offset);\

#define CHECK_SIZE(size, T)\
    if ( size != sizeof(T) )\
    {\
        printf("\nsizecheck %s failed\n", # size);\
        printf("real size 0x%X != 0x%X object size\n\n", size, sizeof(T));\
    }\
    else\
        printf("sizecheck %s succeded\n", # size);\

enum
{
    CMovement_SIZE = 0x148,
    SplineInfo_SIZE = 0x224,
    C3Spline_SIZE   = 0x150,


    FIELD_SPLINE_FLAGS  = 0x20,
    FIELD_SPLINE_PATH = 0x34,
    FIELD_SPLINE_PARABOLIC_SPEED = 0x20C,
    FIELD_SPLINE_MOV_TIME_PASSED = 0x28,

    FIELD_MOVE_FLAGS = 0x44,
    FIELD_TRANSPORT_GUID = 0x8,
    FIELD_POSITION = 0x4C,
    FIELD_SPLINE_INFO_PTR = 0xBC,
    FIELD_SKIPPED_TIME = 0xC4,
};


void CheckOffsets()
{
    CHECK_SIZE(CMovement_SIZE, CMovement);
    CHECK_SIZE(SplineInfo_SIZE, CMoveSpline);
    CHECK_SIZE(C3Spline_SIZE, C3Spline);

    CHECK_OFFSET(FIELD_SPLINE_FLAGS, CMoveSpline, CMoveSpline::splineflags);
    //CHECK_OFFSET(FIELD_SPLINE_PATH, CMoveSpline, CMoveSpline::m_spline);
    CHECK_OFFSET(FIELD_SPLINE_MOV_TIME_PASSED, CMoveSpline, CMoveSpline::move_time_passed);
    CHECK_OFFSET(FIELD_SPLINE_PARABOLIC_SPEED, CMoveSpline, CMoveSpline::z_acceleration);

    CHECK_OFFSET(FIELD_MOVE_FLAGS, CMovement, CMovement::m_moveFlags);
    CHECK_OFFSET(FIELD_TRANSPORT_GUID, CMovement, CMovement::m_transportGUID);
    CHECK_OFFSET(FIELD_POSITION, CMovement, CMovement::m_position);
    CHECK_OFFSET(FIELD_SPLINE_INFO_PTR, CMovement, CMovement::m_spline);
    //CHECK_OFFSET(FIELD_SKIPPED_TIME, CMovement, CMovement::some_skipped_time);
    ;
}


int main()
{
    CheckOffsets();
    //_getch();

    return 0;
} 
 
