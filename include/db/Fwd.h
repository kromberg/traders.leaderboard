#ifndef DB_FWD_H
#define DB_FWD_H

#include <cstdint>
#include <memory>

namespace db
{
class AbstractLogic;
typedef std::unique_ptr<AbstractLogic> LogicPtr;

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