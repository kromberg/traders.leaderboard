#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <cstdio>
#include <unordered_map>
#include <mutex>

#include "../common/Types.h"
#include "LoggerFwd.h"
#include "LogWriter.h"

namespace logger
{
using common::State;

class Logger
{
private:
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

    static Logger& instance();
    static CategoryPtr getLogCategory(const std::string& name);
    CategoryPtr getCategory(const std::string& name);

    bool configure(const std::string& filename = std::string());
    bool initialize();
    bool deinitialize();
};

} // namespace logger

#include "LoggerImpl.hpp"

#endif // LOGGER_H