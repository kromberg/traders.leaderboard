#include <unistd.h>
#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <rabbitmq/Handler.h>
#include <rabbitmq/Processor.h>

void writeTestData(rabbitmq::Publisher& publisher, const size_t trasnactionSize)
{
    // start a transaction
    publisher.startTransaction();
    publisher.waitTransactionStarted();
    for (size_t i = 0; i < 1000; ++i)
    {
        std::string idStr = std::to_string(i);
        publisher.publish("my-exchange", "my-key", "user_registered(" + idStr + ",Abuda " + idStr + ")");
        publisher.publish("my-exchange", "my-key", "user_deal(" + idStr + ",2017:05:16T10:10:10," + std::to_string(rand()) + ")");
        publisher.publish("my-exchange", "my-key", "user_deal_won(" + idStr + ",2017:05:16T10:10:10," + std::to_string(rand()) + ")");
        publisher.publish("my-exchange", "my-key", "user_deal(" + idStr + ",2017:05:16T10:10:10," + std::to_string(rand()) + ")");
        publisher.publish("my-exchange", "my-key", "user_connected(" + idStr + ")");

        if (publisher.transactionMessagesCount() >= trasnactionSize)
        {
            publisher.commitTransaction();
            publisher.waitTransactionCommitted();

            publisher.startTransaction();
            publisher.waitTransactionStarted();
        }
    }

    //publisher.publish("my-exchange", "my-key", "user_connected(10)");
    //publisher.publish("my-exchange", "my-key", "user_disconnected(500)");

    publisher.commitTransaction();
    publisher.waitTransactionCommitted();
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

    // address of the server
    AMQP::Address address("amqp://guest:guest@localhost:5672/");

    if (!generator)
    {
        rabbitmq::ProcessorPtr processor(new rabbitmq::Processor());
        if (!processor->start())
        {
            LOG_ERROR(m_logger, "Cannot start processor\n");
            return 3;
        }

        rabbitmq::Consumer consumer(eventLoop, address);
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
        size_t transactionSize = 100;
        if (argc >= 3)
        {
            transactionSize = std::stoul(argv[2]);
        }

        rabbitmq::Publisher publisher(eventLoop, address);

        publisher.waitChannelReady();

        // use the channel object to call the AMQP method you like
        publisher.declareExchange("my-exchange", AMQP::fanout);
        publisher.declareQueue("my-queue");
        publisher.bindQueue("my-exchange", "my-queue", "my-routing-key");

        writeTestData(publisher, transactionSize);

        sleep(100);

        publisher.channel().close();

        sleep(20);
    }

    sleep(1);

    eventLoop.stop();

    l.deinitialize();

    return 0;
}