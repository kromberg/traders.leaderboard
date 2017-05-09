#include <memory>
#include <db/DBInMemoryLogic.h>
#include <rabbitmq/CommandsDispatcher.h>

namespace rabbitmq
{

const Dispatcher::MessageProcFuncsMap Dispatcher::m_messageProcFuncsMap = 
{
    { "user_registered"     , &Dispatcher::processUserRegistered     },
    { "user_renamed"        , &Dispatcher::processUserRenamed        },
    { "user_deal"           , &Dispatcher::processUserDeal           },
    { "user_deal_won"       , &Dispatcher::processUserDealWon        },
    { "user_connected"      , &Dispatcher::processUserConnected      },
    { "user_disconnected"   , &Dispatcher::processUserDisconnected   },
};

Dispatcher::Dispatcher()
{
    m_logic.reset(new db::InMemoryLogic());
    m_logger = logger::Logger::getLogCategory("RMQ_DISPATCHER");
}

// user_registered(id,name)
Result Dispatcher::processUserRegistered(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    uint64_t id;
    std::unique_ptr<char[]> buf(new char[item.m_args.size()]);
    Result r = getArguments(dispatcher.m_logger, command, item.m_args, "(%lu,%[^)]s", std::ref(id), buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(dispatcher.m_logger, "'%s' command arguments: <id: %lu, name: %s>",
        command.c_str(), id, buf.get());

    // todo:
    db::Result dbRes = m_logic->onUserRegistered(id, buf);
    

    return Result::SUCCESS;
}

// user_renamed(id,name)
Result Dispatcher::processUserRenamed(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    uint64_t id;
    std::unique_ptr<char[]> buf(new char[item.m_args.size()]);
    Result r = getArguments(dispatcher.m_logger, command, item.m_args, "(%lu,%[^)]s", std::ref(id), buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(dispatcher.m_logger, "'%s' command arguments: <id: %lu, name: %s>",
        command.c_str(), id, buf.get());

    // todo:

    return Result::SUCCESS;
}

// user_deal(id,time,amount)
Result Dispatcher::processUserDeal(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    return Result::SUCCESS;
}
// user_deal_won(id,time,amount)
Result Dispatcher::processUserDealWon(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    return Result::SUCCESS;
}
// user_connected(id)
Result Dispatcher::processUserConnected(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    uint64_t id;
    Result r = getArguments(dispatcher.m_logger, command, item.m_args, "(%lu)", std::ref(id));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(dispatcher.m_logger, "'%s' command arguments: <id: %lu>", command.c_str(), id);

    // todo:

    return Result::SUCCESS;
}
// user_disconnected(id)
Result Dispatcher::processUserDisconnected(Dispatcher& dispatcher, const std::string& command, ProcessingItem&& item)
{
    uint64_t id;
    Result r = getArguments(dispatcher.m_logger, command, item.m_args, "(%lu)", std::ref(id));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(dispatcher.m_logger, "'%s' command arguments: <id: %lu>", command.c_str(), id);

    // todo:

    return Result::SUCCESS;
}

Result Dispatcher::processMessage(ProcessingItem&& item)
{
    if (*item.m_message.rbegin() != ')')
    {
        LOG_ERROR(m_logger, "Cannot process command: invalid format");
        return Result::INVFMT;
    }

    size_t pos = item.m_message.find('(');
    if (std::string::npos == pos)
    {
        LOG_ERROR(m_logger, "Cannot process command: invalid format");
        return Result::INVFMT;
    }

    std::string command(item.m_message, 0, pos);
    auto commandIt = m_messageProcFuncsMap.find(command);
    if (m_messageProcFuncsMap.end() == commandIt)
    {
        LOG_ERROR(m_logger, "Cannot process command '%s': dispatcher is not found",
            command.c_str());
        return Result::CMD_NOT_SUPPORTED;
    }

    item.m_args = std::string(item.m_message, pos);
    LOG_DEBUG(dispatcher.m_logger, "Start processing '%s' command. Arguments: %s",
        command.c_str(), item.m_args.c_str());

    Result r = commandIt->second(dispatcher, command, std::move(item));
    if (Result::SUCCESS != r)
    {
        LOG_ERROR(m_logger, "Cannot process command '%s'. Result: %u(%s)",
            command.c_str(), static_cast<uint16_t>(r), resultToStr(r));
        return r;
    }

    return Result::SUCCESS;
}

} // namespace rabbitmq