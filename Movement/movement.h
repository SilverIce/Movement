
#pragma once

#include "typedefs.h"
#include "mov_constants.h"
#include "packet_builder.h"
#include "SplineState.h"
#include "G3D\Vector3.h"
#include "G3D\Vector4.h"


namespace Movement
{
    class WorldObject;


    class MovementState
    {
        friend class PacketBuilder;

    public:
        MovementState() : msg_builder(this, MovControlServer)
        {
            move_mode = 0;
            time_stamp = 0;
            moveFlags = 0;
            move_flags2 = 0;

            speed_obj.init();
        }

        ~MovementState()
        {
        }


        #pragma region methtods
        PacketBuilder& GetBuilder() { return msg_builder; }
        PacketBuilder msg_builder;

        //WorldObject* wow_object;

        /// Get-Set methtods

        /// Speed
        void SetSpeed(SpeedType type, float s) { speed[type] = s; }
        float GetSpeed(SpeedType type) const { return speed[type]; }
        float GetCurrentSpeed() const { return speed_obj.current; }

        /// Movement flags
        void AddMovementFlag(uint32 f) { moveFlags |= f; }
        void RemoveMovementFlag(uint32 f) { moveFlags &= ~f; }
        bool HasMovementFlag(uint32 f) const { return moveFlags & f; }
        uint32 GetMovementFlags() const { return moveFlags; }
        void SetMovementFlags(uint32 f) { moveFlags = f; }

        /// Direction flags
        void AddDirectionFlag(uint8 f) { direction_flags |= f; }
        void RemoveDirectionFlag(uint8 f) { direction_flags &= ~f; }
        bool HasDirectionFlag(uint8 f) const { return direction_flags & f; }
        uint8 GetDirectionFlags() const { return direction_flags; }
        void SetDirectionFlags(uint8 f) { direction_flags = f; }

        bool HasDest() const { return direction_flags & DIRECTIONS_MASK; }
        void Stop()  { direction_flags &= ~DIRECTIONS_MASK; }

        /// Move Modes
        bool HasMode(MoveMode m) const { return move_mode & (1 << m);}

        void ApplyMoveMode(MoveMode mode, bool apply)
        {
            if (apply)
            {
                AddMovementFlag(Mode2Flag_table[mode]);
                move_mode |= (1 << mode);
            }
            else
            {
                RemoveMovementFlag(Mode2Flag_table[mode]);
                move_mode &= ~(1 << mode);
            }
        }

        /// Apply/remove modes
        void Root(bool apply) { ApplyMoveMode(MoveModeRoot, apply); }
        void Swim(bool apply) { ApplyMoveMode(MoveModeSwim, apply); }
        void Walk(bool apply) { ApplyMoveMode(MoveModeWalk, apply); }
        void WaterWalk(bool apply) { ApplyMoveMode(MoveModeWaterwalk, apply); }
        void SlowFall(bool apply) { ApplyMoveMode(MoveModeSlowfall, apply); }
        void Fly(bool apply) { ApplyMoveMode(MoveModeFly, apply); }
        void Hover(bool apply) { ApplyMoveMode(MoveModeHover, apply); }

        /// end of Get-Set methtods
        #pragma endregion

        /// Transport info
        struct TransportData
        {
            TransportData() : t_guid(0), t_time(0) {}
            uint64 t_guid;
            Vector3 t_offset;
            uint32 t_time;
        };

        struct SpeedInfo
        {
            void init()
            {
                memcpy(this, BaseSpeed, sizeof SpeedInfo);
            }

            float current;
            float walk;
            float run;
            float run_back;
            float swim;
            float swim_back;
            float flight;
            float flight_back;
            float turn;
            float pitch;
        };

        uint32          move_mode;

        // time-position pair
        Vector3         position;
        uint32          time_stamp;

        union {
            uint8       direction_flags;
            uint32      moveFlags;
        };
        uint16          move_flags2;

        TransportData   m_transport;
   
        union {
            SpeedInfo   speed_obj;
            float       speed[SpeedMaxCount];
        };

        SplineState     spline;

        /// Some client's formulas:

        float CalculateCurrentSpeed(bool is_walking = false) const
        {
            uint32 splineflags = spline.splineflags;

            // g_moveFlags_mask - some global client's moveflag mask
            // TODO: get real value
            static uint32 g_moveFlags_mask = 0xFFFFFFFF;
            float speed = 0.0f;

            if ( !(g_moveFlags_mask & moveFlags) )
                return 0.0f;

            if ( /*!spline ||*/ splineflags & SPLINEFLAG_NO_SPLINE )
            {
                if ( moveFlags & MOVEFLAG_FLYING )
                {
                    if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.flight >= speed_obj.flight_back )
                        return speed_obj.flight_back;
                    else
                        return speed_obj.flight;
                }
                else if ( moveFlags & MOVEFLAG_SWIMMING )
                {
                    if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.swim >= speed_obj.swim_back )
                        return speed_obj.swim_back;
                    else
                        return speed_obj.swim;
                }
                else
                {
                    if ( moveFlags & MOVEFLAG_WALK_MODE || is_walking )
                    {
                        if ( speed_obj.run > speed_obj.walk )
                            return speed_obj.walk;
                    }
                    else
                    {
                        if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.run >= speed_obj.run_back )
                            return speed_obj.run_back;
                    }
                    return speed_obj.run;
               }
            }
            else
            {
                if ( !spline.duration() )
                    return 0.0f;
                //speed = spline.total_lenght / spline.duration * 1000.0f;
            }
            return speed;
        }

    };



}
