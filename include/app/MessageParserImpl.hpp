#ifndef MY_APP_MESSAGE_PARSER_IMPL_HPP
#define MY_APP_MESSAGE_PARSER_IMPL_HPP

namespace app
{

template<class... Args>
Result MessageParser::getArguments(
    const std::string& command,
    const std::string& str,
    const char* fmt,
    Args... args)
{
    const size_t countOfExpectedArgs = sizeof...(Args);
    int res = sscanf(str.c_str(), fmt, std::forward<Args>(args)...);
    if (res != countOfExpectedArgs)
    {
        LOG_ERROR(m_logger, "Cannot process command '%s': invalid arguments format: "
            "actual number of arguments[%d] does not equal to expected number of arguments[%zu]",
            command.c_str(), res, countOfExpectedArgs);
        return Result::INVALID_FORMAT;
    }
    return Result::SUCCESS;
}

inline void MessageParser::registerOnUserRegisteredCallback(const OnUserRegisteredCallback& cb)
{
    m_onUserRegisteredCallback = cb;
}

inline void MessageParser::registerOnUserRenamedCallback(const OnUserRenamedCallback& cb)
{
    m_onUserRenamedCallback = cb;
}

inline void MessageParser::registerOnUserDealCallback(const OnUserDealCallback& cb)
{
    m_onUserDealCallback = cb;
}

inline void MessageParser::registerOnUserDealWonCallback(const OnUserDealWonCallback& cb)
{
    m_onUserDealWonCallback = cb;
}

inline void MessageParser::registerOnUserConnectedCallback(const OnUserConnectedCallback& cb)
{
    m_onUserConnectedCallback = cb;
}

inline void MessageParser::registerOnUserDisconnectedCallback(const OnUserDisconnectedCallback& cb)
{
    m_onUserDisconnectedCallback = cb;
}

template<class T>
inline void MessageParser::registerCallbackObject(T& obj)
{
    using namespace std::placeholders;

    registerOnUserRegisteredCallback(std::bind(&T::onUserRegistered, std::ref(obj), _1, _2));
    registerOnUserRenamedCallback(std::bind(&T::onUserRenamed, std::ref(obj), _1, _2));
    registerOnUserDealCallback(std::bind(&T::onUserDeal, std::ref(obj), _1, _2, _3));
    registerOnUserDealWonCallback(std::bind(&T::onUserDeal, std::ref(obj), _1, _2, _3));
    registerOnUserConnectedCallback(std::bind(&T::onUserConnected, std::ref(obj), _1));
    registerOnUserDisconnectedCallback(std::bind(&T::onUserDisconnected, std::ref(obj), _1));
}

} // namespace app

#endif // MY_APP_MESSAGE_PARSER_IMPL_HPP