#ifndef MY_RABBIT_MQ_PUBLISHER_H
#define MY_RABBIT_MQ_PUBLISHER_H

#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Handler.h"
#include "Utils.h"

namespace rabbitmq
{

class Publisher : public Handler
{
private:
    SynchObj m_transactionStarted;
    SynchObj m_transactionCommitted;
    size_t m_transactionMessagesCount = 0;

protected:
    void onSuccessStartTransactionCallback();
    void onErrorStartTransactionCallback(const char* msg);

    void onSuccessCommitTransactionCallback(const size_t transactionMessagesCount);
    void onErrorCommitTransactionCallback(const char* msg);

public:
    Publisher(EventLoop& loop, const AMQP::Address &address);
    virtual ~Publisher() = default;
    Publisher(const Publisher& c) = delete;
    Publisher(Publisher&& c) = default;
    Publisher& operator=(const Publisher& c) = delete;
    Publisher& operator=(Publisher&& c) = default;

    AMQP::Deferred& startTransaction();
    AMQP::Deferred& commitTransaction();

    void startTransactionSync();
    void commitTransactionSync();

    size_t transactionMessagesCount() const;

    template<class... Args>
    bool publish(Args... args);
};

////////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////////
inline Publisher::Publisher(EventLoop& loop, const AMQP::Address &address):
    Handler(loop, address)
{
    m_logger = logger::Logger::getLogCategory("RMQ_PUBLISHER");
}

inline size_t Publisher::transactionMessagesCount() const
{
    return m_transactionMessagesCount;
}

template<class... Args>
inline bool Publisher::publish(Args... args)
{
    ++ m_transactionMessagesCount;
    return channel().publish(std::forward<Args>(args)...);
}


} // namespace rabbitmq

#endif // MY_RABBIT_MQ_PUBLISHER_H