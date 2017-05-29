#include <random>

#include <logger/LoggerDefines.h>
#include <rabbitmq/Publisher.h>

#include "TrafficGenerator.h"

namespace tg
{
Generator::Generator()
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

Result Generator::initialize()
{
    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot initialize Generator in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    LOG_INFO(m_logger, "TG initialization started");

    m_publisher.reset(new rabbitmq::Publisher(m_eventLoop));
    Result res = m_publisher->initialize();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot initialize m_publisher-> Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_state = State::INITIALIZED;

    LOG_INFO(m_logger, "TG initialization finished");
    return Result::SUCCESS;
}

Result Generator::configure(const std::string& filename)
{
    using namespace libconfig;

    if (State::INITIALIZED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure Generator in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }
    LOG_INFO(m_logger, "TG configuration started");

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

    if (!m_cfg.read(cfg, m_logger))
    {
        LOG_ERROR(m_logger, "Cannot read configuration");
        return Result::CFG_INVALID;
    }

    Result res = m_publisher->configure(cfg);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot configure m_publisher-> Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    m_state = State::CONFIGURED;

    LOG_INFO(m_logger, "TG configuration finished");
    return Result::SUCCESS;
}

Result Generator::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start Generator in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    LOG_INFO(m_logger, "TG start started");

    m_eventLoop.start();

    Result res = m_publisher->start();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot start m_publisher-> Result: %d(%s)",
            static_cast<int32_t>(res), common::resultToStr(res));
        return res;
    }

    res = m_publisher->declareExchangeSync(m_cfg.m_exchangeName, AMQP::fanout);
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(m_logger, "Cannot declare exchange '%s'", m_cfg.m_exchangeName.c_str());
        return res;
    }

    m_state = State::STARTED;

    LOG_INFO(m_logger, "TG start finished");
    return Result::SUCCESS;
}

void Generator::stop()
{
    if (State::STARTED != m_state)
    {
        return ;
    }

    LOG_INFO(m_logger, "TG stop started");

    m_publisher->stop();
    m_eventLoop.stop();

    m_state = State::STOPPED;

    LOG_INFO(m_logger, "TG stop finished");
    return ;
}

void Generator::deinitialize()
{
    if (State::STOPPED != m_state)
    {
        return ;
    }

    LOG_INFO(m_logger, "TG deinitialization started");

    m_publisher->deinitialize();

    m_state = State::DEINITIALIZED;

    LOG_INFO(m_logger, "TG deinitialization finished");
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
        if (!m_publisher->publish(m_cfg.m_exchangeName, m_cfg.m_routingKey, "user_registered(" + idStr + ",Abuda " + idStr + ")"))
        {
            LOG_ERROR(m_logger, "Cannot publish message");
            return Result::FAILED;
        }

        for (size_t deal = 0; deal < m_cfg.m_dealsPerUser; ++ deal)
        {
            if (!m_publisher->publish(m_cfg.m_exchangeName, m_cfg.m_routingKey, "user_deal(" + idStr + ",2017-05-24T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                return Result::FAILED;
            }
            if (!m_publisher->publish(m_cfg.m_exchangeName, m_cfg.m_routingKey, "user_deal_won(" + idStr + ",2017-05-24T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(m_logger, "Cannot publish message");
                return Result::FAILED;
            }
        }
        if (!m_publisher->publish(m_cfg.m_exchangeName, m_cfg.m_routingKey, "user_connected(" + idStr + ")"))
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