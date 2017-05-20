#include <common/Utils.h>
#include <logger/LoggerDefines.h>
#include <app/Logic.h>
#include <db/InMemoryStorage.h>
#include <db/MongodbStorage.h>

namespace app
{

Logic::Logic():
    m_storage(new db::MongodbStorage())
{
    m_logger = logger::Logger::getLogCategory("APP_LOGIC");
}

Logic::~Logic()
{
    stop();
}

void Logic::loop()
{
    using std::chrono::system_clock;

    static constexpr uint32_t maxSleepSeconds = 60;
    while (m_loopIsRunning)
    {
        time_t startTime = time(nullptr);
        LOG_INFO(m_logger, "Logic loop was started at %s", common::timeToString(startTime).c_str());
        std::unordered_set<int64_t> connectedUsers;
        {
            std::unique_lock<std::mutex> l(m_connectedUsersGuard);
            connectedUsers = m_connectedUsers;
        }

        loopFunc();

        time_t endTime = time(nullptr);

        uint32_t diffSeconds = static_cast<uint32_t>(difftime(endTime, startTime));
        LOG_INFO(m_logger, "Logic loop was ended at %s. Duration: %u seconds",
            common::timeToString(endTime).c_str(), diffSeconds);

        while (m_loopIsRunning && diffSeconds < m_loopIntervalSeconds)
        {
            uint32_t sleepSeconds = std::min(maxSleepSeconds, m_loopIntervalSeconds - diffSeconds);
            LOG_DEBUG(m_logger, "Logic loop will sleep for %u seconds", sleepSeconds);
            std::this_thread::sleep_for(std::chrono::seconds(sleepSeconds));
            endTime = time(nullptr);
            diffSeconds = static_cast<uint32_t>(difftime(endTime, startTime));
        }
    }
}

void Logic::loopFunc()
{
    db::UserLeaderboards leaderboards;
    db::Result res = m_storage->getLeaderboards(leaderboards, m_connectedUsers, 10, 1, 1);
    if (db::Result::SUCCESS != res)
    {
        LOG_WARN(m_logger, "Cannot get leaderboard");
        return;
    }

    LOG_DEBUG(m_logger, "Leaderboard:");
    for (auto&& scoreUser : leaderboards[-1])
    {
        LOG_DEBUG(m_logger, "\t%015ld -> <%ld, %s>",
            scoreUser.first, scoreUser.second.m_id, scoreUser.second.m_name.c_str());
    }

    for (auto&& userLb : leaderboards)
    {
        if (-1 == userLb.first)
        {
            continue;
        }
        LOG_DEBUG(m_logger, "User %ld leaderboard:", userLb.first);
        for (auto&& scoreUser : userLb.second)
        {
            LOG_DEBUG(m_logger, "\t%015ld -> <%ld, %s>",
                scoreUser.first, scoreUser.second.m_id, scoreUser.second.m_name.c_str());
        }
    }
}


Result Logic::configure()
{
    if (State::CREATED != m_state)
    {
        return Result::INVALID_STATE;
    }
    m_state = State::CONFIGURED;
    return Result::SUCCESS;
}

Result Logic::start()
{
    using std::swap;

    if (State::CONFIGURED != m_state)
    {
        return Result::INVALID_STATE;
    }
    m_loopIsRunning = true;
    std::thread loopThread(&Logic::loop, this);
    swap(loopThread, m_loopThread);
    m_state = State::STARTED;
    return Result::SUCCESS;
}

void Logic::stop()
{
    if (State::STARTED != m_state)
    {
        return ;
    }
    m_loopIsRunning = false;
    m_loopThread.join();
    m_state = State::STOPPED;
}

// user_registered(id,name)
Result Logic::onUserRegistered(const int64_t id, const std::string& name)
{
    db::Result res = m_storage->storeUser(id, name);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_renamed(id,name)
Result Logic::onUserRenamed(const int64_t id, const std::string& name)
{
    db::Result res = m_storage->renameUser(id, name);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal(id,time,amount)
Result Logic::onUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
{
    db::Result res = m_storage->storeUserDeal(id, t, amount);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal_won(id,time,amount)
Result Logic::onUserDealWon(const int64_t id, const std::time_t t, const int64_t amount)
{
    db::Result res = m_storage->storeUserDeal(id, t, amount);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_connected(id)
Result Logic::onUserConnected(const int64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_connectedUsersGuard);
        m_connectedUsers.emplace(id);
    }

    LOG_DEBUG(m_logger, "User was connected <id: %ld>", id);

    return Result::SUCCESS;
}
// user_disconnected(id)
Result Logic::onUserDisconnected(const int64_t id)
{
    {
        std::unique_lock<std::mutex> l(m_connectedUsersGuard);
        m_connectedUsers.erase(id);
    }

    LOG_DEBUG(m_logger, "User was disconnected <id: %ld>", id);

    return Result::SUCCESS;
}

} // namespace app