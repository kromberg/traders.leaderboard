#ifndef COMMON_UTILS_IMPL_H
#define COMMON_UTILS_IMPL_H

#include <ctime>
#include <string>

namespace common
{
inline std::string timeToString(const std::time_t t, const char* fmt)
{
    static thread_local char buf[100];
    std::strftime(buf, sizeof(buf), fmt, std::gmtime(&t));
    return std::string(buf);
}

inline void timeToBuf(char* const buf, const size_t size, const std::time_t t, const char* fmt)
{
    std::strftime(buf, size, fmt, std::gmtime(&t));
}

inline time_t timeFromString(const char* buf, const char* fmt) 
{
    struct tm timeStruct;
    strptime(buf, fmt, &timeStruct);
    return mktime(&timeStruct);
}

} // namespace common

#endif // COMMON_UTILS_IMPL_H