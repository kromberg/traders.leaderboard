#ifndef LOGGER_DEFINES_H
#define LOGGER_DEFINES_H

#include "Category.h"

#define LOG_DEBUG(l, ...) \
{ \
    if (l->isCategoryLevel(logger::Level::DEBUG)) \
    { \
        l->debug(__VA_ARGS__); \
    } \
}

#define LOG_INFO(l, ...) \
{ \
    if (l->isCategoryLevel(logger::Level::INFO)) \
    { \
        l->info(__VA_ARGS__); \
    } \
}

#define LOG_WARN(l, ...) \
{ \
    if (l->isCategoryLevel(logger::Level::WARN)) \
    { \
        l->warn(__VA_ARGS__); \
    } \
}

#define LOG_ERROR(l, ...) \
{ \
    if (l->isCategoryLevel(logger::Level::ERROR)) \
    { \
        l->error(__VA_ARGS__); \
    } \
}

#endif // LOGGER_DEFINES_H