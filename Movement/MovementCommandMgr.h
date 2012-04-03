#pragma once

#include "framework/CommandMgr.h"

namespace Movement
{
    class MovementCommand : public ICommandHandler
    {
        public: static void ExecCommand(CommandInvoker& invoker) {
            CommandMgrInstance().Invoke(invoker);
        }

        private: static CommandMgr& CommandMgrInstance() {
            static CommandMgr inst;
            return inst;
        }

        protected: void Init(const char * aliases) {
            CommandMgrInstance().Register(*this, aliases);
        }
    };

    EXPORT inline void ExecCommand(CommandInvoker& invoker) {
        MovementCommand::ExecCommand(invoker);
    }
}
