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

void signalHandler(int s)
{
    fprintf(stderr, "Caught signal %d\n",s);

    app::Application& application = app::Application::getInstance();
    application.stop();
    application.deinitialize();

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

    app::Application& application = app::Application::getInstance();
    Result res = application.initialize();
    if (Result::SUCCESS != res)
    {
        return 3;
    }
    res = application.configure(cfgName);
    if (Result::SUCCESS != res)
    {
        return 3;
    }
    res = application.start();
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