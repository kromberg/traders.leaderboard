#ifndef DB_LOGIC_H
#define DB_LOGIC_H

#include <ctime>
#include <mutex>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>

#include "../logger/LoggerFwd.h"

#include "DBAbstractLogic.h"

namespace db
{
class InMemoryLogic : public AbstractLogic
{
private:
    struct User
    {
        std::string m_name;
        // todo: rework
        typedef std::time_t Time;
        typedef int64_t Score;
        typedef std::map<Time, Score> ScoreByDay;
        ScoreByDay m_scoreByDay;

        User(const std::string& name):
            m_name(name)
        {}

        void addScore(const Time t, const Score score);
        uint64_t getWeekScores(const Time currentTime) const;
    };
    typedef std::unordered_map<uint64_t, User> UsersMap;
    UsersMap m_usersMap;
    std::mutex m_usersMapGuard;
    typedef std::unordered_set<uint64_t> ConnectedUserIds;
    ConnectedUserIds m_connectedUserIds;
    std::mutex m_connectedUserIdsGuard;

    struct Leaderboard
    {
        typedef std::map<int64_t, uint64_t> ScoreToUser;
        typedef std::unordered_map<uint64_t, ScoreToUser::iterator> UserToScore;

        ScoreToUser m_scoreToUser;
        UserToScore m_userToScore;
    };
    Leaderboard m_leaderboard;
    std::mutex m_leaderboardGuard;

    logger::CategoryPtr m_logger;

private:
    void buildLeaderBoard();

public:
    InMemoryLogic();
    virtual ~InMemoryLogic() = default;

    // user_registered(id,name)
    virtual Result onUserRegistered(const uint64_t id, const std::string& name) override;
    // user_renamed(id,name)
    virtual Result onUserRenamed(const uint64_t id, const std::string& name) override;
    // user_deal(id,time,amount)
    virtual Result onUserDeal(const uint64_t id, const std::time_t t, const int64_t amount) override;
    // user_deal_won(id,time,amount)
    virtual Result onUserDealWon(const uint64_t id, const std::time_t t, const int64_t amount) override;
    // user_connected(id)
    virtual Result onUserConnected(const uint64_t id) override;
    // user_disconnected(id)
    virtual Result onUserDisconnected(const uint64_t id) override;
};
} // namespace db

#endif // DB_LOGIC_H