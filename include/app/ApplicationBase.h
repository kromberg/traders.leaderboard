#ifndef MY_APP_APPLICATION_BASE_H
#define MY_APP_APPLICATION_BASE_H

#include "../rabbitmq/Fwd.h"
#include "../rabbitmq/EventLoop.h"
#include "../logger/LoggerFwd.h"
#include "../common/Types.h"
#include "Logic.h"
#include "Configuration.h"

namespace libconfig
{
class Config;
} // namespace libconfig

namespace app
{
using common::State;
using common::Result;

class ApplicationBase
{
protected:
    logger::CategoryPtr m_logger;

    State m_state;
    // event loop for sending/receiving events for AMQP-CPP library
    rabbitmq::EventLoop m_eventLoop;

protected:
    ApplicationBase();
    Result prepareExchangeQueue(
        rabbitmq::Handler& handler,
        const RmqConsumerCfg& cfg);

    virtual Result doInitialize();
    virtual Result doConfigure(const libconfig::Config& cfg);
    virtual Result doStart();
    virtual void doStop();
    virtual void doDeinitialize();

public:
    ApplicationBase(const ApplicationBase&) = delete;
    ApplicationBase(ApplicationBase&&) = delete;
    ApplicationBase& operator=(const ApplicationBase&) = delete;
    ApplicationBase& operator=(ApplicationBase&&) = delete;
    virtual ~ApplicationBase();

    Result initialize();
    Result configure(const std::string& filename = "cfg.cfg");
    Result start();
    void stop();
    void deinitialize();
};
} // namespace app

#endif // MY_APP_APPLICATION_BASE_H