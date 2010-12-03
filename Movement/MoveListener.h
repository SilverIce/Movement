#pragma once

namespace Movement
{
    class IListener
    {
    public:

        virtual ~IListener() {}

        virtual void OnSplineDone() {}

        virtual void OnEvent(int eventId) {};
    };

}