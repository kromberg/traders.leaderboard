#ifndef MY_RABBIT_MQ_FWD_H
#define MY_RABBIT_MQ_FWD_H

#include <memory>

namespace rabbitmq
{
class EventLoop;
class TcpHandler;
class Handler;
typedef std::shared_ptr<Handler> HandlerPtr;
class Consumer;
class Publisher;
class Processor;
typedef std::shared_ptr<Processor> ProcessorPtr;

enum Result : uint16_t
{
    SUCCESS,
    FAILED,
    INVSTATE,
    INVFMT,
    QUEUE_OVERFLOW,
    NULL_CHANNEL,
    CMD_NOT_SUPPORTED,
    DB_ERROR,
};

const char* resultToStr(const Result r);

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_FWD_H