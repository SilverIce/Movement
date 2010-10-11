#pragma once

#include "typedefs.h"

#pragma pack(push,1)

// IDA doesn't supports templates

#define TSBaseArray(T)\
    struct TSBaseArray_## T \
    {\
        void* vtable;\
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

struct Vector3;

TSGrowableArray(float);
TSGrowableArray(Vector3);

#pragma pack(pop)
