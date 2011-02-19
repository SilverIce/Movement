
/**
  file:         MoveListener.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    class IListener
    {
    public:

        virtual ~IListener() {}

        virtual void OnSplineDone() {}

        virtual void OnEvent(int eventId, int data) {};
    };

}
