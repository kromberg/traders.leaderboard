#ifndef CATEGORY_IMPL_HPP
#define CATEGORY_IMPL_HPP

#include "LogWriter.h"

namespace logger
{
inline Category::Category(
    const std::string& name,
    const Level level,
    LogWriter& writer):
    m_name(name),
    m_level(level),
    m_writer(writer)
{}

inline const Level Category::getLevel() const
{
    return m_level;
}

inline bool Category::isCategoryLevel(const Level level) const
{
    return level <= m_level;
}

template <class ... Args>
inline void Category::error(const std::string& fmt, Args ... args)
{
    log(Level::ERROR, fmt, args...);
}

template <class ... Args>
inline void Category::warn(const std::string& fmt, Args ... args)
{
    log(Level::WARN, fmt, args...);
}

template <class ... Args>
inline void Category::info(const std::string& fmt, Args ... args)
{
    log(Level::INFO, fmt, args...);
}

template <class ... Args>
inline void Category::debug(const std::string& fmt, Args ... args)
{
    log(Level::DEBUG, fmt, args...);
}

template <class ... Args>
inline void Category::log(const Level l, const std::string& fmt, Args ... args)
{
    if (isCategoryLevel(l))
    {
        m_writer.write(l, "%-10s " + fmt, m_name.c_str(), args...);
    }
}
} // namespace logger

#endif // CATEGORY_IMPL_HPP