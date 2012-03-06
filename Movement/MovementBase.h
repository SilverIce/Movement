
/**
  file:         MovementBase.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/Component.h"
#include "ObjectGuid.h"

class WorldObject;

namespace Movement
{
    struct WowObject : ComponentT<WowObject>
    {
        ObjectGuid guid;
        WorldObject* object;

        explicit WowObject() : object(nullptr) {}
    };
}
