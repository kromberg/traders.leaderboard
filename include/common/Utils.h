#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <ctime>
#include <string>

namespace common
{
std::string timeToString(const std::time_t t, const char* fmt = "%Y-%m-%dT%H:%M:%S");
void timeToBuf(char* const buf, const size_t size, const std::time_t t, const char* fmt = "%Y-%m-%dT%H:%M:%S");
time_t timeFromString(const char* buf, const char* fmt = "%Y-%m-%dT%H:%M:%S");
} // namespace common

#include "UtilsImpl.hpp"

#endif // COMMON_UTILS_H