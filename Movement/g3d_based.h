
/**
  file:         g3d_based.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "G3D/Spline.h"

class GD3_spline : public G3D::Spline<G3D::Vector3>
{
public:
    typedef  G3D::Spline<G3D::Vector3> base;

    void append(G3D::Vector3 &v);

    void evaluate(float time, G3D::Vector3& c, const G3D::Matrix4& basis ) const;

    void settime(int i, float s) { time[i] = s; }
    float gettime(int i) const { return time[i]; }
};
