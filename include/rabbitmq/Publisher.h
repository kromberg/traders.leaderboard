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

namespace rabbitmq
{

class Publisher : public Handler
{
private:
    std::mutex m_transactionCommittedGuard;
    std::condition_variable m_transactionCommittedCV;
    volatile bool m_transactionCommitted = false;

protected:
    void onSuccessStartTransactionCallback();
    void onErrorStartTransactionCallback(const char* msg);

    void onSuccessCommitTransactionCallback();
    void onErrorCommitTransactionCallback(const char* msg);

public:
    Publisher(AMQP::TcpConnection* connection);
    Publisher(std::shared_ptr<AMQP::TcpChannel>& channel);
    virtual ~Publisher() = default;
    Publisher(const Publisher& c) = delete;
    Publisher(Publisher&& c) = default;
    Publisher& operator=(const Publisher& c) = delete;
    Publisher& operator=(Publisher&& c) = default;

    void waitTransactionEnd();

    AMQP::Deferred& startTransaction();
    AMQP::Deferred& commitTransaction();

    template<class... Args>
    bool publish(Args... args);
};

////////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////////
inline Publisher::Publisher(AMQP::TcpConnection* connection):
    Handler(connection)
{
    m_logger = logger::Logger::getLogCategory("RMQ_PUBLISHER");
}

inline Publisher::Publisher(std::shared_ptr<AMQP::TcpChannel>& _channel):
    Handler(_channel)
{
    m_logger = logger::Logger::getLogCategory("RMQ_PUBLISHER");
}

template<class... Args>
inline bool Publisher::publish(Args... args)
{
    return channel().publish(std::forward<Args>(args)...);
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_PUBLISHER_H