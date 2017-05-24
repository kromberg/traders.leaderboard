#ifndef MY_APP_CONFIGURATION_H
#define MY_APP_CONFIGURATION_H

#include <string>

namespace app
{

struct RmqHandlerCfg
{
    std::string m_exchangeName;
    std::string m_routingKey;

    RmqHandlerCfg()
    {}

    template<class T1, class T2>
    RmqHandlerCfg(T1&& exchangeName, T2& routingKey):
        m_exchangeName(std::forward<T1>(exchangeName)),
        m_routingKey(std::forward<T2>(routingKey))
    {}
};

struct RmqConsumerCfg : public RmqHandlerCfg
{
    std::string m_queueName;

    template<class T1, class T2, class T3>
    RmqConsumerCfg(T1&& exchangeName, T2& routingKey, T3&& queueName):
        RmqHandlerCfg(std::forward<T1>(exchangeName), std::forward<T2>(routingKey)),
        m_queueName(std::forward<T3>(queueName))
    {}
};

} // namespace app

#endif // MY_APP_CONFIGURATION_H