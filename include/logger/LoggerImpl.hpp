#ifndef LOGGER_IMPL_HPP
#define LOGGER_IMPL_HPP

#include <cassert>
#include <ctime>
#include <chrono>

#include "Category.h"

namespace logger
{

inline Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

inline CategoryPtr Logger::getLogCategory(const std::string& name)
{
    Logger& logger = getInstance();
    return logger.getCategory(name);
}

inline CategoryPtr Logger::getCategory(const std::string& name)
{
    if (State::INITIALIZED != m_state)
    {
        return CategoryPtr();
    }
    std::unique_lock<std::mutex> l(m_loggerGuard);
    auto it = m_loggers.find(name);
    if (m_loggers.end() == it)
    {
        auto res = m_loggers.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(new Category(name, m_writer.getLevel(), m_writer)));
        it = res.first;
    }
    return it->second;
}

inline bool Logger::configure(const std::string& filename)
{
    if (!m_writer.configure(filename))
    {
        return false;
    }

    m_state = State::CONFIGURED;
    return true;
}

inline bool Logger::initialize()
{
    if (!m_writer.initialize())
    {
        return false;
    }

    m_state = State::INITIALIZED;
    return true;
}

inline void Logger::deinitialize()
{
    m_writer.deinitialize();

    m_state = State::DEINITIALIZED;
}

} // namespace logger

#endif // LOGGER_IMPL_HPP