namespace Movement
{
    double gravity = 19.29110527038574;

    /// Velocity bounds that makes fall speed limited
    float terminalVelocity = 60.148003f;
    float terminalSavefallVelocity = 7.f;

    double Gravity() {
        return gravity;
    }

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

    double terminal_length = (terminalVelocity * terminalVelocity) / (2 * gravity);
    double terminal_savefall_length = (terminalSavefallVelocity * terminalSavefallVelocity) / (2 * gravity);

    double terminalFallTime = terminalVelocity/gravity; // the time that needed to reach terminalVelocity

    double computeFallTime(float path_length)
    {
        if (path_length < 0)
            return 0;

        double time;
        if (path_length >= terminal_length)
            time = (path_length - terminal_length)/terminalVelocity + terminalFallTime;
        else
            time = sqrt(2 * path_length/gravity);
        return time;
    }

    double computeSafeFallTime(float path_length)
    {
        if (path_length < 0)
            return 0;

        double time;
        if (path_length >= terminal_savefall_length)
            time = (path_length - terminal_savefall_length)/terminalSavefallVelocity + terminalSavefallVelocity/gravity;
        else
            time = sqrt(2 * path_length/gravity);
        return time;
    }

    double computeFallElevation(float t_passed, float start_velocity)
    {
        if (start_velocity > terminalVelocity)
            start_velocity = terminalVelocity;

        double terminal_time = terminalFallTime - start_velocity / gravity; // the time that needed to reach terminalVelocity
        double elevation = 0;

        if (t_passed > terminal_time)
        {
            elevation = terminalVelocity*(t_passed - terminal_time) +
                start_velocity*terminal_time + gravity*terminal_time*terminal_time*0.5;
        }
        else
            elevation = t_passed * (start_velocity + t_passed * gravity * 0.5);

        return elevation;
    }

    double computeSafeFallElevation(float time, float start_velocity)
    {
        if (start_velocity > terminalSavefallVelocity)
            start_velocity = terminalSavefallVelocity;

        double terminal_time = terminalFallTime - start_velocity / gravity; // the time that needed to reach terminalVelocity
        double elevation = 0;

        if (time > terminal_time)
        {
            elevation = terminalVelocity*(time - terminal_time) +
                start_velocity*terminal_time + gravity*terminal_time*terminal_time*0.5;
        }
        else
            elevation = time * (start_velocity + time * gravity * 0.5);

        return elevation;
    }

    double computeFallElevation(float time)
    {
        double elevation;

        if (time > terminalFallTime)
        {
            //result = terminalVelocity * (t_passed - terminal_time) + gravity*terminal_time*terminal_time*0.5f;
            // simplified view:
            elevation = terminalVelocity * (time - terminalFallTime) + terminal_length;
        }
        else
            elevation = time * time * gravity * 0.5;

        return elevation;
    }

#define STR(x) #x

    template<class Flags, int N>
    void print_flags(Flags flag, const char * (&names)[N], QString& str)
    {
        if (flag == 0)
        {
            str += "None";
            return;
        }

        static_assert(N <= (sizeof(Flags) * 8), "flag names array size is always lesser or equal to flag bits amount");

        for (int i = 0, flagsAppended = 0; i < N; ++i)
        {
            if ((flag & (Flags(1) << i)) && names[i] != NULL) {
                if (flagsAppended > 0)
                    str += " ";
                str += names[i];
                ++flagsAppended;
            }
        }
    }

