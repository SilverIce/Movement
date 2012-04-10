//#define QT_NO_CAST_FROM_ASCII

#include "CommandMgr.h"
#include "DelayInit.h"
#include "gtest.h"
#include "Component.h"

#include "typedefs_p.h"

#include <typeinfo>
#include <hash_map>
#include <string>
#include <vector>

namespace Movement
{
    using namespace std;

    struct CommandMgrException {
        const char* msg;
        explicit CommandMgrException(const char* message) : msg(message) {}
    };

    class CommandMgrImpl : private ICommandHandler
    {
        typedef stdext::hash_map<string, ICommandHandler* > base;
        base m_commands;

        void Invoke(StringReader& command, CommandInvoker& invoker) override
        {
            stdext::hash_map<ICommandHandler*, vector<const string*> > com2name;
            for (base::const_iterator it = m_commands.begin(); it!= m_commands.end(); ++it)
                com2name[it->second].push_back(&it->first);
            if (command.atEnd()) {
                invoker.output << endl << "Command list:";
                for (stdext::hash_map<ICommandHandler*, vector<const string*> >::const_iterator it = com2name.begin();
                    it != com2name.end(); ++it)
                    describeCommand(*it->first, it->second, invoker.output);
            }
            else {
                const char * com = command.readArg();
                if (ICommandHandler* hdl = getHandler(com))
                    describeCommand(*hdl, com2name[hdl], invoker.output);
                else
                    invoker.output << endl << "Command '" << com << "' is not registered.";
            }
        }

        static void describeCommand(const ICommandHandler& hdl, const vector<const string*>& aliases, ostringstream& output)
        {
            output << endl << "'" << *aliases.at(0) << "' description - " << hdl.Description;
            //output.setFieldWidth(output.fieldAlignment() + 5);
            //output.setFieldAlignment(QTextStream::AlignLeft);
            output << endl << "    Handler - " << typeid(hdl).name() << '.';
            if (aliases.size() > 1) {
                output << endl << "    Aliases: ";
                for (uint32 idx = 0; idx < aliases.size(); ++idx)
                    output << *aliases.at(idx) << ((idx < aliases.size()-1) ? ", " : ".");
            }
            //output.setFieldWidth(output.fieldAlignment() - 5);
        }

    public:

        CommandMgrImpl() {
            ICommandHandler::Description = "Lists all available commands or prints a description of user specified command.";
            Register(*this, "help|?");
        }

        ICommandHandler* getHandler(const char * command) const {
            assert_state(command);
            base::const_iterator it = m_commands.find(command);
            return it == m_commands.end() ? nullptr : it->second;
        }

        void Register(ICommandHandler& handler, const char * aliases)
        {
            try {
                if (!aliases)
                    throw CommandMgrException("'aliases' is null");
                char text[1024];
                if (!(strlen(aliases) < CountOf(text)))
                    throw CommandMgrException("'aliases' length is more that 1023");
                strcpy(text, aliases);
                StringReader reader(text);
                while(!reader.atEnd()) {
                    const char * command = reader.readArg('|');
                    if (getHandler(command))
                        throw CommandMgrException("command handler with such name already registered");
                    m_commands.insert(base::value_type(command, &handler));
                }
            }
            catch (const CommandMgrException& exc) {
                log_function("can't initialize '%s' command handler - initialization failed with exception: %s",
                    typeid(handler).name(), exc.msg);
            }
        }
    };

    CommandMgr::CommandMgr() : impl(nullptr) {
        impl = new CommandMgrImpl();
    }

    CommandMgr::~CommandMgr() {
        delete impl;
        impl = nullptr;
    }

    ICommandHandler::ICommandHandler()
    {
        Description = "No description specified.";
    }

    void CommandMgr::Register(ICommandHandler& handler, const char * aliases)
    {
        impl->Register(handler, aliases);
    }

    void CommandMgr::Invoke(CommandInvoker& invoker)
    {
        assert_state(invoker.command);
        try {
            char text[4096]; {
                int commandLen = strlen(invoker.command);
                if (commandLen >= CountOf(text))
                    throw CommandMgrException("command string length is more than 4095 symbols");
                strcpy(text, invoker.command);
            }

            StringReader rd(text);
            const char * sub0 = rd.readArg();

            if (ICommandHandler* hdl = impl->getHandler(sub0))
                hdl->Invoke(rd, invoker);
            else
                throw CommandMgrException("unknown command");
        }
        catch (const CommandMgrException& exc) {
            log_function("can't parse '%s' command - parsing failed with exception: %s", invoker.command, exc.msg);
            invoker.output << endl << "can't parse '" << invoker.command << "' command - parsing failed with exception: " << exc.msg;
        }
    }

    bool cmp(const char * str1, const char* str2) {
        return str1 && str2 && (strcmp(str1,str2) == 0);
    }

    float StringReader::readFloat(int separator /*= defaultSeparator*/) {
        return (float)atof(readArg(separator));
    }

    int32 StringReader::readInt(int separator /*= defaultSeparator*/) {
        return atoi(readArg(separator));
    }

    const char * StringReader::readArg(int separator /*= defaultSeparator*/)
    {
        // move pointer to last occurence of 'separator'
        while(*_string && *_string == separator) {
            //*_string = '\0';
            ++_string;
        }

        if (atEnd()) // points to end of the string, nothing to read
            throw CommandMgrException(__FUNCTION__ " can't read more");

        const char * arg = _string;

        // move pointer to next first occurence of 'separator'
        while(*_string) {
            if (*_string == separator) {
                *_string = '\0'; // cut string
                if (*(1+_string) != '\0')
                    ++_string;
                break;
            }
            ++_string;
        }

        return arg;
    }

    StringReader::StringReader(char * str)
    {
        assert_state(str);
        _string = str;
    }
}

namespace Movement
{
    TEST(CommandMgr, StringReader)
    {
        {
            char text[] = "  eat  me ! ";
            StringReader rd(text);
            const char * arg = rd.readArg();
            EXPECT_TRUE( cmp(arg, "eat") );
            arg = rd.readArg();
            EXPECT_TRUE( cmp(arg, "me") );
            arg = rd.readArg();
            EXPECT_TRUE( cmp(arg, "!") );
            EXPECT_TRUE( rd.atEnd() );
            EXPECT_THROW( rd.readArg(), CommandMgrException );
        }
        {
            char text[] = "canfly|";
            StringReader rd(text);
            EXPECT_TRUE( cmp(rd.readArg('|'), "canfly") );
            EXPECT_TRUE( rd.atEnd() );
        }
    }

    TEST(CommandMgr, help_command)
    {
        Component com;
        CommandInvoker invoker(com, "help");

        CommandMgr mgr;
        mgr.Invoke(invoker);
        log_console("output is = %s", invoker.output.str().c_str());
    }
}
