#include <rabbitmq/Fwd.h>

namespace rabbitmq
{
const char* resultToStr(const Result r)
{
    switch (r)
    {
        case Result::SUCCESS:
            return "SUCCESS";
        case Result::FAILED:
            return "FAILED";
        case Result::INVSTATE:
            return "INVALID STATE";
        case Result::INVFMT:
            return "INVALID FORMAT";
        case Result::QUEUE_OVERFLOW:
            return "QUEUE OVERFLOW";
        case Result::NULL_CHANNEL:
            return "NULL CHANNEL";
        case Result::CMD_NOT_SUPPORTED:
            return "COMMAND IS NOT SUPPORTED";
        case Result::DB_ERROR:
            return "DB ERROR";
    }
    return "UNKNOWN";
}
} // namespace rabbitmq