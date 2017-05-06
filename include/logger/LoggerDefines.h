#ifndef LOGGER_DEFINES_H
#define LOGGER_DEFINES_H

#include "Category.h"

#define LOG_DEBUG(LOGGER, ...) \
{ \
    if ((LOGGER)->isCategoryLevel(logger::Level::DEBUG)) \
    { \
        (LOGGER)->debug(__VA_ARGS__); \
    } \
}

#define LOG_INFO(LOGGER, ...) \
{ \
    if ((LOGGER)->isCategoryLevel(logger::Level::INFO)) \
    { \
        (LOGGER)->info(__VA_ARGS__); \
    } \
}

#define LOG_WARN(LOGGER, ...) \
{ \
    if ((LOGGER)->isCategoryLevel(logger::Level::WARN)) \
    { \
        (LOGGER)->warn(__VA_ARGS__); \
    } \
}

#define LOG_ERROR(LOGGER, ...) \
{ \
    if ((LOGGER)->isCategoryLevel(logger::Level::ERROR)) \
    { \
        (LOGGER)->error(__VA_ARGS__); \
    } \
}


#define LOG(LOGGER, LEVEL, ...) \
{ \
    if ((LOGGER)->isCategoryLevel((LEVEL))) \
    { \
        (LOGGER)->log((LEVEL), __VA_ARGS__); \
    } \
}

#endif // LOGGER_DEFINES_H