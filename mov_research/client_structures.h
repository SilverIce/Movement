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
    void* vtable;
    float cachedLength;

    TSGrowableArray_Vector3 points;
    TSGrowableArray_float cachedSegLength;
};

struct C3Spline_CatmullRom
{
    C3Spline    baseclass_0;
    SplineMode splineMode : 4; 
};


//444
struct CMoveSpline   //SMemAlloc(544, (int)".\\Movement_C.cpp", 0xA6u, 0);  ~544 bytes
{
    uint32 data0[4];//16

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
    uint32* path_ptr;
    uint32 data2[111];
    uint32 isFlying;//113
    Vector3 position;
    float some_coeff;
    float sync_coeff;
    float parabolic_speed;//524
    uint32 parabolic_time;//528
    uint32 data3[4];
};

struct WorldObject;

struct CMovement
{
    uint8 data0[8];//8

    uint64 m_transportGUID; //objGUID transport;//8
    Vector3 platform_offset;
    uint32 data1[10];
    uint32 m_moveFlags;
    /////////// 29
    uint16 some_flags2;
    uint16 data21;
    C3Vector m_position;
    float float_data[8];
    float m_cosAnchorPitch;
    float m_sinAnchorPitch;

    struct SpeedBlock// 15 * 4
    {
        float data3[3];
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

    /////// 122
    uint32 data6;
    int32 some_skipped_time;
    uint32 data5[31];//33
    WorldObject* controller;    //208
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
