#include <libconfig.h++>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <app/ApplicationBase.h>

namespace app
{

ApplicationBase::ApplicationBase()
{}

ApplicationBase::~ApplicationBase()
{}

Result ApplicationBase::initialize()
{
    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot initialize ApplicationBase in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    LOG_INFO(m_logger, "ApplicationBase initialization started");

    Result res = doInitialize();
    if (Result::SUCCESS != res)
    {
        return res;
    }

    m_state = State::INITIALIZED;

    LOG_INFO(m_logger, "ApplicationBase initialization finished");
    return Result::SUCCESS;
}

Result ApplicationBase::configure(const std::string& filename)
{
    using namespace std::placeholders;
    using namespace libconfig;

    if (State::INITIALIZED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure ApplicationBase in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    LOG_INFO(m_logger, "ApplicationBase configuration started");

    Config cfg;
    try
    {
        cfg.readFile(filename.c_str());
    }
    catch (const ParseException& e)
    {
        LOG_WARN(m_logger, "Cannot parse configuration file. Exception occurred: %s. "
            "Default values will be used", e.what());
    }
    catch (const FileIOException& e)
    {
        LOG_WARN(m_logger, "Cannot read configuration file. Exception occurred: %s. "
            "Default values will be used", e.what());
    }

    Result res = doConfigure(cfg);
    if (Result::SUCCESS != res)
    {
        return res;
    }

    m_state = State::CONFIGURED;

    LOG_INFO(m_logger, "ApplicationBase configuration finished");
    return Result::SUCCESS;
}

Result ApplicationBase::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start ApplicationBase in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    LOG_INFO(m_logger, "ApplicationBase start started");

    m_eventLoop.start();

    Result res = doStart();
    if (Result::SUCCESS != res)
    {
        return res;
    }

    m_state = State::STARTED;

    LOG_INFO(m_logger, "ApplicationBase start finished");
    return Result::SUCCESS;
}

void ApplicationBase::stop()
{
    if (State::STARTED != m_state)
    {
        return ;
    }

    LOG_INFO(m_logger, "ApplicationBase stop started");

    doStop();

    m_eventLoop.stop();

    m_state = State::STOPPED;
    LOG_INFO(m_logger, "ApplicationBase stop finished");
    return ;
}

void ApplicationBase::deinitialize()
{
    if (State::STOPPED != m_state)
    {
        return ;
    }

    LOG_INFO(m_logger, "ApplicationBase deinitialization started");

    doDeinitialize();

    m_state = State::DEINITIALIZED;

    LOG_INFO(m_logger, "ApplicationBase deinitialization finished");
    return ;
}

Result ApplicationBase::doInitialize()
{
    return Result::SUCCESS;
}

Result ApplicationBase::doConfigure(const libconfig::Config& )
{
    return Result::SUCCESS;
}

Result ApplicationBase::doStart()
{
    return Result::SUCCESS;
}

void ApplicationBase::doStop()
{
    return;
}

void ApplicationBase::doDeinitialize()
{
    return;
}

Result ApplicationBase::prepareExchangeQueue(
    rabbitmq::Handler& handler,
    const RmqConsumerCfg& cfg)
{
    Result res = handler.declareExchangeSync(cfg.m_exchangeName, AMQP::fanout);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare exchange '%s'", cfg.m_exchangeName.c_str());
        return res;
    }
    res = handler.declareQueueSync(cfg.m_queueName);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare queue '%s'", cfg.m_queueName.c_str());
        return res;
    }
    res = handler.bindQueueSync(cfg.m_exchangeName, cfg.m_queueName, cfg.m_routingKey);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot bind queue '%s' to exchange '%s' with key '%s'",
            cfg.m_exchangeName.c_str(), cfg.m_queueName.c_str(), cfg.m_routingKey.c_str());
        return res;
    }
    return Result::SUCCESS;
}

} // namespace app