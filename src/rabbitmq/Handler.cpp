#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/Handler.h>

namespace rabbitmq
{

Result Handler::configure(AMQP::Address&& address)
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }
    Result res = Result::SUCCESS;

    m_address = std::move(address);

    res = customConfigure();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Custom configuration was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::CONFUGRED;
    return Result::SUCCESS;
}

Result Handler::initialize()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot initialize Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    res = customInitialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Custom initialize was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::INITIALIZED;
    return Result::SUCCESS;
}

Result Handler::start()
{
    if (State::STARTED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    SyncObj<Result> channelResult;
    // create a AMQP connection object
    m_connection.reset(new AMQP::TcpConnection(&m_handler, m_address));
    m_channel.reset(new AMQP::TcpChannel(m_connection.get()));

    // set callbacks
    channel().onError([] (const char* message) -> void
        {
            LOG_ERROR(m_logger, "Channel error. Message: %s", message);
            channelResult.set(Result::FAILED);
        });
    channel().onReady([] () -> void
        {
            LOG_INFO(m_logger, "Channel is ready");
            channelResult.set(Result::SUCCESS);
        });

    m_channelReady.wait();
    res = channelResult.get();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Failed to start channel");
        return res;
    }

    res = customStart();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Custom start was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::STARTED;
    return Result::SUCCESS;
}

Result Handler::stop()
{
    if (State::STARTED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot stop Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    res = customStart();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Custom stop was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::STOPPED;
    return Result::SUCCESS;
}

Result Handler::deinitialize()
{
    if (State::STOPPED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot DEinitialize Handler in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    Result res = Result::SUCCESS;

    res = customDeinitialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Custom deinitialize was failed with result %d(%s)",
            static_cast<int32_t>(res), resultToStr(res));
        return res;
    }
    m_state = State::STOPPED;
    return Result::SUCCESS;
}

Result Handler::customInitialize()
{
    return Result::SUCCESS;
}

Result Handler::customConfigure()
{
    return Result::SUCCESS;
}

Result Handler::customStart()
{
    return Result::SUCCESS;
}

Result Handler::customStop()
{
    return Result::SUCCESS;
}

Result Handler::customDeinitialize()
{
    return Result::SUCCESS;
}

} // namespace rabbitmq