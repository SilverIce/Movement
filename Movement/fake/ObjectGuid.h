#pragma once

class ObjectGuid
{
public:
    const ObjectGuid& ReadAsPacked() const { return *this;}
    const ObjectGuid& WriteAsPacked() const { return *this;}
    int GetRawValue() const { return 0;}
    void Set(int rawValue) {}
};

struct PackedGuid
{
    PackedGuid(){}
    PackedGuid(int){}
};
