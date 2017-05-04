#ifndef MY_RABBIT_MQ_TCP_HANDLER_H
#define MY_RABBIT_MQ_TCP_HANDLER_H

#include <amqpcpp.h>

#include "Fwd.h"

namespace rabbitmq
{
class TcpHandler : public AMQP::TcpHandler
{
private:
    typedef AMQP::TcpHandler BaseClass;

private:
    EventLoop& m_eventLoop;

public:
    TcpHandler(EventLoop& eventLoop);
    virtual ~TcpHandler();

private:

    virtual uint16_t onNegotiate(AMQP::TcpConnection *connection, uint16_t interval) override;
    virtual void onConnected(AMQP::TcpConnection *connection) override;
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override;
    virtual void onClosed(AMQP::TcpConnection *connection) override;
    virtual void monitor(AMQP::TcpConnection *connection, int fd, int flags) override;
};
} // namespace rabbitmq

#endif // MY_RABBIT_MQ_TCP_HANDLER_H