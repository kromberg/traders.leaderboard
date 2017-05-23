#include <libconfig.h++>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <app/Application.h>

namespace app
{

Application::Application()
{
    m_logger = logger::Logger::getLogCategory("APP_LOGIC");
}

Application::~Application()
{}

Application& Application::getInstance()
{
    static Application instance;
    return instance;
}

Result Application::initialize()
{
    using namespace std::placeholders;

    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot initialize application in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    m_consumer.reset(new rabbitmq::Consumer(m_eventLoop));
    Result res = m_consumer->initialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot initialize consumer. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_publisher.reset(new rabbitmq::Publisher(m_eventLoop));
    res = m_publisher->initialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot initialize publisher. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_logic.initialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot initialize logic. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_consumer->registerCallback(std::bind(&Logic::processMessage, std::ref(m_logic), _1));

    m_state = State::INITIALIZED;
    return Result::SUCCESS;
}

Result Application::configure(const std::string& filename)
{
    using namespace libconfig;

    if (State::INITIALIZED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure application in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

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

    Result res = m_consumer->configure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot configure consumer. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_publisher->configure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot configure publisher. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_logic.configure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot configure logic. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_state = State::CONFIGURED;
    return Result::SUCCESS;
}

Result Application::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start application in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    m_eventLoop.start();

    Result res = m_consumer->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start consumer. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_publisher->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start publisher. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_logic.start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start logic. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_state = State::STARTED;
    return Result::SUCCESS;
}

void Application::stop()
{
    if (State::STARTED != m_state)
    {
        return ;
    }

    m_logic.stop();
    m_publisher->stop();
    m_consumer->stop();

    m_eventLoop.stop();

    m_state = State::STOPPED;
    return ;
}

void Application::deinitialize()
{
    if (State::STOPPED != m_state)
    {
        return ;
    }

    m_publisher->deinitialize();
    m_consumer->deinitialize();

    m_state = State::DEINITIALIZED;
    return ;
}

} // namespace app