#ifndef MY_APP_H
#define MY_APP_H

#include <ctime>
#include <string>
#include <unordered_set>
#include <mutex>
#include <thread>

#include "../common/Types.h"
#include "../logger/LoggerFwd.h"
#include "../db/Fwd.h"
#include "Utils.h"

namespace app
{
using common::State;
using common::Result;

class Logic
{
private:
    logger::CategoryPtr m_logger;
    State m_state = State::CREATED;
    int32_t m_loopIntervalSeconds = 5;
    volatile bool m_loopIsRunning = false;
    std::thread m_loopThread;

    db::StoragePtr m_storage;
    std::unordered_set<int64_t> m_connectedUsers;
    std::mutex m_connectedUsersGuard;

private:
    void loop();
    void loopFunc();

public:
    Logic();
    virtual ~Logic();

    Result configure();
    Result start();
    void stop();

    // user_registered(id,name)
    virtual Result onUserRegistered(const int64_t id, const std::string& name);
    // user_renamed(id,name)
    virtual Result onUserRenamed(const int64_t id, const std::string& name);
    // user_deal(id,time,amount)
    virtual Result onUserDeal(const int64_t id, const std::time_t t, const int64_t amount);
    // user_deal_won(id,time,amount)
    virtual Result onUserDealWon(const int64_t id, const std::time_t t, const int64_t amount);
    // user_connected(id)
    virtual Result onUserConnected(const int64_t id);
    // user_disconnected(id)
    virtual Result onUserDisconnected(const int64_t id);
};
} // namespace app

#endif // MY_APP_H