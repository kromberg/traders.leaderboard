#ifndef MY_RABBIT_MQ_CONSUMER_H
#define MY_RABBIT_MQ_CONSUMER_H

#include <functional>
#include <memory>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Handler.h"

namespace rabbitmq
{

class Consumer : public Handler
{
protected:
    ProcessorPtr m_processor;

protected:
    void onMessageCallback(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered);

public:
    Consumer(AMQP::TcpConnection* connection);
    Consumer(std::shared_ptr<AMQP::TcpChannel>& channel);
    virtual ~Consumer() = default;
    Consumer(const Consumer& c) = delete;
    Consumer(Consumer&& c) = default;
    Consumer& operator=(const Consumer& c) = delete;
    Consumer& operator=(Consumer&& c) = default;

    void attachProcessor(ProcessorPtr& processor);

    // todo: return DeferredConsumer
    template<class... Args>
    AMQP::Deferred& consume(Args... args);
};

////////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////////
inline Consumer::Consumer(AMQP::TcpConnection* connection):
    Handler(connection)
{
    m_logger = logger::Logger::getLogCategory("RMQ_CONSUMER");
}

inline Consumer::Consumer(std::shared_ptr<AMQP::TcpChannel>& _channel):
    Handler(_channel)
{
    m_logger = logger::Logger::getLogCategory("RMQ_CONSUMER");
}

inline void Consumer::attachProcessor(ProcessorPtr& processor)
{
    atomic_store(&m_processor, processor);
}

template<class... Args>
inline AMQP::Deferred& Consumer::consume(Args... args)
{
    using namespace logger;
    using namespace std::placeholders;
    return channel().consume(std::forward<Args>(args)...)
        .onReceived(std::bind(&Consumer::onMessageCallback, this, _1, _2, _3))
        .onSuccess(std::bind(&Consumer::onStartCallback, this, _1))
        .onError(std::bind(&Consumer::onErrorCallback, this, _1))
        .onFinalize(
            std::bind(
                &Consumer::onFinalizeCallback,
                this,
                Level::INFO,
                "Consume operation was finalized"));
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_CONSUMER_H