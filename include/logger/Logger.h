#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <cstdio>
#include <unordered_map>
#include <mutex>

#include "LoggerFwd.h"
#include "LogWriter.h"

namespace logger
{
class Logger
{
private:
    enum class State : uint8_t
    {
        CREATED,
        CONFIGURED,
        INITIALIZED,
        DEINITIALIZED,
    };

    State m_state = State::CREATED;
    LogWriter m_writer;

    // map of logger categories
    std::unordered_map<std::string, CategoryPtr> m_loggers;
    std::mutex m_loggerGuard;

    Logger() = default;

public:
    ~Logger() = default;
    Logger(const Logger& level) = delete;
    Logger(Logger&& level) = delete;
    Logger& operator=(const Logger& level) = delete;
    Logger& operator=(Logger&& level) = delete;

    static Logger& getInstance();
    static CategoryPtr getLogCategory(const std::string& name);
    CategoryPtr getCategory(const std::string& name);

    bool configure(const std::string& filename = std::string());
    bool initialize();
    void deinitialize();
};

} // namespace logger

#include "LoggerImpl.hpp"

#endif // LOGGER_H