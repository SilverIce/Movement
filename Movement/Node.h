
#pragma once

#include "typedefs.h"
#include "G3D/Vector3.h"

#include <vector>

namespace Movement
{
    using G3D::Vector3;

    struct MoveNode 
    {
        MoveNode(Vector3 const& poi) : vec(poi), time(0), seg_lenght(0) {}

        Vector3 vec;
        uint32 time;
        float seg_lenght;
    };

    typedef std::vector<MoveNode> NodeList;
}
