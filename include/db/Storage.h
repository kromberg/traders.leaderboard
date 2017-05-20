#ifndef DB_STORAGE_H
#define DB_STORAGE_H

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>

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
// user id to leaderboard
typedef std::unordered_map<int64_t, Leaderboard> UserLeaderboards;
// user id to leaderboard
typedef std::unordered_map<int64_t, Leaderboard> Leaderboards;

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
    virtual Result getLeaderboard(Leaderboard& lb, const int64_t count = -1) const = 0;
    virtual Result getUsersLeaderboards(UserLeaderboards& lb, const std::unordered_set<int64_t>& ids, const uint64_t before = 10, const uint64_t after = 10) const = 0;

    virtual Result getLeaderboards(
        Leaderboards& leaderboards,
        const std::unordered_set<int64_t>& ids,
        const int64_t count = -1,
        const uint64_t before = 10,
        const uint64_t after = 10) const = 0;
};
} // namespace db

#endif // DB_STORAGE_H