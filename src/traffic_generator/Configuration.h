#ifndef MY_TG_CONFIGURATION_H
#define MY_TG_CONFIGURATION_H

#include <cstdint>
#include <string>

#include <libconfig.h++>

namespace tg
{
struct Configuration
{
    uint64_t m_usersCount = 1000;
    uint64_t m_dealsPerUser = 10;
    uint64_t m_transactionSize = 100;
    std::string m_exchangeName = "leaderboard-users";
    std::string m_queueName = "users-events-queue";
    std::string m_routingKey = "user-key";

    bool read(libconfig::Config& cfg, logger::CategoryPtr log);
};

inline bool Configuration::read(libconfig::Config& cfg, logger::CategoryPtr log)
{
    using namespace libconfig;

    try
    {
        Setting& setting = cfg.lookup("tg");

        if (!setting.lookupValue("users-count", m_usersCount))
        {
            LOG_WARN(log, "Canont find 'users_count' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("deals-per-user", m_dealsPerUser))
        {
            LOG_WARN(log, "Canont find 'deals_per_user' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("transaction-size", m_transactionSize))
        {
            LOG_WARN(log, "Canont find 'transaction_size' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("tg.exchange", m_exchangeName))
        {
            LOG_WARN(log, "Canont find 'tg.exchange' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("tg.queue", m_queueName))
        {
            LOG_WARN(log, "Canont find 'tg.queue' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("tg.routing-key", m_routingKey))
        {
            LOG_WARN(log, "Canont find 'tg.routing-key' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(log, "Canont find section 'tg' in configuration. Default values will be used");
    }

    LOG_INFO(log, "Traffic generator configuration: <users-count: %lu, deals-per-user: %lu, transaction-size: %lu, "
        "exchange: %s, queue: %s, routing-key: %s>",
        m_usersCount, m_dealsPerUser, m_transactionSize,
        m_exchangeName.c_str(), m_queueName.c_str(), m_routingKey.c_str());

    return true;
}

} // namespace tg
#endif // MY_TG_CONFIGURATION_H