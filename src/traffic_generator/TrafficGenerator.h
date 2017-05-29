#ifndef MY_TG_H
#define MY_TG_H

#include <cstdint>
#include <string>

#include <rabbitmq/Fwd.h>
#include <rabbitmq/EventLoop.h>
#include <logger/LoggerFwd.h>
#include <common/Types.h>
#include "Configuration.h"

namespace tg
{
using common::State;
using common::Result;

class Generator
{
private:
    logger::CategoryPtr m_logger;

    State m_state = State::CREATED;
    Configuration m_cfg;
    rabbitmq::EventLoop m_eventLoop;
    rabbitmq::PublisherPtr m_publisher;

private:
    Generator();
public:
    ~Generator();
    Generator(const Generator&) = delete;
    Generator(Generator&&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator& operator=(Generator&&) = delete;
    static Generator& instance();

    Result initialize();
    Result configure(const std::string& filename = "cfg.cfg");
    Result start();
    void stop();
    void deinitialize();

    Result writeData();
};

} // namespace tg
#endif // MY_TG_H