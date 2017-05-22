#ifndef MY_RABBIT_MQ_CONSUMER_IMPL_HPP
#define MY_RABBIT_MQ_CONSUMER_IMPL_HPP

#include <functional>

namespace rabbitmq
{
inline Consumer::Consumer(EventLoop& loop):
    Handler(loop)
{
    m_logger = logger::Logger::getLogCategory("RMQ_CONSUMER");
}

inline void Consumer::attachProcessor(ProcessorPtr& processor)
{
    atomic_store(&m_processor, processor);
}

template<class... Args>
inline Result Consumer::consume(Args... args)
{
    using namespace std::placeholders;

    SyncObj<Result> result;
    channel().consume(std::forward<Args>(args)...)
        .onReceived(std::bind(&Consumer::onMessageCallback, this, _1, _2, _3))
        .onSuccess([&] (const std::string& tag) -> void
            {
                LOG_INFO(m_logger, "Started consuming under tag: %s", tag.c_str());
                result.set(Result::SUCCESS);
            })
        .onError([&] (const std::string& tag) -> void
            {
                LOG_ERROR(m_logger, "Cannot start consuming");
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_CONSUMER_IMPL_HPP