#ifndef MY_RABBIT_MQ_CONSUMER_H
#define MY_RABBIT_MQ_CONSUMER_H

#include <functional>
#include <memory>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"

namespace rabbitmq
{

class Consumer
{
private:
    logger::CategoryPtr m_logger;
    std::shared_ptr<AMQP::TcpChannel> m_channel;
    ProcessorPtr m_processor;

private:
    void onStartCallback(const std::string &consumertag);
    void onErrorCallback(const char *message);
    void onReadyCallback();
    void onSuccessCallback(const logger::Level level, const char* logMessage);
    void onFinalizeCallback(const logger::Level level, const char* logMessage);
    void onQueueSuccessCallback(const std::string &name, uint32_t messagecount, uint32_t consumercount);
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

    AMQP::TcpChannel& channel();

    template<class... Args>
    AMQP::Deferred& declareExchange(Args... args);

    template<class... Args>
    AMQP::Deferred& declareQueue(Args... args);

    // todo: return DeferredConsumer
    template<class... Args>
    AMQP::Deferred& consume(Args... args);
};

////////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////////
inline Consumer::Consumer(AMQP::TcpConnection* connection):
    m_channel(new AMQP::TcpChannel(connection))
{
    using namespace std::placeholders;

    m_logger = logger::Logger::getLogCategory("RMQ_CONSUMER");

    channel().onError(std::bind(&Consumer::onErrorCallback, this, _1));
    channel().onReady(std::bind(&Consumer::onReadyCallback, this));
}

inline Consumer::Consumer(std::shared_ptr<AMQP::TcpChannel>& _channel):
    m_channel(_channel)
{
    using namespace std::placeholders;

    if (!m_channel)
    {
        throw std::runtime_error("Cannot create consumer with empty channel");
    }

    m_logger = logger::Logger::getLogCategory("RMQ_CONSUMER");

    channel().onError(std::bind(&Consumer::onErrorCallback, this, _1));
    channel().onReady(std::bind(&Consumer::onReadyCallback, this));
}

inline void Consumer::attachProcessor(ProcessorPtr& processor)
{
    atomic_store(&m_processor, processor);
}

inline AMQP::TcpChannel& Consumer::channel()
{
    return *m_channel;
}

template<class... Args>
inline AMQP::Deferred& Consumer::declareExchange(Args... args)
{
    using namespace logger;
    return channel().declareExchange(args...)
        .onSuccess(
            std::bind(
                &Consumer::onSuccessCallback,
                this,
                Level::INFO,
                "Exchange was successfully declared"))
        .onFinalize(
            std::bind(
                &Consumer::onFinalizeCallback,
                this,
                Level::INFO,
                "Exchange was finalized"));
}

template<class... Args>
inline AMQP::Deferred& Consumer::declareQueue(Args... args)
{
    using namespace logger;
    using namespace std::placeholders;
    return channel().declareQueue(args...)
        .onSuccess(std::bind(&Consumer::onQueueSuccessCallback, this, _1, _2, _3))
        .onFinalize(
            std::bind(
                &Consumer::onFinalizeCallback,
                this,
                Level::INFO,
                "Queue was finalized"));;
}

template<class... Args>
inline AMQP::Deferred& Consumer::consume(Args... args)
{
    using namespace logger;
    using namespace std::placeholders;
    return channel().consume(args...)
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