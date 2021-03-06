#include <libconfig.h++>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

namespace rabbitmq
{

Result Publisher::startTransactionSync()
{
    m_transactionMessagesCount = 0;

    SyncObj<Result> result(Result::FAILED, 1, 5);
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
    SyncObj<Result> result(Result::FAILED, 1, 5);
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

Result Publisher::rollbackTransactionSync()
{
    SyncObj<Result> result(Result::FAILED, 1, 5);
    channel().rollbackTransaction()
        .onSuccess([&] () -> void
            {
                LOG_DEBUG(m_logger, "Transaction was rollbacked.");
                result.set(Result::SUCCESS);
            })
        .onError([&] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot rollback transaction. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

} // namespace rabbitmq