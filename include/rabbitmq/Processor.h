#ifndef MY_RABBIT_MQ_PROCESSOR_H
#define MY_RABBIT_MQ_PROCESSOR_H

#include <utility>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"

namespace rabbitmq
{

class Processor
{
private:
    enum class State : uint16_t
    {
        CREATED,
        STARTED,
        STOPPED,
    };
    static const char* stateToStr(const State s);

    volatile State m_state;
    logger::CategoryPtr m_logger;
    std::thread m_thread;

private:
    struct ProcessingItem
    {
        std::shared_ptr<AMQP::TcpChannel> m_channel;
        std::string m_message;
        std::string m_exchange;
        std::string m_routingkey;
        uint64_t m_deliveryTag;
        bool m_redelivered;
        ProcessingItem(
            std::shared_ptr<AMQP::TcpChannel> channel,
            std::string&& message,
            std::string&& exchange,
            std::string&& routingkey,
            const uint64_t deliveryTag,
            const bool redelivered):
            m_channel(channel),
            m_message(std::move(message)),
            m_exchange(std::move(exchange)),
            m_routingkey(std::move(routingkey)),
            m_deliveryTag(deliveryTag),
            m_redelivered(redelivered)
        {}
        ProcessingItem(const ProcessingItem&) = delete;
        ProcessingItem(ProcessingItem&&) = default;
        ProcessingItem& operator=(const ProcessingItem&) = delete;
        ProcessingItem& operator=(ProcessingItem&&) = default;
    };
    size_t m_queueCapacity;
    std::queue<ProcessingItem> m_processingQueue;
    std::mutex m_processingQueueGuard;
    std::condition_variable m_processingQueueCv;

private:
    Result processMessage(ProcessingItem&& item);
    void mainThreadFunc();

public:
    Processor(const size_t capacity = 100000);
    virtual ~Processor() = default;

    bool start();
    void stop();

    template<class... Args>
    Result pushMessage(Args... args);
};

template<class... Args>
Result Processor::pushMessage(Args... args)
{
    if (State::STARTED != m_state)
    {
        return Result::INVSTATE;
    }
    std::unique_lock<std::mutex> l(m_processingQueueGuard);

    if (m_processingQueue.size() >= m_queueCapacity)
    {
        return Result::QUEUE_OVERFLOW;
    }

    m_processingQueue.emplace(std::forward<Args>(args)...);

    m_processingQueueCv.notify_one();
    return Result::SUCCESS;
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_PROCESSOR_H
