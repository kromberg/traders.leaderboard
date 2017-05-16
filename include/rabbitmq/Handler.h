#ifndef MY_RABBIT_MQ_HANDLER_H
#define MY_RABBIT_MQ_HANDLER_H

#include <functional>
#include <memory>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Utils.h"
#include "EventLoop.h"
#include "TcpHandler.h"

namespace rabbitmq
{

class Handler
{
protected:
    EventLoop& m_eventLoop;
    TcpHandler m_handler;
    std::shared_ptr<AMQP::TcpConnection> m_connection;
    std::shared_ptr<AMQP::TcpChannel> m_channel;
    SynchObj m_channelReady;

    logger::CategoryPtr m_logger;

protected:
    virtual void onStartCallback(const std::string &Handlertag);
    virtual void onErrorCallback(const char *message);
    virtual void onReadyCallback();
    virtual void onSuccessCallback(const logger::Level level, const char* logMessage);
    virtual void onFinalizeCallback(const logger::Level level, const char* logMessage);
    virtual void onQueueSuccessCallback(const std::string &name, uint32_t messagecount, uint32_t Handlercount);

public:
    Handler(EventLoop& loop, const AMQP::Address &address) throw(std::runtime_error);
    virtual ~Handler() = default;
    Handler(const Handler& c) = delete;
    Handler(Handler&& c) = default;
    Handler& operator=(const Handler& c) = delete;
    Handler& operator=(Handler&& c) = default;

    AMQP::TcpChannel& channel();
    void waitChannelReady();

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
inline Handler::Handler(EventLoop& loop, const AMQP::Address &address) throw(std::runtime_error):
    m_eventLoop(loop),
    m_handler(m_eventLoop)
{
    using namespace std::placeholders;

    m_logger = logger::Logger::getLogCategory("RMQ_HANDLER");

    // create a AMQP connection object
    m_connection.reset(new AMQP::TcpConnection(&m_handler, address));
    m_channel.reset(new AMQP::TcpChannel(m_connection.get()));

    // set callbacks
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

    return channel().declareExchange(std::forward<Args>(args)...)
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

    return channel().declareQueue(std::forward<Args>(args)...)
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
    return channel().bindQueue(std::forward<Args>(args)...);
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_HANDLER_H