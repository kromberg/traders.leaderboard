#include <random>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>
#include <external/fantasyname/namegen.h>

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
    NameGen::Generator namesGenerator("ssM ssM");
    std::default_random_engine generator;
    std::uniform_int_distribution<uint16_t> distribution(
        0,
        std::numeric_limits<uint16_t>::max());

    // start a transaction
    Result res = Result::FAILED;
    do
    {
        res = m_publisher->startTransactionSync();
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(m_logger, "Cannot start transaction. Rollback it. Result: %d(%s)",
                static_cast<int32_t>(res), common::resultToStr(res));
            m_publisher->rollbackTransactionSync();
        }
    } while (res != Result::SUCCESS);

    for (size_t i = 0; i < m_cfg.m_usersCount; ++i)
    {
        std::string idStr = std::to_string(m_cfg.m_userOffset + i);
        if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey,
            "user_registered(" + idStr + "," + namesGenerator.toString() + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            continue;
        }
        time_t t = time(nullptr);
        for (size_t deal = 0; deal < m_cfg.m_dealsPerUser; ++ deal)
        {
            if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey,
                "user_deal(" + idStr + "," + common::timeToString(t) + "," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                continue;
            }
            if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey,
                "user_deal_won(" + idStr + "," + common::timeToString(t) + "," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                continue;
            }
        }
        if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey,
            "user_connected(" + idStr + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            continue;
        }

        if (rand() % 5 != 0)
        {
            if (!m_publisher->publish(m_publisherCfg.m_exchangeName, m_publisherCfg.m_routingKey,
                "user_disconnected(" + idStr + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                continue;
            }
        }

        if (m_publisher->transactionMessagesCount() >= m_cfg.m_transactionSize)
        {
            res = m_publisher->commitTransactionSync();
            if (Result::SUCCESS != res)
            {
                LOG_ERROR(m_logger, "Cannot commit transaction. Rollback it. Result: %d(%s)",
                    static_cast<int32_t>(res), common::resultToStr(res));
                m_publisher->rollbackTransactionSync();
            }

            LOG_INFO(m_logger, "%zu users were processed. last user: %zu", i, m_cfg.m_userOffset + i);

            res = Result::FAILED;
            do
            {
                res = m_publisher->startTransactionSync();
                if (Result::SUCCESS != res)
                {
                    LOG_ERROR(m_logger, "Cannot start transaction. Rollback it. Result: %d(%s)",
                        static_cast<int32_t>(res), common::resultToStr(res));
                    m_publisher->rollbackTransactionSync();
                }
            } while (res != Result::SUCCESS);
        }
    }

    res = m_publisher->commitTransactionSync();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot commit transaction. it. Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        m_publisher->rollbackTransactionSync();
    }

    LOG_INFO(m_logger, "%zu users were processed. last user: %zu",
        m_cfg.m_usersCount, m_cfg.m_userOffset + m_cfg.m_usersCount);

    return Result::SUCCESS;
}

} // namespace tg