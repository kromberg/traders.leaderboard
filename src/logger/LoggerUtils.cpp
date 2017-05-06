#include <logger/LoggerFwd.h>

namespace logger
{
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
    }
    return "UNKNOWN";
}
} // namespace logger