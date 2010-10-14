#pragma once

#include "typedefs.h"

#pragma pack(push,1)

// IDA doesn't supports templates

#define TSBaseArray(T)\
    struct TSBaseArray_## T \
    {\
        uint32 m_alloc;\
        uint32 m_count;\
        T * m_data;\
    }\

#define TSFixedArray(T)\
    TSBaseArray(T);\
    struct TSFixedArray_## T\
    {\
        TSBaseArray_## T baseclass_0;\
    }\

#define TSGrowableArray(T)\
    TSFixedArray(T);\
    struct TSGrowableArray_## T\
    {\
        TSFixedArray_## T baseclass_0;\
        uint32 m_chunk;\
    }\

struct C3Vector;

TSGrowableArray(float);
TSGrowableArray(C3Vector);

#pragma pack(pop)
