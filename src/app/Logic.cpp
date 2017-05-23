#include <libconfig.h++>

#include <common/Utils.h>
#include <logger/LoggerDefines.h>
#include <app/Logic.h>
#include <db/InMemoryStorage.h>
#include <db/MongodbStorage.h>

namespace app
{

Logic::Logic()
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
    Result res = m_storage->getLeaderboards(leaderboards, 10, 1, 1);
    if (Result::SUCCESS != res)
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

Result Logic::initialize()
{
    if (State::CREATED != m_state)
    {
        return Result::INVALID_STATE;
    }
    m_parser.registerCallbackObject(*this);
    m_state = State::INITIALIZED;
    return Result::SUCCESS;
}

Result Logic::configure(libconfig::Config& cfg)
{
    using namespace libconfig;

    if (State::INITIALIZED != m_state)
    {
        return Result::INVALID_STATE;
    }

    m_loopIntervalSeconds = 60;
    try
    {
        Setting& setting = cfg.lookup("application");
        if (!setting.lookupValue("loop_interval", m_loopIntervalSeconds))
        {
            LOG_WARN(m_logger, "Canont find 'loop_interval' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find 'application' section in configuration. Default values will be used");
    }
    LOG_INFO(m_logger, "Configuration parameters: <loop_interval: %d seconds>", m_loopIntervalSeconds);

    std::string storageTypeStr = "mongo";
    try
    {
        Setting& setting = cfg.lookup("db");
        if (!setting.lookupValue("type", storageTypeStr))
        {
            LOG_WARN(m_logger, "Canont find 'db.type' parameter in configuration. Default value will be used");
        }
    }
    catch (const SettingNotFoundException& e)
    {
        LOG_WARN(m_logger, "Canont find 'db' section in configuration. Default values will be used");
    }
    db::Storage::Type storageType = db::Storage::typeFromString(storageTypeStr);
    LOG_INFO(m_logger, "Configuration parameters: <db.type: %s -> %d(%s)>",
        storageTypeStr.c_str(), static_cast<int32_t>(storageType), db::Storage::typeToString(storageType));

    switch (storageType)
    {
        case db::Storage::Type::IN_MEMORY:
            m_storage.reset(new db::InMemoryStorage());
            break;
        case db::Storage::Type::MONGODB:
            m_storage.reset(new db::MongodbStorage());
            break;
        case db::Storage::Type::UNKNOWN:
        default:
            break;
    }
    if (!m_storage)
    {
        LOG_ERROR(m_logger, "Cannot create storage");
        return Result::STORAGE_ERROR;
    }
    Result res = m_storage->configure(cfg);
    if (Result::SUCCESS != res)
    {
        return res;
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

    Result res = m_storage->start();
    if (Result::SUCCESS != res)
    {
        return res;
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
    Result res = m_storage->storeUser(id, name);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_renamed(id,name)
Result Logic::onUserRenamed(const int64_t id, const std::string& name)
{
    Result res = m_storage->renameUser(id, name);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal(id,time,amount)
Result Logic::onUserDeal(const int64_t id, const std::time_t t, const int64_t amount)
{
    Result res = m_storage->storeUserDeal(id, t, amount);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal_won(id,time,amount)
Result Logic::onUserDealWon(const int64_t id, const std::time_t t, const int64_t amount)
{
    Result res = m_storage->storeUserDeal(id, t, amount);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_connected(id)
Result Logic::onUserConnected(const int64_t id)
{
    Result res = m_storage->storeConnectedUser(id);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_disconnected(id)
Result Logic::onUserDisconnected(const int64_t id)
{
    Result res = m_storage->removeConnectedUser(id);
    if (Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}

Result Logic::processMessage(rabbitmq::ProcessingItem&& item)
{
    return m_parser.parseMessage(std::move(item));
}

} // namespace app