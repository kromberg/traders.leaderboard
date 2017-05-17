#ifndef MY_APP_H
#define MY_APP_H

#include <ctime>
#include <string>

#include "../logger/LoggerFwd.h"
#include "../db/Fwd.h"
#include "Utils.h"

namespace app
{

class Logic
{
private:
    db::StoragePtr m_storage;

    logger::CategoryPtr m_logger;

public:
    Logic();
    virtual ~Logic();

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