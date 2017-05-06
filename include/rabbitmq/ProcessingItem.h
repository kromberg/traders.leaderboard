#ifndef MY_RABBIT_MQ_PROCESSING_ITEM_H
#define MY_RABBIT_MQ_PROCESSING_ITEM_H

#include <memory>
#include <string>
#include <amqpcpp.h>

#include "Fwd.h"

namespace rabbitmq
{
struct ProcessingItem
{
    std::shared_ptr<AMQP::TcpChannel> m_channel;
    std::string m_message;
    std::string m_exchange;
    std::string m_routingkey;
    uint64_t m_deliveryTag;
    bool m_redelivered;
    std::string m_args;

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
} // namespace rabbitmq

#endif // MY_RABBIT_MQ_PROCESSING_ITEM_H