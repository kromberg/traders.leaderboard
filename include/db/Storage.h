#ifndef DB_STORAGE_H
#define DB_STORAGE_H

#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "../common/Types.h"
#include "Fwd.h"

namespace libconfig
{
class Config;
} // namespace libconfig

namespace db
{
using common::State;
using common::Result;

struct User
{
    int64_t m_id;
    std::string m_name;

    User():
        m_id(-1), m_name("unknown")
    {}

    User(const int64_t id, const std::string& name):
        m_id(id), m_name(name)
    {}

    bool operator<(const User& u) const
    {
        return m_id < u.m_id;
    }

    bool operator<(const int64_t id) const
    {
        return m_id < id;
    }
};

// score to user
typedef std::multimap<int64_t, User, std::greater<int64_t> > Leaderboard;
// user id to leaderboard
typedef std::map<User, Leaderboard> Leaderboards;

class Storage
{
public:
    enum class Type
    {
        IN_MEMORY,
        MONGODB,
        UNKNOWN,
    };
    static Type typeFromString(const std::string& typeStr);
    static const char* typeToString(const Type t);

public:
    Storage() = default;
    virtual ~Storage() = default;

    virtual Result configure(const libconfig::Config& cfg) = 0;
    virtual Result start() = 0;

    virtual Result storeUser(const int64_t id, const std::string& name) = 0;
    virtual Result renameUser(const int64_t id, const std::string& name) = 0;
    virtual Result storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount) = 0;

    virtual Result storeConnectedUser(const int64_t id) = 0;
    virtual Result removeConnectedUser(const int64_t id) = 0;

    virtual Result getUser(User& user, const int64_t id) const = 0;

    virtual Result getLeaderboards(
        Leaderboards& leaderboards,
        const int64_t count = -1,
        const uint64_t before = 10,
        const uint64_t after = 10) const = 0;
};

inline Storage::Type Storage::typeFromString(const std::string& tmpTypeStr)
{
    static const std::unordered_map<std::string, Type> stringToTypeMap =
    {
        {"inmemory",    Type::IN_MEMORY},
        {"in_memory",   Type::IN_MEMORY},
        {"in-memory",   Type::IN_MEMORY},
        {"mongo",       Type::MONGODB},
        {"mongodb",     Type::MONGODB},
        {"mongo_db",    Type::MONGODB},
        {"mongo-db",    Type::MONGODB},
    };
    std::string typeStr;
    std::transform(tmpTypeStr.begin(), tmpTypeStr.end(), std::back_inserter(typeStr), ::tolower);

    auto it = stringToTypeMap.find(typeStr);
    if (stringToTypeMap.end() == it)
    {
        return Type::UNKNOWN;
    }
    return it->second;
}

inline const char* Storage::typeToString(const Type t)
{
    switch (t)
    {
        case Type::IN_MEMORY:
            return "IN MEMORY";
        case Type::MONGODB:
            return "MONGODB";
        case Type::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}
} // namespace db

#endif // DB_STORAGE_H