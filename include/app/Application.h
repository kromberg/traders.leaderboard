#ifndef MY_APP_APPLICATION_H
#define MY_APP_APPLICATION_H

#include "Utils.h"

namespace app
{
class Application
{
private:
    ConsumerPtr m_consumer;
    PublisherPtr m_publisher;

public:
    Application();
    ~Application();
};
} // namespace app

#endif // MY_APP_APPLICATION_H