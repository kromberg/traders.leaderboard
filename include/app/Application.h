#ifndef MY_APP_APPLICATION_H
#define MY_APP_APPLICATION_H

#include "../rabbitmq/Fwd.h"
#include "../rabbitmq/EventLoop.h"
#include "../logger/LoggerFwd.h"
#include "../common/Types.h"
#include "Logic.h"
#include "ApplicationBase.h"
#include "Configuration.h"

namespace app
{
using common::State;
using common::Result;

class Application : public ApplicationBase
{
private:
    // consumers are used to received messages from RabbitMQ
    rabbitmq::ConsumerPtr m_consumer;
    RmqConsumerCfg m_consumerCfg;
    // publisher is used to send messages to RabbitMQ
    rabbitmq::PublisherPtr m_publisher;
    RmqHandlerCfg m_publisherCfg;

    // Application logic with storage
    Logic m_logic;

private:
    Application();

    virtual Result doInitialize() override;
    virtual Result doConfigure(const libconfig::Config& cfg) override;
    virtual Result doStart() override;
    virtual void doStop() override;
    virtual void doDeinitialize() override;

public:
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&) = delete;
    ~Application();

    static Application& instance();
};
} // namespace app

#endif // MY_APP_APPLICATION_H