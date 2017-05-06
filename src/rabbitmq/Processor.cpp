#include <logger/Logger.h>
#include <logger/LoggerDefines.h>

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
    m_state = State::STARTED;

    std::thread t(std::bind(&Processor::mainThreadFunc, this));
    std::swap(t, m_thread);

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
    m_state = State::STOPPED;
    m_processingQueueCv.notify_one();
    m_thread.join();
}

Result Processor::processMessage(ProcessingItem&& item)
{
    LOG_DEBUG(m_logger, "Start processing message: <Channel: %p, Body: %s. Exchange: %s. Routing key :%s."
        "Delivery tag: %lu. Redelivered: %s>",
        item.m_channel.get(), item.m_message.c_str(),
        item.m_exchange.c_str(), item.m_routingkey.c_str(),
        item.m_deliveryTag, (item.m_redelivered ? "true" : "false"));



    // acknowledge the message
    item.m_channel->ack(item.m_deliveryTag);
    return Result::SUCCESS;
}

void Processor::mainThreadFunc()
{
    while (State::STARTED == m_state)
    {
        std::unique_lock<std::mutex> l(m_processingQueueGuard);
        if (m_processingQueue.empty())
        {
            m_processingQueueCv.wait(l);
        }

        while (!m_processingQueue.empty())
        {
            ProcessingItem item = std::move(m_processingQueue.front());
            l.release();

            Result r = processMessage(std::move(item));
            if (Result::SUCCESS != r)
            {
                LOG_ERROR(m_logger, "Message processing failed with result %u(%s)",
                    static_cast<uint16_t>(r), resultToStr(r));
            }

            l.lock();
        }

    }
}
} // namespace rabbitmq