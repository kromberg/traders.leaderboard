#ifndef DB_FWD_H
#define DB_FWD_H

namespace db
{
enum class Result : uint16_t
{
    SUCCESS,
    FAILED,
    USER_ALREADY_REG,
    USER_NOT_FOUND,
};

const char* resultToStr(const Result r);

}
#endif // DB_FWD_H