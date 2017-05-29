#ifndef COMMON_UTILS_IMPL_H
#define COMMON_UTILS_IMPL_H

#include <ctime>
#include <string>

namespace common
{
inline std::string timeToString(const std::time_t t, const char* fmt)
{
    static thread_local char buf[100];
    std::strftime(buf, sizeof(buf), fmt, std::localtime(&t));
    return std::string(buf);
}

inline void timeToBuf(char* const buf, const size_t size, const std::time_t t, const char* fmt)
{
    std::strftime(buf, size, fmt, std::localtime(&t));
}

inline Result timeFromString(time_t& timeRes, const char* buf, const char* fmt) 
{
    struct tm timeStruct = {0};
    char* res = strptime(buf, fmt, &timeStruct);
    if (!res)
    {
        return Result::INVALID_FORMAT;
    }
    timeRes = mktime(&timeStruct);
    return Result::SUCCESS;
}

} // namespace common

#endif // COMMON_UTILS_IMPL_H