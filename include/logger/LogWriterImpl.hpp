#ifndef LOG_WRITER_IMPL_HPP
#define LOG_WRITER_IMPL_HPP

#include <cassert>
#include <ctime>
#include <chrono>

#include "../common/Utils.h"
#include "Logger.h"
#include "Category.h"

namespace logger
{

inline const Level LogWriter::level() const
{
    return m_level;
}

inline bool LogWriter::configure(const std::string& filename)
{
    // todo: read configuration
    return true;
}

inline bool LogWriter::initialize()
{
    switch (m_type)
    {
        case Type::FILE:
            assert(!"Not implemented");
            break;

        case Type::STDOUT:
            m_file = stdout;
            break;

        case Type::STDERR:
            m_file = stderr;
            break;
    }

    return true;
}

inline void LogWriter::deinitialize()
{}

template<class... Args>
inline void LogWriter::write(
    const Level level,
    const std::string& fmt,
    Args ... args)
{
    static thread_local char timeBuf[100];
    common::timeToBuf(timeBuf, sizeof(timeBuf), std::time(nullptr));
    fprintf(m_file, ("%s %-7s " + fmt + "\n").c_str(), timeBuf, levelToStr(level), std::forward<Args>(args)...);
}

} // namespace logger

#endif // LOG_WRITER_IMPL_HPP