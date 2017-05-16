#ifndef DB_STORAGE_H
#define DB_STORAGE_H

#include <string>
#include <map>

#include "Fwd.h"

namespace db
{

struct User
{
    uint64_t m_id;
    std::string m_name;

    User(const uint64_t id, const std::string& name):
        m_id(id), m_name(name)
    {}
};

// score to user
typedef std::multimap<int64_t, User> Leaderboard;

class Storage
{
private:

public:
    Storage() = default;
    virtual ~Storage() = default;

    virtual Result storeUser(const uint64_t id, const std::string& name) = 0;
    virtual Result renameUser(const uint64_t id, const std::string& name) = 0;
    virtual Result storeUserDeal(const uint64_t id, const std::time_t t, const int64_t amount) = 0;

    virtual Result getUser(User& user, const uint64_t id) const = 0;
    virtual Result getUserLeaderboard(Leaderboard& lb, const uint64_t id, const uint64_t before, const uint64_t after) const = 0;
    virtual Result getLeaderboard(Leaderboard& lb, const int64_t count) const = 0;
};
} // namespace db

#endif // DB_STORAGE_H