#ifndef MY_RABBIT_MQ_PUBLISHER_IMPL_HPP
#define MY_RABBIT_MQ_PUBLISHER_IMPL_HPP

#include "../logger/Logger.h"
#include "../logger/LoggerDefines.h"

namespace rabbitmq
{
inline Publisher::Publisher(EventLoop& loop):
    Handler(loop)
{
    m_logger = logger::Logger::getLogCategory("RMQ_PUBLISHER");
}

inline size_t Publisher::transactionMessagesCount() const
{
    return m_transactionMessagesCount;
}

template<class... Args>
inline bool Publisher::publish(Args... args)
{
    ++ m_transactionMessagesCount;
    return channel().publish(std::forward<Args>(args)...);
}


} // namespace rabbitmq

#endif // MY_RABBIT_MQ_PUBLISHER_IMPL_HPP