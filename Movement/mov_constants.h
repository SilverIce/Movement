
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
        MoveModeMaxCount
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

    enum MovControlType
    {
        MovControlClient,
        MovControlServer,
        MovControlCount,
    };

    float computeFallTime(float path_length, bool isSafeFall);
    float computeFallElevation(float time_passed, bool isSafeFall, float start_velocy);
    float computeFallElevation(float time_passed);
}
