#ifndef MY_RABBIT_MQ_PUBLISHER_H
#define MY_RABBIT_MQ_PUBLISHER_H

#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Handler.h"
#include "Utils.h"

namespace rabbitmq
{

class Publisher : public Handler
{
private:
    size_t m_transactionMessagesCount = 0;

protected:
    virtual Result customConfigure(libconfig::Config& cfg) override;

public:
    Publisher(EventLoop& loop);
    virtual ~Publisher() = default;
    Publisher(const Publisher& c) = delete;
    Publisher(Publisher&& c) = default;
    Publisher& operator=(const Publisher& c) = delete;
    Publisher& operator=(Publisher&& c) = default;

    Result startTransactionSync();
    Result commitTransactionSync();

    size_t transactionMessagesCount() const;

    template<class... Args>
    bool publish(Args... args);
};

} // namespace rabbitmq

#include "PublisherImpl.hpp"

#endif // MY_RABBIT_MQ_PUBLISHER_H