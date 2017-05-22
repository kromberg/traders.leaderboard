#ifndef MY_RABBIT_MQ_HANDLER_H
#define MY_RABBIT_MQ_HANDLER_H

#include <memory>

#include <amqpcpp.h>

#include "../common/State.h"
#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Utils.h"
#include "EventLoop.h"
#include "TcpHandler.h"

namespace rabbitmq
{
using common::State;
using common::Result;

class Handler
{
protected:
    State m_state = State::CREATED;

    AMQP::Address m_address;

    EventLoop& m_eventLoop;
    TcpHandler m_handler;

    std::shared_ptr<AMQP::TcpConnection> m_connection;
    std::shared_ptr<AMQP::TcpChannel> m_channel;

    logger::CategoryPtr m_logger;

protected:
    virtual Result customInitialize();
    virtual Result customConfigure();
    virtual Result customStart();
    virtual Result customStop();
    virtual Result customDeinitialize();

public:
    Handler(EventLoop& loop) throw(std::runtime_error);
    virtual ~Handler() = default;
    Handler(const Handler& h) = delete;
    Handler(Handler&& h) = default;
    Handler& operator=(const Handler& h) = delete;
    Handler& operator=(Handler&& h) = default;

    Result configure(AMQP::Address&& address);
    Result initialize();
    Result start();
    Result stop();
    Result deinitialize();

    AMQP::TcpChannel& channel();

    template<class... Args>
    Result declareExchangeSync(Args... args);

    template<class... Args>
    Result declareQueueSync(Args... args);

    template<class... Args>
    Result bindQueueSync(Args... args);

    template<class... Args>
    Result setQosSync(Args... args);
};
} // namespace rabbitmq

#include "HandlerImpl.hpp"

#endif // MY_RABBIT_MQ_HANDLER_H