#include <libconfig.h++>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

namespace rabbitmq
{

Result Publisher::customConfigure(libconfig::Config& cfg)
{
    using namespace libconfig;

    m_exchangeName = "leaderboard";
    m_queueName = "leaderboard";
    m_routingKey = "routing-key";
    try
    {
        Setting& setting = cfg.lookup("rabbitmq.publisher");
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

Result Publisher::startTransactionSync()
{
    m_transactionMessagesCount = 0;

    SyncObj<Result> result;
    channel().startTransaction()
        .onSuccess([&] () -> void
            {
                LOG_DEBUG(m_logger, "Transaction was started");
                result.set(Result::SUCCESS);
            })
        .onError([&] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot start transaction. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

Result Publisher::commitTransactionSync()
{
    SyncObj<Result> result;
    channel().commitTransaction()
        .onSuccess([&] () -> void
            {
                LOG_DEBUG(m_logger, "Transaction was committed. %zu messages were published",
                    m_transactionMessagesCount);
                result.set(Result::SUCCESS);
            })
        .onError([&] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot commit transaction. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

} // namespace rabbitmq