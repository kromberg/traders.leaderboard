#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <chrono>

#define LOG(logger, level, ...) \
{ \
    static thread_local char buf[100]; \
    std::time_t t = std::time(nullptr); \
    std::strftime(buf, sizeof(buf), "%Y%m%dT%H:%M:%S", std::gmtime(&t)); \
    printf("%s %-7s %-10s ", buf, level, logger); \
    printf(__VA_ARGS__); \
    printf("\n"); \
}

#define LOG_DEBUG(logger, ...) \
{ \
    LOG(logger, "DEBUG", __VA_ARGS__) \
}

#define LOG_INFO(logger, ...) \
{ \
    LOG(logger, "INFO", __VA_ARGS__) \
}

#define LOG_WARN(logger, ...) \
{ \
    LOG(logger, "WARN", __VA_ARGS__) \
}

#define LOG_ERROR(logger, ...) \
{ \
    LOG(logger, "ERROR", __VA_ARGS__) \
}

#endif // LOGGER_H