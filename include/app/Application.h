#ifndef MY_APP_APPLICATION_H
#define MY_APP_APPLICATION_H

#include "../rabbitmq/Fwd.h"
#include "../rabbitmq/EventLoop.h"
#include "../logger/LoggerFwd.h"
#include "../common/Types.h"
#include "Logic.h"
#include "Configuration.h"

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
    RmqConsumerCfg m_consumerCfg;
    // publisher is used to send messages to RabbitMQ
    rabbitmq::PublisherPtr m_publisher;
    RmqHandlerCfg m_publisherCfg;

    // Application logic with storage
    Logic m_logic;

private:
    Application();
    Result readRmqHandlerCfg(
        RmqHandlerCfg& rmqCfg,
        libconfig::Config& cfg,
        const std::string& section);
    Result readRmqConsumerCfg(
        RmqConsumerCfg& rmqCfg,
        libconfig::Config& cfg,
        const std::string& section);
    Result prepareExchangeQueue(
        rabbitmq::Handler& handler,
        const RmqConsumerCfg& cfg);

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