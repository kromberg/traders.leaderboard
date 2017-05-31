#include <libconfig.h++>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Handler.h>

namespace rabbitmq
{


Result Handler::initialize()
{
    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot initialize Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    res = doInitialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "do initialize was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::INITIALIZED;
    return Result::SUCCESS;
}

Result Handler::configure(const libconfig::Config& cfg)
{
    using namespace libconfig;

    if (State::INITIALIZED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }
    Result res = Result::SUCCESS;

    std::string addressString("amqp://guest:guest@localhost:5672/");
    try
    {
        const Setting& setting = cfg.lookup("rabbitmq");
        if (!setting.lookupValue("address", addressString))
        {
            LOG_WARN(m_logger, "Canont find 'address' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find 'rabbitmq' section in configuration. Default values will be used");
    }

    if (addressString.empty())
    {
        LOG_ERROR(m_logger, "Configuration is invalid: address is empty");
        return Result::CFG_INVALID;
    }
    LOG_INFO(m_logger, "Configuration parameters: <address: %s>", addressString.c_str());

    m_address.reset(new AMQP::Address(addressString));

    res = doConfigure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "do configuration was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::CONFIGURED;
    return Result::SUCCESS;
}

Result Handler::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    SyncObj<Result> channelResult(Result::FAILED, 1, 5);;
    // create a AMQP connection object
    m_connection.reset(new AMQP::TcpConnection(&m_handler, *m_address));
    m_channel.reset(new AMQP::TcpChannel(m_connection.get()));

    // set callbacks
    channel().onError([&] (const char* message) -> void
        {
            LOG_ERROR(m_logger, "Channel error. Message: %s", message);
            channelResult.set(Result::FAILED);
        });
    channel().onReady([&] () -> void
        {
            LOG_INFO(m_logger, "Channel is ready");
            channelResult.set(Result::SUCCESS);
        });

    channelResult.wait();
    res = channelResult.get();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Failed to start channel");
        return res;
    }

    res = doStart();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "do start was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::STARTED;
    return Result::SUCCESS;
}

void Handler::stop()
{
    if (State::STARTED != m_state)
    {
        return ;
    }

    doStop();

    m_channel->close();
    m_channel.reset();
    m_connection.reset();

    m_state = State::STOPPED;
    return ;
}

void Handler::deinitialize()
{
    if (State::STOPPED != m_state)
    {
        return ;
    }

    doDeinitialize();

    m_state = State::DEINITIALIZED;
    return ;
}

Result Handler::doInitialize()
{
    return Result::SUCCESS;
}

Result Handler::doConfigure(const libconfig::Config&)
{
    return Result::SUCCESS;
}

Result Handler::doStart()
{
    return Result::SUCCESS;
}

void Handler::doStop()
{
    return ;
}

void Handler::doDeinitialize()
{
    return ;
}

} // namespace rabbitmq