#include <libconfig.h++>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>

namespace rabbitmq
{

Result Consumer::customConfigure(libconfig::Config& cfg)
{
    using namespace libconfig;

    m_exchangeName = "leaderboard";
    m_queueName = "users-events-queue";
    m_routingKey = "routing-key";
    try
    {
        Setting& setting = cfg.lookup("rabbitmq.consumer");
        Result res = readRabbitMqParameters(setting);
        if (Result::SUCCESS != res)
        {
            return res;
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find 'rabbitmq' section in configuration. Default values will be used");
    }
    LOG_INFO(m_logger, "Configuration parameters: <exchange: %s, queue: %s, key: %s>",
        m_exchangeName.c_str(), m_queueName.c_str(), m_routingKey.c_str());

    return Result::SUCCESS;
}

Result Consumer::customStart()
{
    Result res = setQosSync(1);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot set QOS");
        return res;
    }

    consume(m_queueName);

    return Result::SUCCESS;
}

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
        LOG_ERROR(m_logger, "Cannot process message '%s' to processor. Result: %u(%s)",
            messageString.c_str());
        return ;
    }

    channel().ack(deliveryTag);
}

} // namespace rabbitmq