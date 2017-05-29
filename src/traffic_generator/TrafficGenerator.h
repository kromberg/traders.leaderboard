#ifndef MY_TG_H
#define MY_TG_H

#include <cstdint>
#include <string>

#include <rabbitmq/Fwd.h>
#include <rabbitmq/EventLoop.h>
#include <logger/LoggerFwd.h>
#include <common/Types.h>
#include <app/ApplicationBase.h>
#include <app/Configuration.h>
#include "Configuration.h"

namespace tg
{
using common::State;
using common::Result;

class Generator : public app::ApplicationBase
{
private:
    Configuration m_cfg;

    rabbitmq::PublisherPtr m_publisher;
    app::RmqHandlerCfg m_publisherCfg;

private:
    Generator();

    virtual Result doInitialize() override;
    virtual Result doConfigure(const libconfig::Config& cfg) override;
    virtual Result doStart() override;
    virtual void doStop() override;
    virtual void doDeinitialize() override;

public:
    ~Generator();
    Generator(const Generator&) = delete;
    Generator(Generator&&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator& operator=(Generator&&) = delete;
    static Generator& instance();

    Result writeData();
};

} // namespace tg
#endif // MY_TG_H