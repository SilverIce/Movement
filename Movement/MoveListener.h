
/**
  file:         MoveListener.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    struct OnEventArgs;
    class IListener
    {
    protected:
        ~IListener() {}
    public:
        virtual void OnEvent(const OnEventArgs& args) = 0;
    };
}
