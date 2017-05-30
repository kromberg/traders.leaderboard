#include <random>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

#include "TrafficGenerator.h"

namespace tg
{
Generator::Generator():
    app::ApplicationBase()
{
    m_logger = logger::Logger::getLogCategory("TRAFFIC_GENERATOR");
}

Generator::~Generator()
{}

Generator& Generator::instance()
{
    static Generator instance;
    return instance;
}

Result Generator::doInitialize()
{
    m_publisher.reset(new rabbitmq::Publisher(m_eventLoop));
    Result res = m_publisher->initialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot initialize m_publisher-> Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    return Result::SUCCESS;
}

Result Generator::doConfigure(const libconfig::Config& cfg)
{
    using namespace libconfig;

    Result res = m_cfg.read(cfg, m_logger);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return Result::CFG_INVALID;
    }

    res = m_publisherCfg.read(cfg, "tg.publisher", m_logger);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot read publisher configuration. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return Result::CFG_INVALID;
    }

    res = m_publisher->configure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot configure publisher. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_state = State::CONFIGURED;

    LOG_INFO(m_logger, "TG configuration finished");
    return Result::SUCCESS;
}

Result Generator::doStart()
{
    Result res = m_publisher->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start m_publisher-> Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_publisher->declareExchangeSync(m_publisherCfg.m_exchangeName, AMQP::fanout);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare exchange '%s'", m_publisherCfg.m_exchangeName.c_str());
        return res;
    }

    return Result::SUCCESS;
}

void Generator::doStop()
{
    m_publisher->stop();

    return ;
}

void Generator::doDeinitialize()
{
    m_publisher->deinitialize();
    return ;
}

Result Generator::writeData()
{
    std::default_random_engine generator;
    std::uniform_int_distribution<int16_t> distribution(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    // start a transaction
    Result res = m_publisher->startTransactionSync();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start transaction");
        return res;
    }

    for (size_t i = 0; i < m_cfg.m_usersCount; ++i)
    {
        std::string idStr = std::to_string(i);
        if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey, "user_registered(" + idStr + ",Egor " + idStr + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            return Result::FAILED;
        }

        for (size_t deal = 0; deal < m_cfg.m_dealsPerUser; ++ deal)
        {
            if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey, "user_deal(" + idStr + ",2017-05-24T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                return Result::FAILED;
            }
            if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey, "user_deal_won(" + idStr + ",2017-05-24T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                return Result::FAILED;
            }
        }
        if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey, "user_connected(" + idStr + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            return Result::FAILED;
        }

        /*if (!m_publisher->publish(m_cfg.m_exchangeName, m_cfg.m_routingKey, "user_disconnected(" + idStr + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            return ;
        }*/

        if (m_publisher->transactionMessagesCount() >= m_cfg.m_transactionSize)
        {
            res = m_publisher->commitTransactionSync();
            if (Result::SUCCESS != res)
            {
                LOG_ERROR(m_logger, "Cannot commit transaction");
                return res;
            }

            LOG_INFO(m_logger, "%zu users were processed", i);

            res = m_publisher->startTransactionSync();
            if (Result::SUCCESS != res)
            {
                LOG_ERROR(m_logger, "Cannot start transaction");
                return res;
            }
        }
    }

    res = m_publisher->commitTransactionSync();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot commit transaction");
        return res;
    }

    LOG_INFO(m_logger, "%zu users were processed", m_cfg.m_usersCount);

    return Result::SUCCESS;
}

} // namespace tg