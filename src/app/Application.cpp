#include <libconfig.h++>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <app/Application.h>

namespace app
{

Application::Application():
    ApplicationBase(),
    m_consumerCfg("leaderboard-users", "user-key", "users-events-queue"),
    m_publisherCfg("leaderboard", "leaderboard-key")
{
    m_logger = logger::Logger::getLogCategory("APP_LOGIC");
}

Application::~Application()
{}

Application& Application::instance()
{
    static Application instance;
    return instance;
}

Result Application::doInitialize()
{
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
    return Result::SUCCESS;
}

Result Application::doConfigure(const libconfig::Config& cfg)
{
    using namespace std::placeholders;
    using namespace libconfig;

    Result res = m_consumerCfg.read(cfg, "application.consumer", m_logger);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read consumer configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_publisherCfg.read(cfg, "application.publisher", m_logger);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read publisher configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_consumer->registerCallback(std::bind(&Logic::processMessage, std::ref(m_logic), _1));

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

    return Result::SUCCESS;
}

Result Application::doStart()
{
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

    res = m_publisher->declareExchangeSync(m_publisherCfg.m_exchangeName, AMQP::fanout);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare exchange '%s'", m_publisherCfg.m_exchangeName.c_str());
        return res;
    }

    m_logic.registerPublisher(m_publisher, m_publisherCfg);

    return Result::SUCCESS;
}

void Application::doStop()
{
    m_logic.stop();
    m_publisher->stop();
    m_consumer->stop();

    return ;
}

void Application::doDeinitialize()
{
    m_publisher->deinitialize();
    m_consumer->deinitialize();

    return ;
}

} // namespace app