#pragma once

#include "framework/CommandMgr.h"

namespace Movement
{
    class MovementCommand : public ICommandHandler
    {
        public: static void ExecCommand(CommandInvoker& invoker, const char * cmd) {
            CommandMgrInstance().Invoke(invoker, cmd);
        }

        private: static CommandMgr& CommandMgrInstance() {
            static CommandMgr inst;
            return inst;
        }

        protected: void Init(const char * aliases) {
            CommandMgrInstance().Register(*this, aliases);
        }
    };

    EXPORT inline void ExecCommand(CommandInvoker& invoker, const char * cmd) {
        MovementCommand::ExecCommand(invoker, cmd);
    }
}
