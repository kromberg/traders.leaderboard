#ifndef DB_MONGO_STORAGE_H
#define DB_MONGO_STORAGE_H

#include <ctime>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <mutex>

#include <mongocxx/pool.hpp>

#include "../logger/LoggerFwd.h"
#include "../common/Types.h"
#include "Storage.h"

namespace db
{
using common::State;
using common::Result;

class MongodbStorage : public Storage
{
private:
    State m_state = State::CREATED;

    std::string m_uri;
    std::string m_dbName;
    std::string m_usersCollectionName;
    std::string m_connectedUsersCollectionName;

    mutable std::unique_ptr<mongocxx::pool> m_pool;
    mutable std::mutex m_poolGuard;

    logger::CategoryPtr m_logger;

private:
    std::unordered_set<int64_t> getConnectedUsers() const;

public:
    MongodbStorage();
    virtual ~MongodbStorage() = default;

    virtual Result configure(const libconfig::Config& cfg) override;
    virtual Result start() override;

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