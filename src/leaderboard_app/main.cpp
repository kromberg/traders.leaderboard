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

    logger::CategoryPtr log = logger::Logger::getLogCategory("MAIN");

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

    l.deinitialize();

    return 0;
}