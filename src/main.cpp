#include <unistd.h>
#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <rabbitmq/Handler.h>
#include <rabbitmq/Processor.h>

void writeTestData(rabbitmq::Publisher& publisher)
{
    // start a transaction
    publisher.startTransaction();
    for (size_t i = 1; i < 10000; ++i)
    {
        std::string idStr = std::to_string(i);
        {
            std::string message("user_registered(" + idStr + ",Abuda " + idStr + ")");
            publisher.publish("my-exchange", "my-key", strndup(message.data(), message.size()), message.size());
        }
        {
            std::string message("user_deal(" + idStr + ",2017:05:10T10:10:10," + std::to_string(rand()) + ")");
            publisher.publish("my-exchange", "my-key", strndup(message.data(), message.size()), message.size());
        }
        {
            std::string message("user_deal_won(" + idStr + ",2017:05:10T10:10:10," + std::to_string(rand()) + ")");
            publisher.publish("my-exchange", "my-key", strndup(message.data(), message.size()), message.size());
        }
        {
            std::string message("user_deal(" + idStr + ",2017:05:10T10:10:10," + std::to_string(rand()) + ")");
            publisher.publish("my-exchange", "my-key", strndup(message.data(), message.size()), message.size());
        }

        if (i % 100 == 0)
        {
            publisher.commitTransaction();
            publisher.waitTransactionEnd();
            publisher.startTransaction();
        }
    }

    //publisher.publish("my-exchange", "my-key", "user_connected(10)");
    //publisher.publish("my-exchange", "my-key", "user_disconnected(500)");

    publisher.commitTransaction();
    publisher.waitTransactionEnd();
}

int main(int argc, char* argv[])
{
    bool generator = false;
    if (argc >= 2)
    {
        generator = true;
    }
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

    if (!generator)
    {
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
        consumer.bindQueue("my-exchange", "my-queue", "my-routing-key");

        consumer.consume("my-queue");

        //writeTestData(channel);

        sleep(50);

        processor->stop();
    }
    else
    {
        rabbitmq::Publisher publisher(channel);

        // use the channel object to call the AMQP method you like
        publisher.declareExchange("my-exchange", AMQP::fanout);
        publisher.declareQueue("my-queue");
        publisher.bindQueue("my-exchange", "my-queue", "my-routing-key");

        writeTestData(publisher);

        sleep(20);
    }

    connection.close();


    sleep(1);

    eventLoop.stop();

    l.deinitialize();

    return 0;
}