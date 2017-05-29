#include <libconfig.h++>

#include <unistd.h>
#include <random>

#include <logger/Logger.h>
#include <logger/LoggerDefines.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>
#include <rabbitmq/Consumer.h>
#include <rabbitmq/Publisher.h>
#include <rabbitmq/Handler.h>

#include <common/Types.h>

#include "TrafficGenerator.h"

using common::State;
using common::Result;

void signalHandler(int s)
{
    fprintf(stderr, "Caught signal %d\n", s);

    tg::Generator& gen = tg::Generator::instance();
    gen.stop();
    gen.deinitialize();

    logger::Logger& l = logger::Logger::instance();
    l.stop();

    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Invalid format. Configuration file must be specified.\n");
        fprintf(stderr, "Example: %s cfg.cfg\n", argv[0]);
        return 2;
    }

    std::string cfgName(argv[1]);

    logger::Logger& l = logger::Logger::instance();
    if (!l.configure(cfgName))
    {
        fprintf(stderr, "Cannot configure logger\n");
        return 2;
    }

    if (!l.start())
    {
        fprintf(stderr, "Cannot initialize logger\n");
        return 3;
    }

    tg::Generator& gen = tg::Generator::instance();;
    Result res = gen.initialize();
    if (Result::SUCCESS != res)
    {
        return 3;
    }
    res = gen.configure(cfgName);
    if (Result::SUCCESS != res)
    {
        return 3;
    }
    res = gen.start();
    if (Result::SUCCESS != res)
    {
        return 3;
    }

    res = gen.writeData();
    if (Result::SUCCESS != res)
    {
        return 3;
    }

    signal(SIGINT, signalHandler);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}