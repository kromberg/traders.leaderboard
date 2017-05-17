#ifndef DB_STORAGE_H
#define DB_STORAGE_H

#include <string>
#include <map>

#include "Fwd.h"

namespace db
{

struct User
{
    int64_t m_id;
    std::string m_name;

    User(const int64_t id, const std::string& name):
        m_id(id), m_name(name)
    {}
};

// score to user
typedef std::multimap<int64_t, User, std::greater<int64_t> > Leaderboard;

class Storage
{
private:

public:
    Storage() = default;
    virtual ~Storage() = default;

    virtual Result storeUser(const int64_t id, const std::string& name) = 0;
    virtual Result renameUser(const int64_t id, const std::string& name) = 0;
    virtual Result storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount) = 0;

    virtual Result getUser(User& user, const int64_t id) const = 0;
    virtual Result getUserLeaderboard(Leaderboard& lb, const int64_t id, const uint64_t before, const uint64_t after) const = 0;
    virtual Result getLeaderboard(Leaderboard& lb, const int64_t count) const = 0;
};
} // namespace db

#endif // DB_STORAGE_H