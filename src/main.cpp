#include <unistd.h>
#include <random>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <rabbitmq/Handler.h>
#include <rabbitmq/Processor.h>

void writeTestData(
    rabbitmq::Publisher& publisher,
    const size_t countUsers,
    const size_t dealsPerUser,
    const size_t trasnactionSize)
{
    logger::CategoryPtr logger = logger::Logger::getLogCategory("TEST_DATA_WRITER");

    std::default_random_engine generator;
    std::uniform_int_distribution<int16_t> distribution(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    // start a transaction
    publisher.startTransactionSync();

    for (size_t i = 0; i < countUsers; ++i)
    {
        std::string idStr = std::to_string(i);
        publisher.publish("my-exchange", "my-key", "user_registered(" + idStr + ",Abuda " + idStr + ")");

        for (size_t deal = 0; deal < dealsPerUser; ++ deal)
        {
            publisher.publish("my-exchange", "my-key", "user_deal(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")");
            publisher.publish("my-exchange", "my-key", "user_deal_won(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")");
        }
        publisher.publish("my-exchange", "my-key", "user_connected(" + idStr + ")");

        if (publisher.transactionMessagesCount() >= trasnactionSize)
        {
            publisher.commitTransactionSync();

            LOG_DEBUG(logger, "%zu users were processed", i);

            publisher.startTransactionSync();
        }
    }

    //publisher.publish("my-exchange", "my-key", "user_connected(10)");
    //publisher.publish("my-exchange", "my-key", "user_disconnected(500)");

    publisher.commitTransactionSync();

    LOG_INFO(logger, "%zu users were processed", countUsers);
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

        sleep(50);

        processor->stop();
    }
    else
    {
        size_t countUsers = 1000;
        size_t dealsPerUser = 10;
        size_t transactionSize = 100;

        if (argc >= 3)
        {
            countUsers = std::stoul(argv[2]);
        }
        if (argc >= 4)
        {
            dealsPerUser = std::stoul(argv[3]);
        }
        if (argc >= 5)
        {
            transactionSize = std::stoul(argv[4]);
        }

        rabbitmq::Publisher publisher(eventLoop, address);

        publisher.waitChannelReady();

        // use the channel object to call the AMQP method you like
        publisher.declareExchange("my-exchange", AMQP::fanout);
        publisher.declareQueue("my-queue");
        publisher.bindQueue("my-exchange", "my-queue", "my-routing-key");

        writeTestData(publisher, countUsers, dealsPerUser, transactionSize);

        publisher.channel().close();

        sleep(5);
    }

    sleep(1);

    eventLoop.stop();

    l.deinitialize();

    return 0;
}