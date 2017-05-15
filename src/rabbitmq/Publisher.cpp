#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

namespace rabbitmq
{

AMQP::Deferred& Publisher::startTransaction()
{
    using namespace logger;
    using namespace std::placeholders;

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

AMQP::Deferred& Publisher::commitTransaction()
{
    using namespace logger;
    using namespace std::placeholders;

    m_transactionCommitted = false;

    return channel().commitTransaction()
        .onSuccess(std::bind(&Publisher::onSuccessCommitTransactionCallback, this))
        .onError(std::bind(&Publisher::onErrorCommitTransactionCallback, this, _1))
        .onFinalize(
            std::bind(
                &Publisher::onFinalizeCallback,
                this,
                Level::DEBUG,
                "Commit Transaction operation was finalized"));
}

void Publisher::onSuccessStartTransactionCallback()
{
    LOG_DEBUG(m_logger, "Transaction was started");
}

void Publisher::onErrorStartTransactionCallback(const char* msg)
{
    LOG_ERROR(m_logger, "Error occurred while starting transaction. Description: %s",
        msg);
}

void Publisher::onSuccessCommitTransactionCallback()
{
    LOG_DEBUG(m_logger, "Transaction was committed. All messages were published");

    std::unique_lock<std::mutex> l(m_transactionCommittedGuard);
    m_transactionCommitted = true;
    m_transactionCommittedCV.notify_one();
}

void Publisher::onErrorCommitTransactionCallback(const char* msg)
{
    LOG_ERROR(m_logger, "Error occurred while committing transaction. Description: %s",
        msg);

    std::unique_lock<std::mutex> l(m_transactionCommittedGuard);
    m_transactionCommitted = true;
    m_transactionCommittedCV.notify_one();
}

void Publisher::waitTransactionEnd()
{
    std::unique_lock<std::mutex> l(m_transactionCommittedGuard);
    while (!m_transactionCommitted)
    {
        m_transactionCommittedCV.wait(l);
    }
}

} // namespace rabbitmq