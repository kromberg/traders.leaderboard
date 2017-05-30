#include <string>
#include <algorithm>

#include <logger/LoggerFwd.h>

namespace logger
{

Level levelFromStr(const std::string& str)
{
    std::string strLower;
    strLower.resize(str.size());
    std::transform(str.begin(), str.end(), strLower.begin(), ::tolower);

    if ("error" == strLower)
    {
        return Level::ERROR;
    }
    else if ("warn" == strLower)
    {
        return Level::WARN;
    }
    else if ("info" == strLower)
    {
        return Level::INFO;
    }
    else if ("debug" == strLower)
    {
        return Level::DEBUG;
    }
    return Level::UNKNOWN;
}

const char* levelToStr(const Level level)
{
    switch (level)
    {
        case Level::ERROR:
            return "ERROR";
        case Level::WARN :
            return "WARN";
        case Level::INFO :
            return "INFO";
        case Level::DEBUG:
            return "DEBUG";
        case Level::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
} // namespace logger