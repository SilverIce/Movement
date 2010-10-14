#pragma once

#include "typedefs.h"

namespace Movement {

    template<class T,int,size_t> class G3D::Array;

    // abstract interface 
    class IPathFinder
    {
        virtual bool makePath(const Vector3 & a, const Vector3& b, G3D::Array<Vector3>& path) const = 0;
    };

}