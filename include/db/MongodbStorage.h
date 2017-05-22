#ifndef DB_MONGO_STORAGE_H
#define DB_MONGO_STORAGE_H

#include <ctime>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include <mongocxx/client.hpp>

#include "../logger/LoggerFwd.h"

#include "Storage.h"

namespace db
{
class MongodbStorage : public Storage
{
private:
    mongocxx::client m_client;
    mongocxx::database m_db;
    mutable mongocxx::collection m_collection;
    mutable mongocxx::collection m_connectedUsersCollection;

    logger::CategoryPtr m_logger;

private:
    std::unordered_set<int64_t> getConnectedUsers() const;

public:
    MongodbStorage();
    virtual ~MongodbStorage() = default;

    virtual Result storeUser(const int64_t id, const std::string& name) override;
    virtual Result renameUser(const int64_t id, const std::string& name) override;
    virtual Result storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount) override;

    virtual Result storeConnectedUser(const int64_t id) override;
    virtual Result removeConnectedUser(const int64_t id) override;

    virtual Result getUser(User& user, const int64_t id) const override;

    virtual Result getLeaderboards(
        Leaderboards& leaderboards,
        const int64_t count = -1,
        const uint64_t before = 10,
        const uint64_t after = 10)
            const override;

};
} // namespace db

#endif // DB_MONGO_STORAGE_H