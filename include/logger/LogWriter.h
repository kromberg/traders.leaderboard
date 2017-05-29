#ifndef LOGGER_WRITER_H
#define LOGGER_WRITER_H

#include <atomic>
#include <string>
#include <cstdio>

#include <libconfig.h++>

#include "LoggerFwd.h"

namespace logger
{
class LogWriter
{
public:
    friend class Category;

    enum class Type : uint8_t
    {
        FILE    = 0,
        STDOUT  = 1,
        STDERR  = 2,
        UNKNOWN,
    };
    static const char* typeToStr(const Type t);
    static Type typeFromStr(const std::string& str);

private:
    Level m_level = Level::DEBUG;
    Type m_type = Type::STDERR;
    std::string m_filename;
    FILE* m_file;

    void checkFile();

    template<class... Args>
    void write(const Level level, const std::string& fmt, Args ... args);

public:
    LogWriter() = default;
    ~LogWriter() = default;
    LogWriter(const LogWriter& level) = delete;
    LogWriter(LogWriter&& level) = default;
    LogWriter& operator=(const LogWriter& level) = delete;
    LogWriter& operator=(LogWriter&& level) = default;

    const Level level() const;

    bool configure(const libconfig::Config& cfg);
    bool start();
    void stop();
};

} // namespace logger

#include "LogWriterImpl.hpp"

#endif // LOGGER_H