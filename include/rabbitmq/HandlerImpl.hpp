#ifndef MY_RABBIT_MQ_HANDLER_IMPL_HPP
#define MY_RABBIT_MQ_HANDLER_IMPL_HPP

namespace rabbitmq
{

inline Handler::Handler(EventLoop& loop) throw(std::runtime_error):
    m_eventLoop(loop),
    m_handler(m_eventLoop)
{
    m_logger = logger::Logger::getLogCategory("RMQ_HANDLER");
}

inline AMQP::TcpChannel& Handler::channel()
{
    return *m_channel;
}

template<class... Args>
inline Result Handler::declareExchangeSync(Args... args)
{
    SyncObj<Result> result;
    channel().declareExchange(std::forward<Args>(args)...)
        .onSuccess([&m_logger, &result] () -> void
            {
                LOG_INFO(m_logger, "Exchange was declared");
                result.set(Result::SUCCESS);
            })
        .onError([&m_logger, &result] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot declare exchange. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

template<class... Args>
inline Result Handler::declareQueueSync(Args... args)
{
    SyncObj<Result> result;
    channel().declareQueue(std::forward<Args>(args)...)
        .onSuccess([&m_logger, &result] (const std::string &name, uint32_t messagecount, uint32_t consumercount) -> void
            {
                LOG_INFO(m_logger, "Queue declared %s. Messages count: %u. Consumers count: %u",
                    name.c_str(), messagecount, consumercount);
                result.set(Result::SUCCESS);
            })
        .onError([&m_logger, &result] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot declare queue. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

template<class... Args>
inline Result Handler::bindQueueSync(Args... args)
{
    SyncObj<Result> result;
    channel().bindQueue(std::forward<Args>(args)...)
        .onSuccess([&m_logger, &result] () -> void
            {
                LOG_INFO(m_logger, "Queue was bound");
                result.set(Result::SUCCESS);
            })
        .onError([&m_logger, &result] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot bind queue. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

template<class... Args>
inline Result Handler::setQosSync(Args... args)
{
    SyncObj<Result> result;
    channel().bindQueue(std::forward<Args>(args)...)
        .onSuccess([&m_logger, &result] () -> void
            {
                LOG_INFO(m_logger, "QOS was set");
                result.set(Result::SUCCESS);
            })
        .onError([&m_logger, &result] (const char* message) -> void
            {
                LOG_ERROR(m_logger, "Cannot set QOS. Error message: %s",
                    message);
                result.set(Result::FAILED);
            });
    result.wait();
    return result.get();
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_HANDLER_IMPL_HPP