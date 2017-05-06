#ifndef MY_RABBIT_MQ_FWD_H
#define MY_RABBIT_MQ_FWD_H

#include <memory>

namespace rabbitmq
{
class EventLoop;
class TcpHandler;
class Consumer;
class Processor;
typedef std::shared_ptr<Processor> ProcessorPtr;

enum Result : uint16_t
{
    SUCCESS,
    FAILED,
    INVSTATE,
    QUEUE_OVERFLOW,
};

const char* resultToStr(const Result r);

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_FWD_H