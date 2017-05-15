#ifndef DB_IN_MEMORY_STORAGE_H
#define DB_IN_MEMORY_STORAGE_H

#include <ctime>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "../logger/LoggerFwd.h"

#include "Storage.h"

namespace db
{
class InMemoryStorage : public Storage
{
private:
    struct Scores
    {

    };
    struct UserStorage
    {
        std::string m_name;

        typedef std::time_t Time;
        typedef int64_t Score;
        typedef std::map<Time, Score> Scores;
        Scores m_scores;

        UserStorage(const std::string& name):
            m_name(name)
        {}

        int64_t getWeekScores(const Time currentTime) const;
    };
    typedef std::unordered_map<uint64_t, UserStorage> UsersStorage;
    UsersStorage m_users;
    mutable std::mutex m_usersMapGuard;

    logger::CategoryPtr m_logger;

private:
    void buildLeaderBoard();

public:
    InMemoryStorage();
    virtual ~InMemoryStorage() = default;

    virtual Result storeUser(const uint64_t id, const std::string& name) override;
    virtual Result renameUser(const uint64_t id, const std::string& name) override;
    virtual Result storeUserDeal(const uint64_t id, const std::time_t t, const int64_t amount) override;

    virtual Result getUser(User& user, const uint64_t id) const override;
    virtual Result getLeaderboard(
        Leaderboard& lb,
        const uint64_t id,
        const uint64_t before = 10,
        const uint64_t after = 10)
            const override;
    virtual Result getLeaderboard(
        Leaderboard& lb,
        const int64_t count)
            const override;
};
} // namespace db

#endif // DB_IN_MEMORY_STORAGE_H