#ifndef DB_UTILS_H
#define DB_UTILS_H

#include <cstdint>
#include <ctime>

namespace db
{
enum class Result : uint16_t
{
    SUCCESS,
    FAILED,
    USER_ALREADY_REG,
    USER_REG_ERROR,
    USER_NOT_FOUND,
    UPDATE_ERROR,
};

const char* resultToStr(const Result r);

} // namespace db

#endif // DB_UTILS_H