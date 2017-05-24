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

Result InMemoryStorage::configure(libconfig::Config& cfg)
{
    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot configure storage in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    m_state = State::CONFIGURED;
    return Result::SUCCESS;
}

Result InMemoryStorage::start()
{
    if (State::CONFIGURED != m_state)
    {
        LOG_ERROR(m_logger, "Cannot start storage in state %d(%s)",
            static_cast<int32_t>(m_state), common::stateToStr(m_state));
        return Result::INVALID_STATE;
    }

    m_state = State::STARTED;
    return Result::SUCCESS;
}

Result InMemoryStorage::storeUser(const int64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() != it)
    {
        if (it->second.m_name == name)
        {
            l.unlock();
            LOG_ERROR(m_logger, "Cannot register user with the same name <id: %ld, name: %s>",
                id, name.c_str());
            return Result::USER_ALREADY_REG;
        }
        else
        {
            LOG_ERROR(m_logger, "Cannot register user with different names <id: %ld, name: %s, new name: %s>",
                id, it->second.m_name.c_str(), name.c_str());
            l.unlock();
            return Result::USER_ALREADY_REG;
        }
    }
    m_users.insert(std::make_pair(id, name));
    l.unlock();
    LOG_DEBUG(m_logger, "User was registered <id: %ld, name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result InMemoryStorage::renameUser(const int64_t id, const std::string& name)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot rename user <id: %ld, name: %s>. User is not found",
            id, name.c_str());
        return Result::USER_NOT_FOUND;
    }

    it->second.m_name = name;
    l.unlock();
    LOG_DEBUG(m_logger, "User was renamed <id: %ld new name: %s>",
        id, name.c_str());
    return Result::SUCCESS;
}

Result InMemoryStorage::storeUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot store user deal <id: %ld, time: %s, amount: %ld>. User is not found",
            id, common::timeToString(t).c_str(), amount);
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

    LOG_DEBUG(m_logger, "User deal was stored <id: %ld, time: %s, amount: %ld>",
        id, common::timeToString(t).c_str(), amount);

    return Result::SUCCESS;
}

Result InMemoryStorage::storeConnectedUser(const int64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_connectedUsersGuard);
        m_connectedUsers.emplace(id);
    }

    LOG_DEBUG(m_logger, "Connected user was stored <id: %ld>", id);

    return Result::SUCCESS;
}

Result InMemoryStorage::removeConnectedUser(const int64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_connectedUsersGuard);
        auto it = m_connectedUsers.find(id);
        if (m_connectedUsers.end() == it)
        {
            LOG_ERROR(m_logger, "Cannot remove connected user <id: %ld>. User is not found", id);
            return Result::USER_NOT_FOUND;
        }
        m_connectedUsers.erase(it);
    }

    LOG_DEBUG(m_logger, "Connected user was removed <id: %ld>", id);

    return Result::SUCCESS;
}

Result InMemoryStorage::getUser(User& user, const int64_t id) const
{
    std::unique_lock<std::mutex> l(m_usersMapGuard);
    auto it = m_users.find(id);
    if (m_users.end() == it)
    {
        l.unlock();
        LOG_ERROR(m_logger, "Cannot find user <id: %ld>", id);
        return Result::USER_NOT_FOUND;
    }

    user.m_id = id;
    user.m_name = it->second.m_name;
    return Result::SUCCESS;
}

Result InMemoryStorage::getLeaderboards(
    Leaderboards& leaderboards,
    const int64_t count,
    const uint64_t before,
    const uint64_t after)
        const
{
    time_t currentTime = time(nullptr);

    std::unordered_set<int64_t> connectedUsers;
    {
        std::unique_lock<std::mutex> l(m_connectedUsersGuard);
        connectedUsers = m_connectedUsers;
    }

    Leaderboard tmpLeaderboard;
    {
        std::unique_lock<std::mutex> l(m_usersMapGuard);
        for (const auto& user : m_users)
        {
            tmpLeaderboard.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(user.second.getWeekScores(currentTime)),
                std::forward_as_tuple(user.first, user.second.m_name));
        }
    }

    if (count <= 0)
    {
        leaderboards.emplace(-1, std::move(tmpLeaderboard));
    }
    else
    {
        auto begin = tmpLeaderboard.begin();
        auto end = tmpLeaderboard.begin();
        std::advance(end, std::min(static_cast<int64_t>(tmpLeaderboard.size()), count));

        leaderboards.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(-1),
            std::forward_as_tuple(begin, end));
    }

    if (connectedUsers.empty())
    {
        return Result::SUCCESS;
    }

    Leaderboard currentLeaderboard;
    std::unordered_map<int64_t, uint16_t> currentIdsToCount;
    for (auto&& lbItem : tmpLeaderboard)
    {
        currentLeaderboard.emplace(lbItem);

        auto currentIdIt = currentIdsToCount.begin();
        while (currentIdIt != currentIdsToCount.end())
        {
            auto lbIt = leaderboards.find(currentIdIt->first);
            if (leaderboards.end() == lbIt)
            {
                LOG_ERROR(m_logger, "Cannot find leaderboard for user %ld", lbIt->first);
                ++ currentIdIt;
                continue;
            }
            lbIt->second.emplace(lbItem);

            if (currentIdIt->second + 1 >= after)
            {
                currentIdIt = currentIdsToCount.erase(currentIdIt);
            }
            else
            {
                ++ currentIdIt->second;
                ++ currentIdIt;
            }
        }

        auto idIt = connectedUsers.find(lbItem.second.m_id);
        if (connectedUsers.end() != idIt)
        {
            LOG_DEBUG(m_logger, "User %ld found: adding leaderboard", *idIt);
            currentIdsToCount.emplace(*idIt, 0);
            leaderboards.emplace(*idIt, currentLeaderboard);
        }

        if (currentLeaderboard.size() > before)
        {
            currentLeaderboard.erase(currentLeaderboard.begin());
        }
    }

    return Result::SUCCESS;
}

} // namespace db