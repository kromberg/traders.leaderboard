#include <libconfig.h++>

#include <unistd.h>
#include <random>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <app/Application.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <rabbitmq/Handler.h>

#include <common/Types.h>

using common::State;
using common::Result;

void writeTestData(
    rabbitmq::Publisher& publisher,
    const std::string& exchangeName,
    const std::string& routingKey,
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
        if (!publisher.publish(exchangeName, routingKey, "user_registered(" + idStr + ",Abuda " + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }

        for (size_t deal = 0; deal < dealsPerUser; ++ deal)
        {
            if (!publisher.publish(exchangeName, routingKey, "user_deal(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(log, "Cannot publish message");
                return ;
            }
            if (!publisher.publish(exchangeName, routingKey, "user_deal_won(" + idStr + ",2017-05-20T10:10:10," + std::to_string(distribution(generator)) + ")"))
            {
                LOG_ERROR(log, "Cannot publish message");
                return ;
            }
        }
        if (!publisher.publish(exchangeName, routingKey, "user_connected(" + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }

        /*if (!publisher.publish(exchangeName, routingKey, "user_disconnected(" + idStr + ")"))
        {
            LOG_ERROR(log, "Cannot publish message");
            return ;
        }*/

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

    //publisher.publish(exchangeName, routingKey, "user_connected(10)");
    //publisher.publish(exchangeName, routingKey, "user_disconnected(500)");

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

    if (!generator)
    {
        app::Application& application = app::Application::getInstance();
        Result res = application.initialize();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = application.configure();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = application.start();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        sleep(200);
        application.stop();
        application.deinitialize();
    }
    else
    {
        // creating event loop
        rabbitmq::EventLoop eventLoop;
        eventLoop.start();

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

        libconfig::Config cfg;
        try
        {
            cfg.readFile("cfg.cfg");
        }
        catch (...) {}

        std::string exchangeName = "leaderboard";
        std::string queueName = "users-events-queue";
        std::string routingKey = "routing-key";

        rabbitmq::Publisher publisher(eventLoop);
        Result res = publisher.initialize();
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = publisher.configure(cfg);
        if (Result::SUCCESS != res)
        {
            return 3;
        }
        res = publisher.start();
        if (Result::SUCCESS != res)
        {
            return 3;
        }

        res = publisher.declareExchangeSync(exchangeName, AMQP::fanout);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare exchange");
            return 3;
        }
        res = publisher.declareQueueSync(queueName);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot declare queue");
            return 3;
        }
        res = publisher.bindQueueSync(exchangeName, queueName, routingKey);
        if (Result::SUCCESS != res)
        {
            LOG_ERROR(log, "Cannot bind queue");
            return 3;
        }

        writeTestData(publisher, exchangeName, routingKey, countUsers, dealsPerUser, transactionSize);

        sleep(5);

        publisher.stop();
        publisher.deinitialize();

        sleep(1);

        eventLoop.stop();

    }


    l.deinitialize();

    return 0;
}