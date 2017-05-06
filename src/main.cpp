#include <unistd.h>
#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Processor.h>

void writeTestData(AMQP::Channel& channel)
{
    logger::CategoryPtr CH_PUBLISHER = logger::Logger::getLogCategory("CH_PUBLISHER");
    LOG_DEBUG(CH_PUBLISHER, "Starting transaction");
    // start a transaction
    channel.startTransaction();

    LOG_DEBUG(CH_PUBLISHER, "Publishing messages");
    // publish a number of messages
    channel.publish("my-exchange", "my-key", "user_registered(1,Alex Alex)");
    channel.publish("my-exchange", "my-key", "user_renamed(2,Egor Egor)");
    channel.publish("my-exchange", "my-key", "user_deal(3,time,1)");
    channel.publish("my-exchange", "my-key", "user_deal_won(4,time,1)");
    channel.publish("my-exchange", "my-key", "user_connected(5)");
    channel.publish("my-exchange", "my-key", "user_disconnected(6)");
    channel.publish("my-exchange", "my-key", "user_registered(11,Alex Alex)");
    channel.publish("my-exchange", "my-key", "user_renamed(12,Egor Egor)");
    channel.publish("my-exchange", "my-key", "user_deal(13,time,1)");
    channel.publish("my-exchange", "my-key", "user_deal_won(14,time,1)");
    channel.publish("my-exchange", "my-key", "user_connected(15)");
    channel.publish("my-exchange", "my-key", "user_disconnected(16)");

    channel.commitTransaction()
        .onSuccess([CH_PUBLISHER]() {
            LOG_DEBUG(CH_PUBLISHER, "All messages were published");
        })
        .onError([CH_PUBLISHER](const char* msg) {
            LOG_ERROR(CH_PUBLISHER, "Error occurred while committing rabbitmq transaction. Description: %s",
                msg);
        });
}

int main(int argc, char* argv[])
{
    logger::Logger& l = logger::Logger::instance();
    if (!l.configure())
    {
        fprintf(stderr, "Cannot configure logger\n");
        return 2;
    }

    if (!l.initialize())
    {
        fprintf(stderr, "Cannot initialize logger\n");
        return 3;
    }

    logger::CategoryPtr m_logger = logger::Logger::getLogCategory("MAIN");

    // creating event loop
    rabbitmq::EventLoop eventLoop;
    eventLoop.start();

    // create an instance of your own connection handler
    rabbitmq::TcpHandler myHandler(eventLoop);

    // address of the server
    AMQP::Address address("amqp://guest:guest@localhost:5672/");

    // create a AMQP connection object
    AMQP::TcpConnection connection(&myHandler, address);

    // and create a channel
    std::shared_ptr<AMQP::TcpChannel> channel(new AMQP::TcpChannel(&connection));

#if 1
    rabbitmq::ProcessorPtr processor(new rabbitmq::Processor());
    if (!processor->start())
    {
        LOG_ERROR(m_logger, "Cannot start processor\n");
        return 3;
    }

    rabbitmq::Consumer consumer(channel);
    consumer.attachProcessor(processor);

    // use the channel object to call the AMQP method you like
    consumer.declareExchange("my-exchange", AMQP::fanout);
    consumer.declareQueue("my-queue");
    consumer.channel().bindQueue("my-exchange", "my-queue", "my-routing-key");

    consumer.consume("my-queue");

    //writeTestData(channel);

    sleep(20);

    processor->stop();

#else
    writeTestData(*channel);

    sleep(5);
#endif
    
    connection.close();


    sleep(1);

    eventLoop.stop();

    l.deinitialize();

    return 0;
}