
/**
  file:         mov_constants.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    enum MoveMode
    {
        MoveModeWalk,
        MoveModeRoot,
        MoveModeSwim,
        MoveModeWaterwalk,
        MoveModeSlowfall,
        MoveModeHover,
        MoveModeFly,
        MoveModeGravityDisabled,
        MoveModeCanFly,
        MoveModeCanSwimFlyTransition,
        MoveMode_End
    };

    enum SpeedType
    {
        SpeedWalk           = 0,
        SpeedRun            = 1,
        SpeedSwimBack       = 2,
        SpeedSwim           = 3,
        SpeedRunBack        = 4,
        SpeedFlight         = 5,
        SpeedFlightBack     = 6,
        SpeedTurn           = 7,
        SpeedPitch          = 8,

        SpeedMaxCount       = 9,
    };

    extern double gravity;
    double computeFallTime(float path_length);
    double computeFallElevation(float time_passed, float start_velocy);
    double computeFallElevation(float time_passed);

    double computeSafeFallTime(float path_length);
    double computeSafeFallElevation(float time, float start_velocity);
}
