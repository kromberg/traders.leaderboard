#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

namespace common
{
enum class State
{
    CREATED,
    INITIALIZED,
    CONFIGURED,
    STARTED,
    STOPPED,
    DEINITIALIZED,
};

enum class Result
{
    SUCCESS,
    FAILED,
    CFG_FILE_ERROR,
    CFG_PARSE_ERROR,
    CFG_INVALID,
    USER_ALREADY_REG,
    USER_REG_ERROR,
    USER_CONN_ERROR,
    USER_NOT_FOUND,
    UPDATE_ERROR,
    INVALID_STATE,
    INVALID_FORMAT,
    QUEUE_OVERFLOW,
    NULL_CHANNEL,
    CMD_NOT_SUPPORTED,
    DB_ERROR,
    LOGIC_ERROR,
    CB_NOT_FOUND,
};

const char* stateToStr(const State s);
const char* resultToStr(const Result r);

} // namespace common
#endif // COMMON_TYPES_H