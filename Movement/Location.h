#pragma once

namespace Movement
{
    class Location
    {
    public:
        float x, y, z, orientation;

        explicit Location()
            : x(0)
            , y(0)
            , z(0)
            , orientation(0)
        {}

        explicit Location(float _x, float _y, float _z, float o)
            : x(_x)
            , y(_y)
            , z(_z)
            , orientation(o)
        {}
    };
}