    QString UnitMoveFlag::toString() const
    {
        const char * g_MovementFlag_names[]=
        {
            STR(Forward            ),// 0x00000001,
            STR(Backward           ),// 0x00000002,
            STR(Strafe_Left        ),// 0x00000004,
            STR(Strafe_Right       ),// 0x00000008,
            STR(Turn_Left          ),// 0x00000010,
            STR(Turn_Right         ),// 0x00000020,
            STR(Pitch_Up           ),// 0x00000040,
            STR(Pitch_Down         ),// 0x00000080,

            STR(Walk               ),// 0x00000100,               // Walking
            STR(Ontransport        ),// 0x00000200,
            STR(Levitation         ),// 0x00000400,
            STR(Root               ),// 0x00000800,
            STR(Falling            ),// 0x00001000,
            STR(Fallingfar         ),// 0x00002000,
            STR(Pendingstop        ),// 0x00004000,
            STR(PendingStrafestop  ),// 0x00008000,
            STR(Pendingforward     ),// 0x00010000,
            STR(Pendingbackward    ),// 0x00020000,
            STR(PendingStrafeleft  ),// 0x00040000,
            STR(PendingStraferight ),// 0x00080000,
            STR(Pendingroot        ),// 0x00100000,
            STR(Swimming           ),// 0x00200000,               // Appears With Fly Flag Also
            STR(Ascending          ),// 0x00400000,               // Swim Up Also
            STR(Descending         ),// 0x00800000,               // Swim Down Also
            STR(Can_Fly            ),// 0x01000000,               // Can Fly In 3.3?
            STR(Flying             ),// 0x02000000,               // Actual Flying Mode
            STR(Spline_Elevation   ),// 0x04000000,               // Used For Flight Paths
            STR(Spline_Enabled     ),// 0x08000000,               // Used For Flight Paths
            STR(Waterwalking       ),// 0x10000000,               // Prevent Unit From Falling Through Water
            STR(Safe_Fall          ),// 0x20000000,               // Active Rogue Safe Fall Spell (Passive)
            STR(Hover              ),// 0x40000000
            STR(Flag_0x80000000    ),// 0x80000000
            STR(Unk1              ),
            STR(Unk2              ),
            STR(Unk3              ),
            STR(Fullspeedturning  ),
            STR(Fullspeedpitching ),
            STR(Allow_Pitching    ),
            STR(Unk4              ),
            STR(Unk5              ),
            STR(Unk6              ),
            STR(Unk7              ),
            STR(Interp_Move       ),
            STR(Interp_Turning    ),
            STR(Interp_Pitching   ),
            STR(Unk8              ),
            STR(AllowSwimFlyTransition ),
            STR(Unk10             ),
        };

        QString str;
        print_flags(raw,g_MovementFlag_names,str);
        return str;
    }

    QString MoveSplineFlag::toString() const
    {
        const char * g_SplineFlag_names[32]=
        {
            STR(AnimBit1     ),// 0x00000001,
            STR(AnimBit2     ),// 0x00000002,
            STR(AnimBit3     ),// 0x00000004,
            STR(AnimBit4     ),// 0x00000008,
            STR(AnimBit5     ),// 0x00000010,
            STR(AnimBit6     ),// 0x00000020,
            STR(AnimBit7     ),// 0x00000040,
            STR(AnimBit8     ),// 0x00000080,
            STR(Done         ),// 0x00000100,
            STR(Falling      ),// 0x00000200,           // Not Compartible With Trajectory Movement
            STR(No_Spline    ),// 0x00000400,
            STR(Trajectory   ),// 0x00000800,           // Not Compartible With Fall Movement
            STR(Runmode      ),// 0x00001000,
            STR(Flying       ),// 0x00002000,           // Smooth Movement(Catmullrom Interpolation Mode), Flying Animation
            STR(Knockback    ),// 0x00004000,           // Model Orientation Fixed
            STR(Final_Point  ),// 0x00008000,
            STR(Final_Target ),// 0x00010000,
            STR(Final_Angle  ),// 0x00020000,
            STR(Catmullrom   ),// 0x00040000,           // Used Catmullrom Interpolation Mode
            STR(Cyclic       ),// 0x00080000,           // Movement By Cycled Spline
            STR(Enter_Cycle  ),// 0x00100000,           // Everytime Appears With Cyclic Flag In Monster Move Packet
            STR(Animation    ),// 0x00200000,           // Animationid (0...3), Uint32 Time, Not Compartible With Trajectory And Fall Movement
            STR(Unknown4     ),// 0x00400000,           // Disables Movement By Path
            STR(Unknown5     ),// 0x00800000,
            STR(Unknown6     ),// 0x01000000,
            STR(Unknown7     ),// 0x02000000,
            STR(Unknown8     ),// 0x04000000,
            STR(OrientationInversed ),// 0x08000000,           // Appears With Runmode Flag, Nodes ),// 1, Handles Orientation
            STR(Unknown10    ),// 0x10000000,
            STR(Unknown11    ),// 0x20000000,
            STR(Unknown12    ),// 0x40000000,
            STR(Unknown13    ),// 0x80000000,
        };

        QString str;
        print_flags(raw,g_SplineFlag_names,str);
        return str;
    }
}
