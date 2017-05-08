#include <logger/LoggerDefines.h>
#include <db/DBInMemoryLogic.h>

namespace db
{

void InMemoryLogic::User::addScore(const Time t, const Score score)
{
    // todo: day alignment
    m_scoreByDay.emplace(t, score);
}

uint64_t InMemoryLogic::User::getWeekScores(const Time currentTime) const
{
    uint64_t res = 0;

    struct tm * lastWeekTm;
    lastWeekTm = std::localtime(&currentTime);
    lastWeekTm->tm_mday -= 7;
    time_t lastWeek = std::mktime(lastWeekTm);
    auto it = m_scoreByDay.lower_bound(lastWeek);
    for (; it != m_scoreByDay.end(); ++it)
    {
        res += it->second;
    }
    return res;
}

void InMemoryLogic::buildLeaderBoard()
{
    using std::swap;

    time_t currentTime = time(nullptr);
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    Leaderboard tmpLeaderboard;
    for (const auto& user : m_usersMap)
    {
        uint64_t userScore = user.second.getWeekScores(currentTime);
        auto res = tmpLeaderboard.m_scoreToUser.emplace(userScore, user.first);
        tmpLeaderboard.m_userToScore.emplace(user.first, res.first);
    }
    {
        std::unique_lock<std::mutex> l(m_leaderboardGuard);
        swap(tmpLeaderboard, m_leaderboard);
    }
}

// user_registered(id,name)
Result InMemoryLogic::onUserRegistered(const uint64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_usersMap.find(id);
    if (m_usersMap.end() != it)
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
    m_usersMap.emplace(id, name);
    l.unlock();
    LOG_DEBUG(m_logger, "User was registered <id: %lu, name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}
// user_renamed(id,name)
Result InMemoryLogic::onUserRenamed(const uint64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_usersMap.find(id);
    if (m_usersMap.end() == it)
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
// user_deal(id,time,amount)
Result InMemoryLogic::onUserDeal(const uint64_t id, const std::time_t t, const int64_t amount)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_usersMap.find(id);
    if (m_usersMap.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot add user deal <id: %lu, time: %lu, amount: %ld>. It is not found",
            id, static_cast<uint64_t>(t), amount);
        return Result::USER_NOT_FOUND;
    }

    auto scoreIt = it->second.m_scoreByDay.find(t);
    if (it->second.m_scoreByDay.end() == scoreIt)
    {
        it->second.m_scoreByDay.emplace(t, amount);
    }
    else
    {
        scoreIt->second += amount;
    }
    l.unlock();

    LOG_DEBUG(m_logger, "User deal was done <id: %lu, time: %lu, amount: %ld>",
        id, static_cast<uint64_t>(t), amount);

    return Result::SUCCESS;
}
// user_deal_won(id,time,amount)
Result InMemoryLogic::onUserDealWon(const uint64_t id, const std::time_t t, const int64_t amount)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_usersMap.find(id);
    if (m_usersMap.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot add user deal <id: %lu, time: %lu, amount: %ld>. It is not found",
            id, static_cast<uint64_t>(t), amount);
        return Result::USER_NOT_FOUND;
    }

    auto scoreIt = it->second.m_scoreByDay.find(t);
    if (it->second.m_scoreByDay.end() == scoreIt)
    {
        it->second.m_scoreByDay.emplace(t, amount);
    }
    else
    {
        scoreIt->second += amount;
    }
    l.unlock();

    LOG_DEBUG(m_logger, "User deal was done <id: %lu, time: %lu, amount: %ld>",
        id, static_cast<uint64_t>(t), amount);

    return Result::SUCCESS;
}
// user_connected(id)
Result InMemoryLogic::onUserConnected(const uint64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_usersMapGuard);
        auto it = m_usersMap.find(id);
        if (m_usersMap.end() == it)
        {
            l.unlock();
            LOG_ERROR(m_logger, "Cannot connect user <id: %lu>. It is not found", id);
            return Result::USER_NOT_FOUND;
        }
    }

    {
        std::unique_lock<std::mutex> l(m_connectedUserIdsGuard);
        // we do not care about insertion result
        m_connectedUserIds.insert(id);
    }

    buildLeaderBoard();
    {
        std::unique_lock<std::mutex> l(m_leaderboardGuard);
        for (auto& userScore : m_leaderboard.m_scoreToUser)
        {
            LOG_DEBUG(m_logger, "Score [%ld]: User [%lu]", userScore.first, userScore.second);
        }
    }

    LOG_DEBUG(m_logger, "User was connected <id: %lu>", id);

    return Result::SUCCESS;
}
// user_disconnected(id)
Result InMemoryLogic::onUserDisconnected(const uint64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_connectedUserIdsGuard);
        // we do not care about insertion result
        m_connectedUserIds.erase(id);
    }

    LOG_DEBUG(m_logger, "User was disconnected <id: %lu>", id);

    return Result::SUCCESS;
}

} // namespace db