#ifndef DB_FWD_H
#define DB_FWD_H

#include <cstdint>
#include <memory>

namespace db
{
class Storage;
typedef std::unique_ptr<Storage> StoragePtr;
}

#endif // DB_FWD_H