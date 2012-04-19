#pragma once

#include "typedefs.h"
#include <QtCore/QTextStream>

namespace Movement
{
    struct Component;
    class StringReader;
    class CommandMgr;
    struct CommandInvoker;

    struct ICommandHandler
    {
        virtual void Invoke(StringReader& command, CommandInvoker& invoker) = 0;

        QString Description;

        explicit ICommandHandler();

    protected:

        ~ICommandHandler() {}
    };

    class CommandMgr
    {
        friend struct ICommandHandler;
        class CommandMgrImpl* impl;
    public:

        explicit CommandMgr();
        ~CommandMgr();

        void Invoke(CommandInvoker& invoker, const char * command);

        void Register(ICommandHandler& handler, const char * aliases);
    };

    struct EXPORT CommandInvoker
    {
        public: Component& com;
        public: QTextStream output;
        private: QString string;

        public: explicit CommandInvoker(Component& invoker);
    };

    /** Parses the string.
        Note that StringReader turns string into unusable state!  */
    class StringReader
    {
        char * _string;
    public:
        enum {
            defaultSeparator = ' ',
        };

        explicit StringReader(char * str);

        bool atEnd() const {
            return *_string == '\0';
        }

        const char * constData() const {
            return _string;
        }

        float readFloat(int separator = defaultSeparator);

        int32 readInt(int separator = defaultSeparator);

        const char * readArg(int separator = defaultSeparator);
    };

    bool cmp(const char * str1, const char* str2);

    template<class ParentNode, const char* (*CommandNodeName)() > class CommandNode : public ICommandHandler
    {
        private: struct Redirector : public ParentNode
        {
            explicit Redirector() {
                ParentNode::Init(CommandNodeName());
            }

            void Invoke(StringReader& command, CommandInvoker& invoker) override {
                CommandNode::CommandMgrInstance().Invoke(invoker, command.constData());
            }
        };

        protected: void Init(const char * aliases) {
            CommandMgrInstance().Register(*this, aliases);
        }

        private: static CommandMgr& CommandMgrInstance() {
             static CommandMgr inst;
             static Redirector red;
             return inst;
        }
    };

#define DECLARE_COMMAND_NODE(CommandNodeName, CmdNameString, ParentNodeType) \
    inline const char * CommandNodeName##String() { return CmdNameString;} \
    typedef CommandNode<ParentNodeType, &CommandNodeName##String > CommandNodeName;
}
