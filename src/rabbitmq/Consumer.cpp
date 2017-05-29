#include <libconfig.h++>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>

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

    if (!m_messageProcessingCallback)
    {
        LOG_ERROR(m_logger, "Cannot process message '%s'. Callback is not set",
            messageString.c_str());
        return ;
    }
    ProcessingItem item(
        m_channel,
        std::move(messageString),
        std::move(exchange),
        std::move(routingkey),
        deliveryTag,
        redelivered);

    Result r = m_messageProcessingCallback(std::move(item));
    if (Result::SUCCESS != r)
    {
        channel().reject(deliveryTag);
        LOG_ERROR(m_logger, "Cannot process message. Result: %d(%s)",
            static_cast<int32_t>(r), common::resultToStr(r));
        return ;
    }

    channel().ack(deliveryTag);
}

} // namespace rabbitmq