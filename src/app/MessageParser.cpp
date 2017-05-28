#include <memory>

#include <common/Utils.h>
#include <app/MessageParser.h>

namespace app
{

const MessageParser::MessageProcFuncsMap MessageParser::m_messageProcFuncsMap = 
{
    { "user_registered"     , &MessageParser::processUserRegistered     },
    { "user_renamed"        , &MessageParser::processUserRenamed        },
    { "user_deal"           , &MessageParser::processUserDeal           },
    { "user_deal_won"       , &MessageParser::processUserDealWon        },
    { "user_connected"      , &MessageParser::processUserConnected      },
    { "user_disconnected"   , &MessageParser::processUserDisconnected   },
};

MessageParser::MessageParser()
{
    m_logger = logger::Logger::getLogCategory("APP_MSG_PARSER");
}

MessageParser::~MessageParser()
{
}

// user_registered(id,name)
Result MessageParser::processUserRegistered(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserRegistered(command, std::move(item));
}
// user_registered(id,name)
Result MessageParser::onUserRegistered(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserRegisteredCallback)
    {
        return Result::CB_NOT_FOUND;
    }
    int64_t id;
    std::unique_ptr<char[]> buf(new char[item.m_args.size()]);
    Result r = getArguments(command, item.m_args, "(%ld,%[^)]", std::ref(id), buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(m_logger, "'%s' command arguments: <id: %ld, name: %s>",
        command.c_str(), id, buf.get());

    r = m_onUserRegisteredCallback(id, buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}

// user_renamed(id,name)
Result MessageParser::processUserRenamed(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserRenamed(command, std::move(item));
}

// user_renamed(id,name)
Result MessageParser::onUserRenamed(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserRenamedCallback)
    {
        return Result::CB_NOT_FOUND;
    }

    int64_t id;
    std::unique_ptr<char[]> buf(new char[item.m_args.size()]);
    Result r = getArguments(command, item.m_args, "(%ld,%[^)]", std::ref(id), buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(m_logger, "'%s' command arguments: <id: %ld, name: %s>",
        command.c_str(), id, buf.get());

    r = m_onUserRenamedCallback(id, buf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}

// user_deal(id,time,amount)
Result MessageParser::processUserDeal(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserDeal(command, std::move(item));
}

// user_deal(id,time,amount)
Result MessageParser::onUserDeal(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserDealCallback)
    {
        return Result::CB_NOT_FOUND;
    }

    int64_t id;
    std::unique_ptr<char[]> timeBuf(new char[item.m_args.size()]);
    int64_t amount;
    Result r = getArguments(command, item.m_args, "(%ld,%[^','],%ld)", std::ref(id), timeBuf.get(), std::ref(amount));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    time_t t;
    r = common::timeFromString(t, timeBuf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    r = m_onUserDealCallback(id, t, amount);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}

// user_deal_won(id,time,amount)
Result MessageParser::processUserDealWon(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserDealWon(command, std::move(item));
}

// user_deal_won(id,time,amount)
Result MessageParser::onUserDealWon(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserDealWonCallback)
    {
        return Result::CB_NOT_FOUND;
    }

    int64_t id;
    std::unique_ptr<char[]> timeBuf(new char[item.m_args.size()]);
    int64_t amount;
    Result r = getArguments(command, item.m_args, "(%ld,%[^','],%ld)", std::ref(id), timeBuf.get(), std::ref(amount));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    time_t t;
    r = common::timeFromString(t, timeBuf.get());
    if (Result::SUCCESS != r)
    {
        return r;
    }

    r = m_onUserDealWonCallback(id, t, amount);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}


// user_connected(id)
Result MessageParser::processUserConnected(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserConnected(command, std::move(item));
}

// user_connected(id)
Result MessageParser::onUserConnected(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserConnectedCallback)
    {
        return Result::CB_NOT_FOUND;
    }

    int64_t id;
    Result r = getArguments(command, item.m_args, "(%ld)", std::ref(id));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(m_logger, "'%s' command arguments: <id: %ld>", command.c_str(), id);

    r = m_onUserConnectedCallback(id);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}

// user_disconnected(id)
Result MessageParser::processUserDisconnected(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item)
{
    return parser.onUserDisconnected(command, std::move(item));
}

// user_disconnected(id)
Result MessageParser::onUserDisconnected(const std::string& command, rabbitmq::ProcessingItem&& item)
{
    if (!m_onUserDisconnectedCallback)
    {
        return Result::CB_NOT_FOUND;
    }

    int64_t id;
    Result r = getArguments(command, item.m_args, "(%ld)", std::ref(id));
    if (Result::SUCCESS != r)
    {
        return r;
    }

    LOG_DEBUG(m_logger, "'%s' command arguments: <id: %ld>", command.c_str(), id);

    r = m_onUserDisconnectedCallback(id);
    if (Result::SUCCESS != r)
    {
        return r;
    }

    return Result::SUCCESS;
}

Result MessageParser::parseMessage(rabbitmq::ProcessingItem&& item)
{
    if (*item.m_message.rbegin() != ')')
    {
        LOG_ERROR(m_logger, "Cannot process command: invalid format");
        return Result::INVALID_FORMAT;
    }

    size_t pos = item.m_message.find('(');
    if (std::string::npos == pos)
    {
        LOG_ERROR(m_logger, "Cannot process command: invalid format");
        return Result::INVALID_FORMAT;
    }

    std::string command(item.m_message, 0, pos);
    auto commandIt = m_messageProcFuncsMap.find(command);
    if (m_messageProcFuncsMap.end() == commandIt)
    {
        LOG_ERROR(m_logger, "Cannot process command '%s': parser is not found",
            command.c_str());
        return Result::CMD_NOT_SUPPORTED;
    }

    item.m_args = std::string(item.m_message, pos);
    LOG_DEBUG(m_logger, "Start processing '%s' command. Arguments: %s",
        command.c_str(), item.m_args.c_str());

    Result r = commandIt->second(*this, command, std::move(item));
    if (Result::SUCCESS != r)
    {
        LOG_ERROR(m_logger, "Cannot process command '%s'. Result: %u(%s)",
            command.c_str(), static_cast<uint16_t>(r), resultToStr(r));
        return r;
    }

    return Result::SUCCESS;
}

} // namespace app