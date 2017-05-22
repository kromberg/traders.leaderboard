#ifndef MY_RABBIT_MQ_PUBLISHER_IMPL_HPP
#define MY_RABBIT_MQ_PUBLISHER_IMPL_HPP

namespace rabbitmq
{
inline Publisher::Publisher(EventLoop& loop, const AMQP::Address &address):
    Handler(loop, address)
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