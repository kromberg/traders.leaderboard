#ifndef MY_APP_MESSAGE_PARSER_H
#define MY_APP_MESSAGE_PARSER_H

#include <functional>
#include <unordered_map>
#include <string>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"
#include "../logger/LoggerDefines.h"

#include "../common/Types.h"

#include "../rabbitmq/ProcessingItem.h"

namespace app
{
using common::Result;

class MessageParser
{
private:
    typedef std::function<Result(const int64_t, const std::string&)> OnUserRegisteredCallback;
    typedef std::function<Result(const int64_t, const std::string&)> OnUserRenamedCallback;
    typedef std::function<Result(const int64_t, const std::time_t, const int64_t)> OnUserDealCallback;
    typedef std::function<Result(const int64_t, const std::time_t, const int64_t)> OnUserDealWonCallback;
    typedef std::function<Result(const int64_t)> OnUserConnectedCallback;
    typedef std::function<Result(const int64_t)> OnUserDisconnectedCallback;

    typedef std::function<Result(MessageParser&, const std::string&, rabbitmq::ProcessingItem&&)> MessageProcFunc;
    typedef std::unordered_map<std::string, MessageProcFunc> MessageProcFuncsMap;

private:
    OnUserRegisteredCallback m_onUserRegisteredCallback;
    OnUserRenamedCallback m_onUserRenamedCallback;
    OnUserDealCallback m_onUserDealCallback;
    OnUserDealWonCallback m_onUserDealWonCallback;
    OnUserConnectedCallback m_onUserConnectedCallback;
    OnUserDisconnectedCallback m_onUserDisconnectedCallback;

    static const MessageProcFuncsMap m_messageProcFuncsMap;

    logger::CategoryPtr m_logger;

private:
    template<class... Args>
    Result getArguments(
        const std::string& command,
        const std::string& str,
        const char* fmt,
        Args... args);

    // user_registered(id,name)
    static Result processUserRegistered(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_renamed(id,name)
    static Result processUserRenamed(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_deal(id,time,amount)
    static Result processUserDeal(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_deal_won(id,time,amount)
    static Result processUserDealWon(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_connected(id)
    static Result processUserConnected(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_disconnected(id)
    static Result processUserDisconnected(MessageParser& parser, const std::string& command, rabbitmq::ProcessingItem&& item);

    // user_registered(id,name)
    virtual Result onUserRegistered(const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_renamed(id,name)
    virtual Result onUserRenamed(const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_deal(id,time,amount)
    virtual Result onUserDeal(const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_deal_won(id,time,amount)
    virtual Result onUserDealWon(const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_connected(id)
    virtual Result onUserConnected(const std::string& command, rabbitmq::ProcessingItem&& item);
    // user_disconnected(id)
    virtual Result onUserDisconnected(const std::string& command, rabbitmq::ProcessingItem&& item);

public:
    MessageParser();
    ~MessageParser();
    Result parseMessage(rabbitmq::ProcessingItem&& item);

    void registerOnUserRegisteredCallback(const OnUserRegisteredCallback& cb);
    void registerOnUserRenamedCallback(const OnUserRenamedCallback& cb);
    void registerOnUserDealCallback(const OnUserDealCallback& cb);
    void registerOnUserDealWonCallback(const OnUserDealWonCallback& cb);
    void registerOnUserConnectedCallback(const OnUserConnectedCallback& cb);
    void registerOnUserDisconnectedCallback(const OnUserDisconnectedCallback& cb);
    template<class T>
    void registerCallbackObject(T& obj);
};

} // namespace app

#include "MessageParserImpl.hpp"

#endif // MY_APP_MESSAGE_PARSER_H