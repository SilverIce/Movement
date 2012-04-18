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

        void Invoke(CommandInvoker& invoker);

        void Register(ICommandHandler& handler, const char * aliases);
    };

    struct CommandInvoker
    {
        Component& com;
        QString string;
        QTextStream output;
        const char* command;

        explicit CommandInvoker(Component& invoker, const char* cmd) : com(invoker), command(cmd) {
            output.setString(&string, QIODevice::WriteOnly);
        }
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

        float readFloat(int separator = defaultSeparator);

        int32 readInt(int separator = defaultSeparator);

        const char * readArg(int separator = defaultSeparator);
    };

    bool cmp(const char * str1, const char* str2);
}
