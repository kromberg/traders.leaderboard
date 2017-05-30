#ifndef LOG_WRITER_IMPL_HPP
#define LOG_WRITER_IMPL_HPP

#include <cassert>
#include <ctime>
#include <chrono>
#include <algorithm>

#include "../common/Utils.h"
#include "Logger.h"
#include "Category.h"

namespace logger
{

inline const char* LogWriter::typeToStr(const Type t)
{
    switch (t)
    {
        case Type::FILE:
            return "FILE";
        case Type::STDOUT:
            return "STDOUT";
        case Type::STDERR:
            return "STDERR";
        case Type::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

inline LogWriter::Type LogWriter::typeFromStr(const std::string& str)
{
    std::string strLower;
    strLower.resize(str.size());
    std::transform(str.begin(), str.end(), strLower.begin(), ::tolower);

    if ("file" == strLower)
    {
        return Type::FILE;
    }
    else if ("stdout" == strLower)
    {
        return Type::STDOUT;
    }
    else if ("stderr" == strLower)
    {
        return Type::STDERR;
    }
    return Type::UNKNOWN;
}

inline const Level LogWriter::level() const
{
    return m_level;
}

inline bool LogWriter::configure(const libconfig::Config& cfg)
{
    using namespace libconfig;

    std::string level = "debug", type = "stderr";
    try
    {
        const Setting& setting = cfg.lookup("logger");

        if (!setting.lookupValue("level", level))
        {
            fprintf(stderr, "Canont find 'level' parameter in configuration. Default value will be used\n");
        }
        if (!setting.lookupValue("type", type))
        {
            fprintf(stderr, "Canont find 'level' parameter in configuration. Default value will be used\n");
        }

        Level l = levelFromStr(level);
        if (Level::UNKNOWN == l)
        {
            fprintf(stderr, "Cannot configure log writer with unkown level '%s'\n",
                level.c_str());
            return false;
        }

        Type t = typeFromStr(type);
        if (Type::UNKNOWN == t)
        {
            fprintf(stderr, "Cannot configure log writer with unkown type '%s'\n",
                type.c_str());
            return false;
        }

        if (Type::FILE == t)
        {
            if (!setting.lookupValue("filename", m_filename))
            {
                fprintf(stderr, "Canont find 'filename' parameter in configuration\n");
                return false;
            }
        }

        m_level = l;
        m_type = t;
    }
    catch (const SettingNotFoundException& e)
    {
        fprintf(stderr, "Canont find section 'logger' in configuration. Default values will be used\n");
    }

    return true;
}

inline bool LogWriter::start()
{
    switch (m_type)
    {
        case Type::FILE:
            m_file = fopen(m_filename.c_str(), "r+");
            if (!m_file)
            {
                m_file = fopen(m_filename.c_str(), "w+");
            }
            if (!m_file)
            {
                fprintf(stderr, "Cannot open file '%s' for writing\n", m_filename.c_str());
                return false;
            }
            break;

        case Type::STDOUT:
            m_file = stdout;
            break;

        case Type::STDERR:
            m_file = stderr;
            break;
        case Type::UNKNOWN:
            fprintf(stderr, "Cannot determine log writer type\n");
            return false;
    }

    return true;
}

inline void LogWriter::stop()
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

    fflush(m_file);
}

} // namespace logger

#endif // LOG_WRITER_IMPL_HPP