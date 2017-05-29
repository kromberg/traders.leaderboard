#include <libconfig.h++>

#include <logger/LoggerDefines.h>
#include <app/Configuration.h>

namespace app
{

Result RmqHandlerCfg::read(const libconfig::Config& cfg, const std::string& section, logger::CategoryPtr& log)
{
    using namespace libconfig;

    try
    {
        const Setting& setting = cfg.lookup(section);
        if (!setting.lookupValue("exchange", m_exchangeName))
        {
            LOG_WARN(log, "Canont find 'exchange' parameter in configuration. Default value will be used");
        }
        if (!setting.lookupValue("routing-key", m_routingKey))
        {
            LOG_WARN(log, "Canont find 'key' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(log, "Canont find section '%s' in configuration. Default values will be used", section.c_str());
    }
    if (m_exchangeName.empty())
    {
        LOG_ERROR(log, "Configuration is invalid: exchange is empty");
        return Result::CFG_INVALID;
    }
    if (m_routingKey.empty())
    {
        LOG_ERROR(log, "Configuration is invalid: routing key is empty");
        return Result::CFG_INVALID;
    }
    LOG_INFO(log, "Configuration parameters: <exchange: %s, key: %s>",
        m_exchangeName.c_str(), m_routingKey.c_str());
    return Result::SUCCESS;
}

Result RmqConsumerCfg::read(const libconfig::Config& cfg, const std::string& section, logger::CategoryPtr& log)
{
    using namespace libconfig;

    Result res = RmqHandlerCfg::read(cfg, section, log);
    if (Result::SUCCESS != res)
    {
        return res;
    }

    try
    {
        const Setting& setting = cfg.lookup(section);
        if (!setting.lookupValue("queue", m_queueName))
        {
            LOG_WARN(log, "Canont find 'queue' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(log, "Canont find section '%s' in configuration. Default values will be used", section.c_str());
    }
    if (m_queueName.empty())
    {
        LOG_ERROR(log, "Configuration is invalid: queue is empty");
        return Result::CFG_INVALID;
    }
    LOG_INFO(log, "Configuration parameters: <queue: %s>", m_queueName.c_str());
    return Result::SUCCESS;
}

} // namespace app
