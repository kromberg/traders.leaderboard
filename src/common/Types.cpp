#include <common/Types.h>

namespace common
{
const char* stateToStr(const State s)
{
    switch (s)
    {
        case State::CREATED:
            return "CREATED";
        case State::INITIALIZED:
            return "INITIALIZED";
        case State::CONFIGURED:
            return "CONFIGURED";
        case State::STARTED:
            return "STARTED";
        case State::STOPPED:
            return "STOPPED";
        case State::DEINITIALIZED:
            return "DEINITIALIZED";
    }
    return "UNKNOWN";
}
const char* resultToStr(const Result r)
{
    switch (r)
    {
        case Result::SUCCESS:
            return "SUCCESS";
        case Result::FAILED:
            return "FAILED";
        case Result::CFG_FILE_ERROR:
            return "CONFIGURATION FILE ERROR";
        case Result::CFG_PARSE_ERROR:
            return "COULD NOT PARSE CONFIGURATION FILE";
        case Result::CFG_INVALID:
            return "CONFIGUATION IS INVALID";
        case Result::USER_ALREADY_REG:
            return "USER ALREADY REGISTERED";
        case Result::USER_REG_ERROR:
            return "USER REGISTRATION ERROR";
        case Result::USER_CONN_ERROR:
            return "USER CONNECTION ERROR";
        case Result::USER_NOT_FOUND:
            return "USER NOT FOUND";
        case Result::UPDATE_ERROR:
            return "UPDATE OPERATION ERROR";
        case Result::INVALID_STATE:
            return "INVALID STATE";
        case Result::INVALID_FORMAT:
            return "INVALID FORMAT";
        case Result::QUEUE_OVERFLOW:
            return "QUEUE OVERFLOW";
        case Result::NULL_CHANNEL:
            return "NULL CHANNEL";
        case Result::CMD_NOT_SUPPORTED:
            return "COMMAND IS NOT SUPPORTED";
        case Result::DB_ERROR:
            return "DB ERROR";
        case Result::LOGIC_ERROR:
            return "LOGIC ERROR";
        case Result::CB_NOT_FOUND:
            return "CALLBACK NOT FOUND";
    }
    return "UNKNOWN";
}

} // namespace common