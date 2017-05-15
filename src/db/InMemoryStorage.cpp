#include <queue>


#include <logger/LoggerDefines.h>
#include <db/InMemoryStorage.h>

namespace db
{

int64_t InMemoryStorage::UserStorage::getWeekScores(const Time currentTime) const
{
    int64_t res = 0;

    struct tm * lastWeekTm;
    lastWeekTm = std::localtime(&currentTime);
    lastWeekTm->tm_mday -= 7;
    time_t lastWeek = std::mktime(lastWeekTm);
    auto it = m_scores.lower_bound(lastWeek);
    for (; it != m_scores.end(); ++it)
    {
        res += it->second;
    }
    return res;
}

InMemoryStorage::InMemoryStorage()
{
    m_logger = logger::Logger::getLogCategory("DB_IN_MEM");
}

Result InMemoryStorage::storeUser(const uint64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() != it)
    {
        if (it->second.m_name == name)
        {
            l.unlock();
            LOG_ERROR(m_logger, "Cannot register user with the same name <id: %lu, name: %s>",
                id, name.c_str());
            return Result::USER_ALREADY_REG;
        }
        else
        {
            LOG_ERROR(m_logger, "Cannot register user with different names <id: %lu, name: %s, new name: %s>",
                id, it->second.m_name.c_str(), name.c_str());
            l.unlock();
            return Result::USER_ALREADY_REG;
        }
    }
    m_users.insert(std::make_pair(id, name));
    l.unlock();
    LOG_DEBUG(m_logger, "User was registered <id: %lu, name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result InMemoryStorage::renameUser(const uint64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot rename user <id: %lu, name: %s>. It is not found",
            id, name.c_str());
        return Result::USER_NOT_FOUND;
    }

    it->second.m_name = name;
    l.unlock();
    LOG_DEBUG(m_logger, "User was renamed <id: %lu new name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result InMemoryStorage::storeUserDeal(const uint64_t id, const std::time_t t, const int64_t amount)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot store user deal <id: %lu, time: %lu, amount: %ld>. User is not found",
            id, static_cast<uint64_t>(t), amount);
        return Result::USER_NOT_FOUND;
    }

    auto scoreIt = it->second.m_scores.find(t);
    if (it->second.m_scores.end() == scoreIt)
    {
        it->second.m_scores.insert(std::make_pair(t, amount));
    }
    else
    {
        scoreIt->second += amount;
    }
    l.unlock();

    LOG_DEBUG(m_logger, "User deal was stored <id: %lu, time: %lu, amount: %ld>",
        id, static_cast<uint64_t>(t), amount);

    return Result::SUCCESS;
}

Result InMemoryStorage::getUser(User& user, const uint64_t id) const
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot find user <id: %lu>", id);
        return Result::USER_NOT_FOUND;
    }

    user.m_id = id;
    user.m_name = it->second.m_name;
    return Result::SUCCESS;
}

Result InMemoryStorage::getLeaderboard(
    Leaderboard& lb,
    const uint64_t id,
    const uint64_t before,
    const uint64_t after)
        const
{
    

    return Result::SUCCESS;
}

Result InMemoryStorage::getLeaderboard(
    Leaderboard& lb,
    const int64_t count)
        const
{
    lb.clear();

    time_t currentTime = time(nullptr);
    if (count <= 0)
    {
        std::unique_lock<std::mutex> l(m_usersMapGuard);
        for (const auto& user : m_users)
        {
            lb.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(user.second.getWeekScores(currentTime)),
                std::forward_as_tuple(user.first, user.second.m_name));
        }
    }
    else
    {
        struct UserScore
        {
            int64_t m_score;
            UsersStorage::const_iterator m_userIt;
            UserScore(const int64_t score, UsersStorage::const_iterator userIt):
                m_score(score), m_userIt(userIt)
            {}
        };
        struct UserScoreComp
        {
            bool operator() (const UserScore& lhs, const UserScore& rhs)
            {
                return lhs.m_score > rhs.m_score;
            }
        };

        std::vector<UserScore> userScores;
        userScores.reserve(count);
        std::priority_queue<UserScore, std::vector<UserScore>, UserScoreComp> userScoresQueue(UserScoreComp(), userScores);

        std::unique_lock<std::mutex> l(m_usersMapGuard);
        for (auto userIt = m_users.begin(); userIt != m_users.end(); ++ userIt)
        {
            if (userScoresQueue.size() < count)
            {
                userScoresQueue.emplace(userIt->second.getWeekScores(currentTime), userIt);
            }
            else
            {
                int64_t score = userIt->second.getWeekScores(currentTime);
                if (userScoresQueue.top().m_score < score)
                {
                    userScoresQueue.pop();
                    userScoresQueue.emplace(score, userIt);
                }
            }
        }
        l.unlock();

        for (auto userScore : userScores)
        {
            lb.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(userScore.m_score),
                std::forward_as_tuple(userScore.m_userIt->first, userScore.m_userIt->second.m_name));
        }
    }

    return Result::SUCCESS;
}


} // namespace db