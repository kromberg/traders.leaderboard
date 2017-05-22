#ifndef MY_RABBIT_MQ_CONSUMER_H
#define MY_RABBIT_MQ_CONSUMER_H

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"
#include "Handler.h"

namespace rabbitmq
{

class Consumer : public Handler
{
protected:
    ProcessorPtr m_processor;

protected:
    void onMessageCallback(const AMQP::Message &message, uint64_t deliveryTag, bool redelivered);

public:
    Consumer(EventLoop& loop);
    virtual ~Consumer() = default;
    Consumer(const Consumer& c) = delete;
    Consumer(Consumer&& c) = default;
    Consumer& operator=(const Consumer& c) = delete;
    Consumer& operator=(Consumer&& c) = default;

    void attachProcessor(ProcessorPtr& processor);

    template<class... Args>
    Result consume(Args... args);
};

} // namespace rabbitmq

#include "ConsumerImpl.hpp"

#endif // MY_RABBIT_MQ_CONSUMER_H