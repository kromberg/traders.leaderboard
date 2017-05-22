#ifndef MY_APP_APPLICATION_H
#define MY_APP_APPLICATION_H

#include "../rabbitmq/Fwd.h"

namespace app
{
class Application
{
private:
    rabbitmq::ConsumerPtr m_consumer;
    rabbitmq::PublisherPtr m_publisher;

public:
    Application();
    ~Application();
};
} // namespace app

#endif // MY_APP_APPLICATION_H