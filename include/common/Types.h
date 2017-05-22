#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

namespace common
{
enum class State
{
    CREATED,
    CONFIGURED,
    INITIALIZED,
    STARTED,
    STOPPED,
    DEINITIALIZED,
};

enum class Result
{
    SUCCESS,
    FAILED,
    USER_ALREADY_REG,
    USER_REG_ERROR,
    USER_NOT_FOUND,
    UPDATE_ERROR,
    INVALID_STATE,
    INVALID_FORMAT,
    QUEUE_OVERFLOW,
    NULL_CHANNEL,
    CMD_NOT_SUPPORTED,
    DB_ERROR,
};

const char* stateToStr(const State s);
const char* resultToStr(const Result r);

} // namespace common
#endif // COMMON_TYPES_H