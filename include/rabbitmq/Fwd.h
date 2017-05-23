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
typedef std::shared_ptr<Consumer> ConsumerPtr;
class Publisher;
typedef std::shared_ptr<Publisher> PublisherPtr;
class Processor;
typedef std::shared_ptr<Processor> ProcessorPtr;

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_FWD_H