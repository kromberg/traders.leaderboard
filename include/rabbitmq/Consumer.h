#ifndef MY_RABBIT_MQ_CONSUMER_H
#define MY_RABBIT_MQ_CONSUMER_H

#include <functional>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"
#include "ProcessingItem.h"

#include "Fwd.h"
#include "Handler.h"

namespace rabbitmq
{

class Consumer : public Handler
{
protected:
    typedef std::function<Result(rabbitmq::ProcessingItem&&)> MessageProcessingCallback;
    MessageProcessingCallback m_messageProcessingCallback;

protected:
    void onMessageCallback(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered);

protected:
    virtual Result customConfigure(libconfig::Config& cfg) override;
    virtual Result customStart() override;

public:
    Consumer(EventLoop& loop);
    virtual ~Consumer() = default;
    Consumer(const Consumer& c) = delete;
    Consumer(Consumer&& c) = default;
    Consumer& operator=(const Consumer& c) = delete;
    Consumer& operator=(Consumer&& c) = default;

    void registerCallback(MessageProcessingCallback&& messageProcessingCallback);

    template<class... Args>
    Result consume(Args... args);
};

} // namespace rabbitmq

#include "ConsumerImpl.hpp"

#endif // MY_RABBIT_MQ_CONSUMER_H