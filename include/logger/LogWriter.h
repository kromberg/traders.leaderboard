#ifndef LOGGER_WRITER_H
#define LOGGER_WRITER_H

#include <string>
#include <cstdio>

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
    };
private:
    Level m_level = Level::DEBUG;
    Type m_type = Type::STDERR;
    std::string m_filename;
    FILE* m_file;

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

    bool configure(const std::string& filename);
    bool start();
    void stop();
};

} // namespace logger

#include "LogWriterImpl.hpp"

#endif // LOGGER_H