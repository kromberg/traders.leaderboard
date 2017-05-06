#ifndef LOGGER_FWD_H
#define LOGGER_FWD_H

#include <memory>

namespace logger
{
class Category;
typedef std::shared_ptr<Category> CategoryPtr;

class Logger;
class LogWriter;

enum class Level : uint8_t
{
    ERROR   = 0,
    WARN    = 1,
    INFO    = 2,
    DEBUG   = 3,
};

const char* levelToStr(const Level level);

} // namespace logger
#endif // LOGGER_FWD_H