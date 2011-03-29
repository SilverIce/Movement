#pragma once

#include "UnitMoveFlags.h"
#include "mov_constants.h"
#include "opcodes.h"
#include <math.h>

namespace Movement
{
    const double gravity = 19.29110527038574;

    const float terminalVelocity = 60.148003f;

    const float savefallVelocity = 7.f;

    float computeFallTime(float path_length, bool isSafeFall)
    {
        static const float terminal_length = (terminalVelocity * terminalVelocity) / (2.f * gravity);
        static const float safe_terminal_length = (savefallVelocity * savefallVelocity) / (2.f * gravity);

        float time;
        if ( isSafeFall )
        {
            if (path_length >= safe_terminal_length)
                time = (path_length - safe_terminal_length)/savefallVelocity + savefallVelocity/gravity;
            else
                time = sqrtf(2.f * path_length/gravity);
        }
        else
        {
            if (path_length >= terminal_length)
                time = (path_length - terminal_length)/terminalVelocity + terminalVelocity/gravity;
            else
                time = sqrtf(2.f * path_length/gravity);
        }

        return time;
    }

    float computeFallElevation( float t_passed, bool isSafeFall, float start_velocy )
    {
        float termVel;
        float result;

        if ( isSafeFall )
            termVel = savefallVelocity;
        else
            termVel = terminalVelocity;

        if ( start_velocy > termVel )
            start_velocy = termVel;

        if ( gravity * t_passed + start_velocy > termVel )
        {
            float dvel = (termVel - start_velocy);
            result = termVel * (t_passed - dvel/gravity) + dvel*(start_velocy + dvel / 2.f) / gravity;
        }
        else
            result = t_passed * (start_velocy + t_passed * gravity / 2.f);

        return result;
    }
}
