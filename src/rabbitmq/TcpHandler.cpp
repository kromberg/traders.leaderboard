#include <logger/Logger.h>

#include <logger/LoggerDefines.h>

#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>

namespace rabbitmq
{
TcpHandler::TcpHandler(EventLoop& eventLoop):
    m_eventLoop(eventLoop)
{
    m_logger = logger::Logger::getLogCategory("TCP_HANDLER");
}

TcpHandler::~TcpHandler()
{}

uint16_t TcpHandler::onNegotiate(AMQP::TcpConnection *connection, uint16_t interval)
{
    LOG_DEBUG(m_logger, "[%p] Negotiated. Interval = %u", connection, interval);

    return BaseClass::onNegotiate(connection, interval);
}

void TcpHandler::onConnected(AMQP::TcpConnection *connection)
{
    LOG_INFO(m_logger, "[%p] Connected", connection);
}
void TcpHandler::onError(AMQP::TcpConnection *connection, const char *message)
{
    LOG_INFO(m_logger, "[%p] Connection error", connection);
}
void TcpHandler::onClosed(AMQP::TcpConnection *connection)
{
    LOG_INFO(m_logger, "[%p] Connection was closed", connection);
}
void TcpHandler::monitor(AMQP::TcpConnection *connection, int fd, int flags)
{
    m_eventLoop.addConnectionItem(connection, fd, flags);
}

} // namespace rabbitmq