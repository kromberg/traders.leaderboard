#ifndef MY_TG_CONFIGURATION_H
#define MY_TG_CONFIGURATION_H

#include <cstdint>
#include <string>

#include <libconfig.h++>

#include <common/Types.h>

namespace tg
{
using common::Result;

struct Configuration
{
    uint32_t m_usersCount = 1000;
    uint32_t m_userOffset = 0;
    uint32_t m_dealsPerUser = 10;
    uint32_t m_transactionSize = 100;

    Result read(const libconfig::Config& cfg, logger::CategoryPtr& log);
};

inline Result Configuration::read(const libconfig::Config& cfg, logger::CategoryPtr& log)
{
    using namespace libconfig;

    try
    {
        const Setting& setting = cfg.lookup("tg");

        if (!setting.lookupValue("users-count", m_usersCount))
        {
            LOG_WARN(log, "Canont find 'users-count' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("user-offset", m_userOffset))
        {
            LOG_WARN(log, "Canont find 'user-offset' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("deals-per-user", m_dealsPerUser))
        {
            LOG_WARN(log, "Canont find 'deals-per-user' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("transaction-size", m_transactionSize))
        {
            LOG_WARN(log, "Canont find 'transaction-size' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(log, "Canont find section 'tg' in configuration. Default values will be used");
    }

    LOG_INFO(log, "Traffic generator configuration: <users-count: %u, user-offset: %u, deals-per-user: %u, transaction-size: %u>",
        m_usersCount, m_userOffset, m_dealsPerUser, m_transactionSize);

    return Result::SUCCESS;
}

} // namespace tg
#endif // MY_TG_CONFIGURATION_H