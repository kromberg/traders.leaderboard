#include <unistd.h>
#include <logger/Logger.h>
#include <rabbitmq/EventLoop.h>
#include <rabbitmq/TcpHandler.h>

void writeTestData(AMQP::Channel& channel)
{
    LOG_DEBUG("CH_PUBLISHER", "Starting transaction");
    // start a transaction
    channel.startTransaction();

    LOG_DEBUG("CH_PUBLISHER", "Publishing messages");
    // publish a number of messages
    channel.publish("my-exchange", "my-key", "my first message");
    channel.publish("my-exchange", "my-key", "another message");

    channel.commitTransaction()
        .onSuccess([]() {
            LOG_DEBUG("CH_PUBLISHER", "All messages were published");
        })
        .onError([](const char* msg) {
            LOG_ERROR("CH_PUBLISHER", "Error occurred while committing rabbitmq transaction. Description: %s",
                msg);
        });
}

void addConsumers(AMQP::Channel& channel)
{
    // callback function that is called when the consume operation starts
    auto startCb = [](const std::string &consumertag) {
        LOG_DEBUG("CH_CONSUMER", "Consume operation started");
    };

    // callback function that is called when the consume operation failed
    auto errorCb = [](const char *message) {
        LOG_ERROR("CH_CONSUMER", "Consume operation failed");
    };

    // callback operation when a message was received
    auto messageCb = [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

        std::string messageStr(message.body(), message.bodySize());
        LOG_DEBUG("CH_CONSUMER", "Message received: %s", messageStr.c_str());

        // acknowledge the message
        channel.ack(deliveryTag);
    };

    // start consuming from the queue, and install the callbacks
    channel.consume("my-queue")
        .onReceived(messageCb)
        .onSuccess(startCb)
        .onError(errorCb);
}

int main(int argc, char* argv[])
{
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
    AMQP::TcpChannel channel(&connection);

    channel.onError([](const char *message) {
        LOG_ERROR("RMQ_CHANNEL", "Channel error: %s", message);
    });

    channel.onReady([]() {
        LOG_INFO("RMQ_CHANNEL", "Channel is ready");
    });

    volatile bool isCompleted = false;
    // use the channel object to call the AMQP method you like
    channel.declareExchange("my-exchange", AMQP::fanout)
        .onSuccess([&isCompleted]() {
            LOG_INFO("RMQ_CHANNEL", "Exchange is ready");
            isCompleted = true;
        });

    channel.declareQueue("my-queue")
        .onSuccess([](const std::string &name, uint32_t messagecount, uint32_t consumercount) {
            LOG_INFO("RMQ_CHANNEL", "Queue declared %s. Messages count: %u. Consumers count: %u",
                name.c_str(), messagecount, consumercount);
        });

    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");

    addConsumers(channel);

    //writeTestData(channel);

    sleep(10);

    connection.close();

    sleep(1);

    eventLoop.stop();

    return 0;
}