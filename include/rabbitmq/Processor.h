#ifndef MY_RABBIT_MQ_PROCESSOR_H
#define MY_RABBIT_MQ_PROCESSOR_H

#include <utility>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"
#include "../common/Types.h"

#include "Fwd.h"
#include "ProcessingItem.h"
#include "CommandsDispatcher.h"

namespace rabbitmq
{
using common::State;
using common::Result;

class Processor
{
private:
    static const char* stateToStr(const State s);

    volatile State m_state;
    logger::CategoryPtr m_logger;
    std::thread m_thread;
    Dispatcher m_dispatcher;

private:
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
        return Result::INVALID_STATE;
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
