#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Processor.h>

namespace rabbitmq
{

void Consumer::onMessageCallback(
    const AMQP::Message &message, uint64_t deliveryTag, bool redelivered)
{
    std::string messageString(message.body(), message.bodySize());
    std::string exchange(message.exchange());
    std::string routingkey(message.routingkey());

    LOG_DEBUG(m_logger, "Message received: <Body: %s. Exchange: %s. Routing key :%s."
        "Delivery tag: %lu. Redelivered: %s>",
        messageString.c_str(), exchange.c_str(), routingkey.c_str(),
        deliveryTag, (redelivered ? "true" : "false"));
    if (!m_processor)
    {
        LOG_ERROR(m_logger, "Cannot process message '%s' since no processor is attached",
            messageString.c_str());
        return ;
    }

    Result r = m_processor->pushMessage(
        m_channel,
        std::move(messageString),
        std::move(exchange),
        std::move(routingkey),
        deliveryTag,
        redelivered);
    if (Result::SUCCESS != r)
    {
        LOG_ERROR(m_logger, "Cannot push message '%s' to processor. Result: %u(%s)",
            messageString.c_str());
        return ;
    }
}

} // namespace rabbitmq