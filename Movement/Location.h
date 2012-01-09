#pragma once
#include <G3D/Vector3.h>
#include <sstream>

namespace Movement
{
    using G3D::Vector3;

    struct Location : public Vector3
    {
        explicit Location() : orientation(0) {}
        explicit Location(float x, float y, float z, float o) : Vector3(x,y,z), orientation(o) {}
        explicit Location(const Vector3& v) : Vector3(v), orientation(0) {}
        explicit Location(const Vector3& v, float o) : Vector3(v), orientation(o) {}

        bool isFinite() const
        {
            return Vector3::isFinite() && G3D::isFinite(orientation);
        }

        void operator = (const Vector3& vector)
        {
            Vector3::operator = (vector);
        }

        std::string ToString() const {
            std::stringstream str;
            str << '(' << x << ' ' << y << ' '<< z << ' ' << orientation << ')';
            return str.str();
        }

        float orientation;
    };
}
