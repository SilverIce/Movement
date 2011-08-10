#pragma once

#ifdef USE_FAKE_CLASSES

class ObjectGuid
{
public:
	ObjectGuid& ReadAsPacked() { return *this;}
	ObjectGuid& WriteAsPacked() { return *this;}
    uint64 GetRawValue() const { return 0;}
};

struct PackedGuid 
{
    PackedGuid(){}
    PackedGuid(int){}
};


#endif

