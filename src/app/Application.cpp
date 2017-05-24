#include <libconfig.h++>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <app/Application.h>

namespace app
{

Application::Application():
    m_consumerCfg("leaderboard-users", "user-key", "users-events-queue"),
    m_publisherCfg("leaderboard", "leaderboard-key")
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
    m_logic.registerPublisher(m_publisher, m_publisherCfg);

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

    Result res = readRmqConsumerCfg(
        m_consumerCfg,
        cfg,
        "application.consumer");
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read consumer configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = readRmqHandlerCfg(
        m_publisherCfg,
        cfg,
        "application.consumer");
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read publisher configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_consumer->configure(cfg);
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

    Result res = m_logic.start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start logic. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_consumer->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start consumer. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = prepareExchangeQueue(*m_consumer, m_consumerCfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot prepare consumer exchange and queue. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_consumer->setQosSync(1);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot set consumer QOS");
        return res;
    }

    res = m_consumer->consume(m_consumerCfg.m_queueName);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start consuming");
        return res;
    }

    res = m_publisher->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start publisher. Result: %d(%s)",
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

Result Application::readRmqHandlerCfg(
    RmqHandlerCfg& rmqCfg,
    libconfig::Config& cfg,
    const std::string& section)
{
    using namespace libconfig;

    try
    {
        Setting& setting = cfg.lookup(section);
        if (!setting.lookupValue("exchange", rmqCfg.m_exchangeName))
        {
            LOG_WARN(m_logger, "Canont find 'exchange' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("routing-key", rmqCfg.m_routingKey))
        {
            LOG_WARN(m_logger, "Canont find 'key' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find section '%s' in configuration. Default values will be used", section.c_str());
    }
    if (rmqCfg.m_exchangeName.empty())
    {
        LOG_ERROR(m_logger, "Configuration is invalid: exchange is empty");
        return Result::CFG_INVALID;
    }
    if (rmqCfg.m_routingKey.empty())
    {
        LOG_ERROR(m_logger, "Configuration is invalid: routing key is empty");
        return Result::CFG_INVALID;
    }
    LOG_INFO(m_logger, "Configuration parameters: <exchange: %s, key: %s>",
        rmqCfg.m_exchangeName.c_str(), rmqCfg.m_routingKey.c_str());
    return Result::SUCCESS;
}

Result Application::readRmqConsumerCfg(
    RmqConsumerCfg& rmqCfg,
    libconfig::Config& cfg,
    const std::string& section)
{
    using namespace libconfig;

    Result res = readRmqHandlerCfg(rmqCfg, cfg, section);
    if (Result::SUCCESS != res)
    {
        return res;
    }

    try
    {
        Setting& setting = cfg.lookup(section);
        if (!setting.lookupValue("queue", rmqCfg.m_queueName))
        {
            LOG_WARN(m_logger, "Canont find 'queue' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find section '%s' in configuration. Default values will be used", section.c_str());
    }
    if (rmqCfg.m_queueName.empty())
    {
        LOG_ERROR(m_logger, "Configuration is invalid: queue is empty");
        return Result::CFG_INVALID;
    }
    LOG_INFO(m_logger, "Configuration parameters: <queue: %s>", rmqCfg.m_queueName.c_str());
    return Result::SUCCESS;
}

Result Application::prepareExchangeQueue(
    rabbitmq::Handler& handler,
    const RmqConsumerCfg& cfg)
{
    Result res = handler.declareExchangeSync(cfg.m_exchangeName, AMQP::fanout);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare exchange '%s'",cfg. m_exchangeName.c_str());
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