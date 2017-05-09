#ifndef MY_RABBIT_MQ_COMMANDS_DISPATCHER_H
#define MY_RABBIT_MQ_COMMANDS_DISPATCHER_H

#include <functional>
#include <unordered_map>
#include <string>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"
#include "../logger/LoggerDefines.h"

#include "../db/Fwd.h"

#include "Fwd.h"
#include "ProcessingItem.h"

namespace rabbitmq
{
class Dispatcher
{
private:
    typedef std::function<Result(logger::CategoryPtr&, const std::string&, ProcessingItem&&)> MessageProcFunc;
    typedef std::unordered_map<std::string, MessageProcFunc> MessageProcFuncsMap;
    static const MessageProcFuncsMap m_messageProcFuncsMap;

    logger::CategoryPtr m_logger;
    db::LogicPtr m_logic;

private:
    template<class... Args>
    static Result getArguments(
        logger::CategoryPtr& logger,
        const std::string& command,
        const std::string& str,
        const char* fmt,
        Args... args);

    // user_registered(id,name)
    static Result processUserRegistered(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);
    // user_renamed(id,name)
    static Result processUserRenamed(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);
    // user_deal(id,time,amount)
    static Result processUserDeal(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);
    // user_deal_won(id,time,amount)
    static Result processUserDealWon(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);
    // user_connected(id)
    static Result processUserConnected(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);
    // user_disconnected(id)
    static Result processUserDisconnected(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item);

public:
    Dispatcher();
    ~Dispatcher() = default;
    Result processMessage(ProcessingItem&& item);
};

template<class... Args>
Result Dispatcher::getArguments(
    logger::CategoryPtr& logger,
    const std::string& command,
    const std::string& str,
    const char* fmt,
    Args... args)
{
    const size_t countOfExpectedArgs = sizeof...(Args);
    int res = sscanf(str.c_str(), fmt, args...);
    if (res != countOfExpectedArgs)
    {
        LOG_ERROR(logger, "Cannot process command '%s': invalid arguments format: "
            "actual number of arguments[%d] does not equal to expected number of arguments[%zu]",
            command.c_str(), res, countOfExpectedArgs);
        return Result::INVFMT;
    }
    return Result::SUCCESS;
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_COMMANDS_DISPATCHER_H