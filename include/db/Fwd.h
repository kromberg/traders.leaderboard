#ifndef DB_FWD_H
#define DB_FWD_H

#include <cstdint>
#include <memory>

namespace db
{
class Storage;
typedef std::unique_ptr<Storage> StoragePtr;

enum class Result : uint16_t
{
    SUCCESS,
    FAILED,
    USER_ALREADY_REG,
    USER_NOT_FOUND,
    OPERATION_ERROR,
};

const char* resultToStr(const Result r);

}
#endif // DB_FWD_H