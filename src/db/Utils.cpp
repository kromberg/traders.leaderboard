#include <db/Fwd.h>

namespace db
{
const char* resultToStr(const Result r)
{
    switch (r)
    {
        case Result::SUCCESS:
            return "SUCCESS";
        case Result::FAILED:
            return "FAILED";
        case Result::USER_ALREADY_REG:
            return "USER ALREADY REGISTERED";
        case Result::USER_NOT_FOUND:
            return "USER NOT FOUND";
        case Result::OPERATION_ERROR:
            return "OPERATION ERROR";
    }
    return "UNKNOWN";
}
} // namespace db