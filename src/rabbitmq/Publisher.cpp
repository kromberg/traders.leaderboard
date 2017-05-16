#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

namespace rabbitmq
{

AMQP::Deferred& Publisher::startTransaction()
{
    using namespace logger;
    using namespace std::placeholders;

    m_transactionMessagesCount = 0;

    m_transactionStarted.reset();

    return channel().startTransaction()
        .onSuccess(std::bind(&Publisher::onSuccessStartTransactionCallback, this))
        .onError(std::bind(&Publisher::onErrorStartTransactionCallback, this, _1))
        .onFinalize(
            std::bind(
                &Publisher::onFinalizeCallback,
                this,
                Level::DEBUG,
                "Start Transaction operation was finalized"));
}

void Publisher::startTransactionSync()
{
    startTransaction();
    m_transactionStarted.wait();
}

AMQP::Deferred& Publisher::commitTransaction()
{
    using namespace logger;
    using namespace std::placeholders;

    m_transactionCommitted.reset();

    return channel().commitTransaction()
        .onSuccess(std::bind(&Publisher::onSuccessCommitTransactionCallback, this, m_transactionMessagesCount))
        .onError(std::bind(&Publisher::onErrorCommitTransactionCallback, this, _1))
        .onFinalize(
            std::bind(
                &Publisher::onFinalizeCallback,
                this,
                Level::DEBUG,
                "Commit Transaction operation was finalized"));
}

void Publisher::commitTransactionSync()
{
    commitTransaction();
    m_transactionCommitted.wait();
}

void Publisher::onSuccessStartTransactionCallback()
{
    LOG_DEBUG(m_logger, "Transaction was started");
    m_transactionStarted.set();
}

void Publisher::onErrorStartTransactionCallback(const char* msg)
{
    LOG_ERROR(m_logger, "Error occurred while starting transaction. Description: %s",
        msg);
    m_transactionStarted.set();
}

void Publisher::onSuccessCommitTransactionCallback(const size_t transactionMessagesCount)
{
    LOG_DEBUG(m_logger, "Transaction was committed. %zu messages were published",
        transactionMessagesCount);
    m_transactionCommitted.set();
}

void Publisher::onErrorCommitTransactionCallback(const char* msg)
{
    LOG_ERROR(m_logger, "Error occurred while committing transaction. Description: %s",
        msg);
    m_transactionCommitted.set();
}

} // namespace rabbitmq