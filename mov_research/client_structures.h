#pragma once

#include "typedefs.h"
#include "containers.h"
#include "client_constants.h"

#pragma pack(push,1)

enum SplineMode
{
    SplineModeLinear       = 0,
    SplineModeCatmullRom   = 1,
    SplineModeBezier3      = 2,
};

struct C3Vector
{
    float x;
    float y;
    float z;
};

struct C2Vector
{
    float x;
    float y;
};


struct C4Vector
{
    float pos_X;
    float pos_Y;
    float pos_Z;
    float orient;
};

struct objGUID
{
    uint32 obj_lowGUID;
    uint32 obj_HIGUID;
    uint32 type_id;
    uint32 data;
};

struct C34Matrix //struc ; (sizeof=0x30, standard type
{
    float a0;
    float a1;
    float a2;
    float b0;
    float b1;
    float b2;
    float c0;
    float c1;
    float c2;
    float d0;
    float d1;
    float d2;
};

struct C44Matrix //struc ; (sizeof=0x40, standard type
{
    float a0;
    float a1;
    float a2;
    float a3;
    float b0;
    float b1;
    float b2;
    float b3;
    float c0;
    float c1;
    float c2;
    float c3;
    float d0;
    float d1;
    float d2;
    float d3;
};

struct C3Spline
{
    struct C3Spline__vTable *vTable;
    float CachedLength;
    C3Vector Points[25];
    TSGrowableArray_C3Vector PointOverflowList;
    int CachedSegLengthCount;
    float CachedSegLengths[25];

    //TSGrowableArray_float segLengthsList;

    int field_1AC;
    int field_1B0;
    int field_1B4;
    int field_1B8;
    int IntermediatePointCount;
};

struct C3Spline__vTable
{
    float (__thiscall *IGetLength)(C3Spline *spline);
    void (__thiscall *IValidateCache)(C3Spline *spline);
    int IPosArcLength;
    int IPosParametric;
    int IVelArcLength;
    int IVelParametric;
    int IFrameArcLength;
    void (__stdcall *ISetPoints)(C3Vector *points, int pointCount);
};

struct C3Spline_CatmullRom
{
    C3Spline    baseclass_0;
    SplineMode splineMode; 
};


//444
struct CMoveSpline   //SMemAlloc(544, (int)".\\Movement_C.cpp", 0xA6u, 0);  ~544 bytes
{
    int field_0;
    int field_4;
    int field_8;
    int field_C;

    union SplineFaceData
    {
        struct Point{
            float x,y,z;
        } spot;
        uint64 target;
        float angle;
    };

    SplineFaceData face;

    uint32 data1[1];//32;
    uint32 splineflags;//36
    uint32 xz1;
    uint32 move_time_passed;
    uint32 move_time_full;
    uint32 time_stamp;

    C3Spline_CatmullRom CatmullRom;
    C3Vector FinalDestinationPoint;

    float some_coeff;
    float sync_coeff;
    float parabolic_speed;//524
    uint32 parabolic_time;//528

    int field_210;
    int field_214;
    int field_218;
    int field_21C;
};

struct WGUID
{
    int LowPart;
    int HighPart;
};

struct CMovement
{
    uint8 data0[8];//8

    WGUID m_transportGUID; //objGUID transport;//8
    C3Vector Position;

    int field_1C;
    float Facing;
    float Pitch;
    WGUID *Guid;
    int UnkFlags_field_2C;
    int field_30;
    int field_34;
    C3Vector GroundNormal;

    uint32 m_moveFlags;
    /////////// 29
    uint16 m_moveFlags2;
    uint16 data21;
    C3Vector m_position;

    float AnchorFacing;
    float AnchorPitch;
    int MoveStartTime;
    C3Vector Direction3d;
    C2Vector Direction2d;

    float m_cosAnchorPitch;
    float m_sinAnchorPitch;

    int TimeFallen;
    float FallStartElevation;
    int SplineFloat_Elevation_field_88;

    struct SpeedBlock
    {
        float current;
        float walk;
        float run;
        float run_back;
        float speed_8;
        float speed_9;
        float flight;
        float flight_back;
        float speed_12;
        float speed_13;
        float speed_14;
        float m_jumpVelocity;
    } speed;
    //////////
    CMoveSpline * m_spline;//188

    int UpdateTimeMs_field_C0;
    int LastEventTime;
    int field_C8;
    int field_CC;
    float f_field_D0;
    C3Vector PositionDiffFromLastMoveEvent;
    int FacingDiffFromLastMoveEvent;
    int PitchDiffBetweenLastMoveEvent;
    __int16 field_E8[32];
    int field_128;
    int MSTime_field_12C;
    int TimeSinceLastMoveEvent;
    int MSTime_field_134;
    int field_138;
    int field_13C;
    struct CPlayerMoveEvent *PlayerMoveEventList_Head_field_140;
    struct CGUnit_C *Unit;
};

struct ObjectVFuncs;
struct sUnitFields;
struct CreatureDisplayInfoEntry;

struct WorldObject
{
    ObjectVFuncs* fv;//4
    uint32 d2;//8
    objGUID* guid_ptr;//12
    uint8 datas[196];					//208
    sUnitFields* fields;

    //CMoveSpline* splineInfo;
    uint8 data0[1714];//1914,  size 1718

    CMovement m_info;//1928

    uint32 data5[38];
    //CreatureDisplayInfoEntry* displInfo_idx;
    //uint32 data4[46];

    //uint32 some_flags; //660  , real 2608, 2588
};


struct UnitModel
{
    uint32 data00;
    uint32 flags;
    uint32 data2;

    uint32 data3;

    float scale;
    uint32 data[9];
    float widht;
    float colliz_max;
};

#pragma pack(pop)
