#ifndef LOGGER_IMPL_HPP
#define LOGGER_IMPL_HPP

#include <cassert>
#include <ctime>
#include <chrono>

#include <libconfig.h++>

#include "Category.h"

namespace logger
{

inline Logger& Logger::instance()
{
    static Logger instance;
    return instance;
}

inline CategoryPtr Logger::getLogCategory(const std::string& name)
{
    Logger& logger = instance();
    return logger.getCategory(name);
}

inline CategoryPtr Logger::getCategory(const std::string& name)
{
    if (State::STARTED != m_state)
    {
        fprintf(stderr, "Cannot create logger categor in invalid state: %d(%s)\n",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return CategoryPtr();
    }
    std::unique_lock<std::mutex> l(m_loggerGuard);
    auto it = m_loggers.find(name);
    if (m_loggers.end() == it)
    {
        auto res = m_loggers.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(new Category(name, m_writer.level(), m_writer)));
        it = res.first;
    }
    return it->second;
}

inline bool Logger::configure(const std::string& filename)
{
    using namespace libconfig;

    if (State::CREATED != m_state)
    {
        fprintf(stderr, "Cannot configure logger in invalid state: %d(%s)\n",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return false;
    }

    Config cfg;
    try
    {
        cfg.readFile(filename.c_str());
    }
    catch (const ParseException& e)
    {
        fprintf(stderr, "Cannot parse configuration file. Exception occurred: %s. "
            "Default values will be used\n", e.what());
    }
    catch (const FileIOException& e)
    {
        fprintf(stderr, "Cannot read configuration file. Exception occurred: %s. "
            "Default values will be used\n", e.what());
    }

    // first configure writer
    if (!m_writer.configure(cfg))
    {
        fprintf(stderr, "Cannot configure log writer\n");
        return false;
    }

    try
    {
        const Setting& categoriesSetting = cfg.lookup("logger.categories");

        int count = categoriesSetting.getLength();

        for (int i = 0; i < count; ++i)
        {
            const Setting &categorySetting = categoriesSetting[i];

            std::string category, level;

            if (!categorySetting.lookupValue("category", category))
            {
                fprintf(stderr, "Canont find 'category' parameter in configuration. Skip this section\n");
                continue;
            }
            if (!categorySetting.lookupValue("level", level))
            {
                fprintf(stderr, "Canont find 'level' parameter in configuration. Skip this section\n");
                continue;
            }
            Level l = levelFromStr(level);
            if (Level::UNKNOWN == l)
            {
                fprintf(stderr, "Cannot add category '%s' with unkown level '%s'",
                    category.c_str(), level.c_str());
                continue;
            }
            m_loggers.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(category),
                std::forward_as_tuple(new Category(category, l, m_writer)));
        }
    }
    catch (const SettingNotFoundException& e)
    {
        fprintf(stderr, "Canont find section 'logger.categories' in configuration. Default values will be used\n");
    }

    m_state = State::CONFIGURED;
    return true;
}

inline bool Logger::start()
{
    if (State::CONFIGURED != m_state)
    {
        fprintf(stderr, "Cannot start logger in invalid state: %d(%s)\n",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return false;
    }

    if (!m_writer.start())
    {
        fprintf(stderr, "Cannot start log writer\n");
        return false;
    }

    m_state = State::STARTED;
    return true;
}

inline bool Logger::stop()
{
    if (State::STARTED != m_state)
    {
        fprintf(stderr, "Cannot stop logger in invalid state: %d(%s)\n",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return false;
    }

    m_writer.stop();

    m_state = State::STOPPED;
    return true;
}

} // namespace logger

#endif // LOGGER_IMPL_HPP