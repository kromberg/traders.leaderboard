#ifndef DB_ABSTRACT_LOGIC_H
#define DB_ABSTRACT_LOGIC_H

#include "Fwd.h"

namespace db
{
class AbstractLogic
{
private:

public:
    AbstractLogic() = default;
    virtual ~AbstractLogic() = default;

    // user_registered(id,name)
    virtual Result onUserRegistered(const uint64_t id, const std::string& name) = 0;
    // user_renamed(id,name)
    virtual Result onUserRenamed(const uint64_t id, const std::string& name) = 0;
    // user_deal(id,time,amount)
    virtual Result onUserDeal(const uint64_t id, const std::time_t t, const int64_t amount) = 0;
    // user_deal_won(id,time,amount)
    virtual Result onUserDealWon(const uint64_t id, const std::time_t t, const int64_t amount) = 0;
    // user_connected(id)
    virtual Result onUserConnected(const uint64_t id) = 0;
    // user_disconnected(id)
    virtual Result onUserDisconnected(const uint64_t id) = 0;
};
} // namespace db

#endif // DB_ABSTRACT_LOGIC_H