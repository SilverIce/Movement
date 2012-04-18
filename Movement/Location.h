#pragma once
#include <G3D/Vector3.h>
#include <QtCore/QString>

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

        QString toString() const {
            return QString().sprintf("(%f %f %f %f)", x,y,z,orientation);
        }

        float orientation;
    };
}
