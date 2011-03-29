#pragma once

#include "UnitMoveFlags.h"
#include "mov_constants.h"
#include <math.h>

namespace Movement
{

    double gravity = 19.29110527038574;

    /// Velocity bounds that makes fall speed limited
    float terminalVelocity = 60.148003f;
    float terminalSavefallVelocity = 7.f;

    // original client's code:
    double CMovement__CalcFallStartElevation(float time_, int _boolean, float start_velocity_)
    {
        double terminal_velocity; // st7@2
        double start_velocity; // st6@4
        double time; // st5@6
        double result; // st7@7

        if ( _boolean )
            terminal_velocity = terminalSavefallVelocity;
        else
            terminal_velocity = terminalVelocity;
        start_velocity = start_velocity_;
        if ( start_velocity_ > terminal_velocity )
            start_velocity = terminal_velocity;
        time = time_;
        if ( 19.29110527038574 * time_ + start_velocity > terminal_velocity )
            result = terminal_velocity * (time - (terminal_velocity - start_velocity) * 0.05183736234903336)
            + (start_velocity + 9.645552635192871 * (terminal_velocity - start_velocity) * 0.05183736234903336)
            * (terminal_velocity - start_velocity)
            * 0.05183736234903336;
        else
            result = time_ * (start_velocity + time * 9.645552635192871);
        return result;
    }

    const float terminal_length = (terminalVelocity * terminalVelocity) / (2.f * gravity);
    const float terminal_savefall_length = (terminalSavefallVelocity * terminalSavefallVelocity) / (2.f * gravity);

    const float terminalFallTime = terminalVelocity/gravity; // the time that needed to reach terminalVelocity

    float computeFallTime(float path_length, bool isSafeFall)
    {
        if (path_length < 0.f)
            return 0.f;

        float time;
        if ( isSafeFall )
        {
            if (path_length >= terminal_savefall_length)
                time = (path_length - terminal_savefall_length)/terminalSavefallVelocity + terminalSavefallVelocity/gravity;
            else
                time = sqrtf(2.f * path_length/gravity);
        }
        else
        {
            if (path_length >= terminal_length)
                time = (path_length - terminal_length)/terminalVelocity + terminalFallTime;
            else
                time = sqrtf(2.f * path_length/gravity);
        }

        return time;
    }

    float computeFallElevation( float t_passed, bool isSafeFall, float start_velocity )
    {
        float termVel;
        float result;

        if ( isSafeFall )
            termVel = terminalSavefallVelocity;
        else
            termVel = terminalVelocity;

        if ( start_velocity > termVel )
            start_velocity = termVel;

        float terminal_time = terminalFallTime - start_velocity / gravity; // the time that needed to reach terminalVelocity

        if ( t_passed > terminal_time )
        {
            result = terminalVelocity*(t_passed - terminal_time) +
                start_velocity*terminal_time + gravity*terminal_time*terminal_time*0.5f;
        }
        else
            result = t_passed * (start_velocity + t_passed * gravity * 0.5f);

        return result;
    }

    float computeFallElevation(float t_passed)
    {
        float result;

        if (t_passed > terminalFallTime)
        {
            //result = terminalVelocity * (t_passed - terminal_time) + gravity*terminal_time*terminal_time*0.5f;
            // simplified view:
            result = terminalVelocity * (t_passed - terminalFallTime) + terminal_length;
        }
        else
            result = t_passed * t_passed * gravity * 0.5f;

        return result;
    }
}
