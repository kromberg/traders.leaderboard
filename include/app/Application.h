#ifndef MY_APP_APPLICATION_H
#define MY_APP_APPLICATION_H

#include "../rabbitmq/Fwd.h"
#include "../rabbitmq/EventLoop.h"
#include "../logger/LoggerFwd.h"
#include "../common/Types.h"
#include "Logic.h"

namespace app
{
using common::State;
using common::Result;

class Application
{
private:
    logger::CategoryPtr m_logger;

    State m_state;
    // event loop for sending/receiving events for AMQP-CPP library
    rabbitmq::EventLoop m_eventLoop;
    // consumers are used to received messages from RabbitMQ
    // todo: consumers
    rabbitmq::ConsumerPtr m_consumer;
    // publisher is used to send messages to RabbitMQ
    rabbitmq::PublisherPtr m_publisher;

    // Application logic with storage
    Logic m_logic;

    Application();
public:
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application();

    static Application& getInstance();

    Result initialize();
    Result configure(const std::string& filename = "cfg.cfg");
    Result start();
    void stop();
    void deinitialize();
};
} // namespace app

#endif // MY_APP_APPLICATION_H