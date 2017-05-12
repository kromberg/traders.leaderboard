#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Handler.h>

namespace rabbitmq
{

void Handler::onStartCallback(const std::string &consumertag)
{
    LOG_DEBUG(m_logger, "Consume operation started. Consumer tag: %s",
        consumertag.c_str());
}

void Handler::onErrorCallback(const char *message)
{
    LOG_ERROR(m_logger, "Consumer error. Message: %s", message);
}

void Handler::onReadyCallback()
{
    LOG_INFO(m_logger, "Channel is ready");
}

void Handler::onSuccessCallback(const logger::Level level, const char* logMessage)
{
    LOG(m_logger, level, "%s", logMessage);
}

void Handler::onFinalizeCallback(const logger::Level level, const char* logMessage)
{
    LOG(m_logger, level, "%s", logMessage);
}

void Handler::onQueueSuccessCallback(
    const std::string &name, uint32_t messagecount, uint32_t consumercount)
{
    LOG_INFO(m_logger, "Queue declared %s. Messages count: %u. Consumers count: %u",
        name.c_str(), messagecount, consumercount);
}

} // namespace rabbitmq