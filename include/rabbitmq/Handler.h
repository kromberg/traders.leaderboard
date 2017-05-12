#ifndef MY_RABBIT_MQ_HANDLER_H
#define MY_RABBIT_MQ_HANDLER_H

#include <functional>
#include <memory>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"

namespace rabbitmq
{

class Handler
{
protected:
    logger::CategoryPtr m_logger;
    std::shared_ptr<AMQP::TcpChannel> m_channel;

protected:
    virtual void onStartCallback(const std::string &Handlertag);
    virtual void onErrorCallback(const char *message);
    virtual void onReadyCallback();
    virtual void onSuccessCallback(const logger::Level level, const char* logMessage);
    virtual void onFinalizeCallback(const logger::Level level, const char* logMessage);
    virtual void onQueueSuccessCallback(const std::string &name, uint32_t messagecount, uint32_t Handlercount);

public:
    Handler(AMQP::TcpConnection* connection);
    Handler(std::shared_ptr<AMQP::TcpChannel>& channel);
    virtual ~Handler() = default;
    Handler(const Handler& c) = delete;
    Handler(Handler&& c) = default;
    Handler& operator=(const Handler& c) = delete;
    Handler& operator=(Handler&& c) = default;

    AMQP::TcpChannel& channel();

    template<class... Args>
    AMQP::Deferred& declareExchange(Args... args);

    template<class... Args>
    AMQP::Deferred& declareQueue(Args... args);

    template<class... Args>
    AMQP::Deferred& bindQueue(Args... args);

    // todo: return DeferredHandler
    template<class... Args>
    AMQP::Deferred& consume(Args... args);
};

////////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////////
inline Handler::Handler(AMQP::TcpConnection* connection):
    m_channel(new AMQP::TcpChannel(connection))
{
    using namespace std::placeholders;

    m_logger = logger::Logger::getLogCategory("RMQ_HANDLER");

    channel().onError(std::bind(&Handler::onErrorCallback, this, _1));
    channel().onReady(std::bind(&Handler::onReadyCallback, this));
}

inline Handler::Handler(std::shared_ptr<AMQP::TcpChannel>& _channel):
    m_channel(_channel)
{
    using namespace std::placeholders;

    if (!m_channel)
    {
        throw std::runtime_error("Cannot create Handler with empty channel");
    }

    m_logger = logger::Logger::getLogCategory("RMQ_HANDLER");

    channel().onError(std::bind(&Handler::onErrorCallback, this, _1));
    channel().onReady(std::bind(&Handler::onReadyCallback, this));
}

inline AMQP::TcpChannel& Handler::channel()
{
    return *m_channel;
}

template<class... Args>
inline AMQP::Deferred& Handler::declareExchange(Args... args)
{
    using namespace logger;
    return channel().declareExchange(args...)
        .onSuccess(
            std::bind(
                &Handler::onSuccessCallback,
                this,
                Level::INFO,
                "Exchange was successfully declared"))
        .onFinalize(
            std::bind(
                &Handler::onFinalizeCallback,
                this,
                Level::INFO,
                "Exchange operation was finalized"));
}

template<class... Args>
inline AMQP::Deferred& Handler::declareQueue(Args... args)
{
    using namespace logger;
    using namespace std::placeholders;
    return channel().declareQueue(args...)
        .onSuccess(std::bind(&Handler::onQueueSuccessCallback, this, _1, _2, _3))
        .onFinalize(
            std::bind(
                &Handler::onFinalizeCallback,
                this,
                Level::INFO,
                "Queue operation was finalized"));;
}

template<class... Args>
inline AMQP::Deferred& Handler::bindQueue(Args... args)
{
    using namespace logger;
    using namespace std::placeholders;
    return channel().bindQueue(args...);;
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_HANDLER_H