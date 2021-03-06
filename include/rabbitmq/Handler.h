#ifndef MY_RABBIT_MQ_HANDLER_H
#define MY_RABBIT_MQ_HANDLER_H

#include <memory>

#include <amqpcpp.h>

#include "../common/Types.h"
#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "EventLoop.h"
#include "TcpHandler.h"

namespace libconfig
{
class Config;
class Setting;
} // namespace libconfig

namespace rabbitmq
{
using common::State;
using common::Result;

class Handler
{
protected:
    State m_state = State::CREATED;

    std::unique_ptr<AMQP::Address> m_address;

    EventLoop& m_eventLoop;
    TcpHandler m_handler;

    std::shared_ptr<AMQP::TcpConnection> m_connection;
    std::shared_ptr<AMQP::TcpChannel> m_channel;

    logger::CategoryPtr m_logger;

protected:
    virtual Result doInitialize();
    virtual Result doConfigure(const libconfig::Config& cfg);
    virtual Result doStart();
    virtual void doStop();
    virtual void doDeinitialize();

public:
    Handler(EventLoop& loop) throw(std::runtime_error);
    virtual ~Handler() = default;
    Handler(const Handler& h) = delete;
    Handler(Handler&& h) = default;
    Handler& operator=(const Handler& h) = delete;
    Handler& operator=(Handler&& h) = default;

    Result initialize();
    Result configure(const libconfig::Config& cfg);
    Result start();
    void stop();
    void deinitialize();

    AMQP::TcpChannel& channel();
    AMQP::TcpChannel* channelPtr();

    template<class... Args>
    Result declareExchangeSync(Args... args);

    template<class... Args>
    Result declareQueueSync(Args... args);

    template<class... Args>
    Result bindQueueSync(Args... args);
};
} // namespace rabbitmq

#include "HandlerImpl.hpp"

#endif // MY_RABBIT_MQ_HANDLER_H