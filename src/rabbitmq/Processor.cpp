#include <logger/Logger.h>
#include <logger/LoggerDefines.h>

#include <rabbitmq/CommandsDispatcher.h>
#include <rabbitmq/Processor.h>

namespace rabbitmq
{
const char* Processor::stateToStr(const State s)
{
    switch (s)
    {
        case State::CREATED:
            return "CREATED";
        case State::STARTED:
            return "STARTED";
        case State::STOPPED:
            return "STOPPED";
    }
    return "UNKNOWN";
}

Processor::Processor(const size_t queueCapacity):
    m_state(State::CREATED),
    m_queueCapacity(queueCapacity)
{
    m_logger = logger::Logger::getLogCategory("RMQ_PROCESSOR");
}

bool Processor::start()
{
    if (State::CREATED != m_state)
    {
        LOG_ERROR(m_logger, "Could not start processor in state %u(%s)",
            static_cast<uint16_t>(m_state), stateToStr(m_state));
        return false;
    }
    LOG_INFO(m_logger, "Starting processor");
    m_state = State::STARTED;

    std::thread t(std::bind(&Processor::mainThreadFunc, this));
    std::swap(t, m_thread);

    LOG_INFO(m_logger, "Processor is started");

    return true;
}

void Processor::stop()
{
    if (State::STARTED != m_state)
    {
        LOG_ERROR(m_logger, "Could not stop processor in state %u(%s)",
            static_cast<uint16_t>(m_state), stateToStr(m_state));
        return ;
    }
    LOG_INFO(m_logger, "Stopping processor");
    m_state = State::STOPPED;
    m_processingQueueCv.notify_one();
    m_thread.join();
    LOG_INFO(m_logger, "Processor is stopped");
}

Result Processor::processMessage(ProcessingItem&& item)
{
    LOG_DEBUG(m_logger, "Start processing message: <Channel: %p, Body: %s. Exchange: %s. Routing key :%s."
        "Delivery tag: %lu. Redelivered: %s>",
        item.m_channel.get(), item.m_message.c_str(),
        item.m_exchange.c_str(), item.m_routingkey.c_str(),
        item.m_deliveryTag, (item.m_redelivered ? "true" : "false"));

    if (!item.m_channel)
    {
        LOG_ERROR(m_logger, "Cannot process message with NULL channel");
        return Result::NULL_CHANNEL;
    }

    Result r = Dispatcher::processMessage(m_logger, std::move(item));
    if (Result::SUCCESS != r)
    {
        LOG_ERROR(m_logger, "Cannot process message. Result: %u(%s)",
            static_cast<uint16_t>(r), resultToStr(r));
        return r;
    }

    // acknowledge the message
    item.m_channel->ack(item.m_deliveryTag);
    return Result::SUCCESS;
}

void Processor::mainThreadFunc()
{
    LOG_INFO(m_logger, "Processor thread is started");
    while (State::STARTED == m_state)
    {
        std::unique_lock<std::mutex> l(m_processingQueueGuard);
        if (m_processingQueue.empty())
        {
            m_processingQueueCv.wait(l);
        }

        LOG_DEBUG(m_logger, "Processing %zu messages.", m_processingQueue.size());
        while (!m_processingQueue.empty())
        {
            ProcessingItem item = std::move(m_processingQueue.front());
            m_processingQueue.pop();
            l.unlock();

            Result r = processMessage(std::move(item));
            if (Result::SUCCESS != r)
            {
                LOG_ERROR(m_logger, "Message processing failed with result %u(%s)",
                    static_cast<uint16_t>(r), resultToStr(r));
            }

            l.lock();
        }

    }
    LOG_INFO(m_logger, "Processor thread is stopped");
}

} // namespace rabbitmq