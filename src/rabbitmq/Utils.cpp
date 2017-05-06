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
        case Result::QUEUE_OVERFLOW:
            return "QUEUE OVERFLOW";
    }
    return "UNKNOWN";
}
} // namespace rabbitmq