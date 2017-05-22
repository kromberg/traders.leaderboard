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

#include <common/Types.h>

using common::State;
using common::Result;

void writeTestData(
    rabbitmq::Publisher& publisher,
    const size_t countUsers,
    const size_t dealsPerUser,
    const size_t trasnactionSize)
{
    logger::CategoryPtr log = logger::Logger::getLogCategory("TEST_DATA_WRITER");

    std::default_random_engine generator;
    std::uniform_int_distribution<int16_t> distribution(
        std::numeric_limits<int16_t>::min(),
        std::numeric_limits<int16_t>::max());

    // start a transaction
    Result res = publisher.startTransactionSync();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(log, "Cannot start transaction");
        return ;
    }

    for (size_t i = 0; i < countUsers; ++i)
    {
        std::string idStr = std::to_string(i);
        if (!publisher.publish("my-exchange", "my-key", "user_registered(" + idStr + ",Abuda " + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }

        for (size_t deal = 0; deal < dealsPerUser; ++ deal)
        {
            if (!publisher.publish("my-exchange", "my-key", "user_deal(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(log, "Cannot publish message");
                return ;
            }
            if (!publisher.publish("my-exchange", "my-key", "user_deal_won(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(log, "Cannot publish message");
                return ;
            }
        }
        if (!publisher.publish("my-exchange", "my-key", "user_connected(" + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }

        if (!publisher.publish("my-exchange", "my-key", "user_disconnected(" + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }

        if (publisher.transactionMessagesCount() >= trasnactionSize)
        {
            res = publisher.commitTransactionSync();
            if (Result::SUCCESS != res)
            {
                LOG_ERROR(log, "Cannot commit transaction");
                return ;
            }

            LOG_DEBUG(log, "%zu users were processed", i);

            res = publisher.startTransactionSync();
            if (Result::SUCCESS != res)
            {
                LOG_ERROR(log, "Cannot start transaction");
                return ;
            }
        }
    }

    //publisher.publish("my-exchange", "my-key", "user_connected(10)");
    //publisher.publish("my-exchange", "my-key", "user_disconnected(500)");

    res = publisher.commitTransactionSync();
    if (Result::SUCCESS != res)
    {
        LOG_ERROR(log, "Cannot commit transaction");
        return ;
    }

    LOG_INFO(log, "%zu users were processed", countUsers);
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

    logger::CategoryPtr log = logger::Logger::getLogCategory("MAIN");

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
            LOG_ERROR(log, "Cannot start processor\n");
            return 3;
        }

        rabbitmq::Consumer consumer(eventLoop);
        consumer.attachProcessor(processor);
        Result res = consumer.configure(std::move(address));
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = consumer.initialize();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = consumer.start();
        if (Result::SUCCESS != res)
        {
            return 3;
        }

        res = consumer.declareExchangeSync("my-exchange", AMQP::fanout);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare exchange");
            return 3;
        }
        res = consumer.declareQueueSync("my-queue");
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare queue");
            return 3;
        }
        res = consumer.bindQueueSync("my-exchange", "my-queue", "my-routing-key");
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot bind queue");
            return 3;
        }
        res = consumer.setQosSync(1);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot set QOS");
            return 3;
        }

        consumer.consume("my-queue");

        sleep(50);

        consumer.stop();
        consumer.deinitialize();

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

        // todo: check result
        rabbitmq::Publisher publisher(eventLoop);
        Result res = publisher.configure(std::move(address));
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = publisher.initialize();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = publisher.start();
        if (Result::SUCCESS != res)
        {
            return 3;
        }

        res = publisher.declareExchangeSync("my-exchange", AMQP::fanout);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare exchange");
            return 3;
        }
        res = publisher.declareQueueSync("my-queue");
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare queue");
            return 3;
        }
        res = publisher.bindQueueSync("my-exchange", "my-queue", "my-routing-key");
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot bind queue");
            return 3;
        }

        writeTestData(publisher, countUsers, dealsPerUser, transactionSize);

        sleep(5);

        publisher.stop();
        publisher.deinitialize();
    }

    sleep(1);

    eventLoop.stop();

    l.deinitialize();

    return 0;
}