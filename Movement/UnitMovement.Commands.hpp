namespace Movement
{
    struct ApplyModeCommand : public MovementCommand {
        const char* m_name;
        const char* m_modeName;
        MoveMode m_mode;
        explicit ApplyModeCommand(MoveMode mode, const char* modeName, const char* aliases) {
            m_mode = mode;
            m_modeName = modeName;
            (Description = "Enables or disables ") += modeName;
            MovementCommand::Init(aliases);
        }
        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            bool apply = cmp(command.readArg(), "on");
            invoker.com.as<UnitMovement>().ApplyMoveMode(m_mode, apply);
            invoker.output << endl << "Movement mode " << m_modeName << (apply ? " enabled" : " disabled");
        }
    };

#define DECL_APPLYMODE_COMMAND(Mode, CommandString) DELAYED_INIT(ApplyModeCommand, ApplyModeCommand_##Mode, Mode, #Mode, CommandString);

    DECL_APPLYMODE_COMMAND(MoveModeAllowFly,"canfly");
    DECL_APPLYMODE_COMMAND(MoveModeHover,  "hover");
    DECL_APPLYMODE_COMMAND(MoveModeAllowSlowfall,  "slowfall");
    DECL_APPLYMODE_COMMAND(MoveModeRoot,  "root");
    DECL_APPLYMODE_COMMAND(MoveModeSwim, "swim");
    DECL_APPLYMODE_COMMAND(MoveModeWaterwalk,  "waterwalk");
    DECL_APPLYMODE_COMMAND(MoveModeWalk,  "walk");
    DECL_APPLYMODE_COMMAND(MoveModeGravityDisabled, "gravity");
    DECL_APPLYMODE_COMMAND(MoveModeAllowSwimFlyTransition, "swimfly");

#undef DECL_APPLYMODE_COMMAND

    struct ModifyCollisionHeightCommand : public MovementCommand
    {
        explicit ModifyCollisionHeightCommand() {
            Init("SetCollissionHeight|SetCollHght");
            Description = "Modifies collision box height.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            float value = command.readFloat();
            invoker.com.as<UnitMovement>().SetCollisionHeight(value);
            invoker.output << endl << "Collision box height is " << value;
        }
    };
    DELAYED_INIT(ModifyCollisionHeightCommand, ModifyCollisionHeightCommand);

    struct TestCommand : public MovementCommand
    {
        explicit TestCommand() {
            Init("Test");
            Description = "Launches a test.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            UnitMovementImpl& target = invoker.com.as<UnitMovement>().Impl();
            target.ApplyMoveFlag(UnitMoveFlag::Falling|UnitMoveFlag::Hover, true);
            PacketBuilder::Send_HeartBeat(target);
        }
    };
    DELAYED_INIT(TestCommand, TestCommand);

    struct PrintTransportInfoCommand : public MovementCommand
    {
        explicit PrintTransportInfoCommand() {
            Init("transport");
            Description = "Prints transport info.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            if (MovingEntity_Revolvable2* target = invoker.com.as<MovingEntity_WOW>().Environment())
                invoker.output << endl << "Transport info: " << target->toStringAll();
            else
                invoker.output << endl << "Invoker is not boarded";
        }
    };
    DELAYED_INIT(PrintTransportInfoCommand, PrintTransportInfoCommand);
}
