#include <logger/LoggerDefines.h>
#include <app/Logic.h>
#include <db/InMemoryStorage.h>

namespace app
{

Logic::Logic():
    m_storage(new db::InMemoryStorage())
{
    m_logger = logger::Logger::getLogCategory("APP_LOGIC");
}

Logic::~Logic()
{}

// user_registered(id,name)
Result Logic::onUserRegistered(const uint64_t id, const std::string& name)
{
    db::Result res = m_storage->storeUser(id, name);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_renamed(id,name)
Result Logic::onUserRenamed(const uint64_t id, const std::string& name)
{
    db::Result res = m_storage->renameUser(id, name);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal(id,time,amount)
Result Logic::onUserDeal(const uint64_t id, const std::time_t t, const int64_t amount)
{
    db::Result res = m_storage->storeUserDeal(id, t, amount);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_deal_won(id,time,amount)
Result Logic::onUserDealWon(const uint64_t id, const std::time_t t, const int64_t amount)
{
    db::Result res = m_storage->storeUserDeal(id, t, amount);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }
    return Result::SUCCESS;
}
// user_connected(id)
Result Logic::onUserConnected(const uint64_t id)
{
    db::Leaderboard lb;
    db::Result res = m_storage->getLeaderboard(lb, 10);
    if (db::Result::SUCCESS != res)
    {
        return Result::FAILED;
    }

    for (auto& scoreUser : lb)
    {
        LOG_DEBUG(m_logger, "%15ld -> <%lu, %s>",
            scoreUser.first, scoreUser.second.m_id, scoreUser.second.m_name.c_str());
    }

    LOG_DEBUG(m_logger, "User was connected <id: %lu>", id);

    return Result::SUCCESS;
}
// user_disconnected(id)
Result Logic::onUserDisconnected(const uint64_t id)
{
    LOG_DEBUG(m_logger, "User was disconnected <id: %lu>", id);

    return Result::SUCCESS;
}

} // namespace app