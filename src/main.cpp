#include <unistd.h>
#include <rabbitmq/ConnectionHandler.h>
#include <rabbitmq/TcpHandler.h>

void writeTestData(AMQP::Channel& channel)
{
    // start a transaction
    channel.startTransaction();

    // publish a number of messages
    channel.publish("my-exchange", "my-key", "my first message");
    channel.publish("my-exchange", "my-key", "another message");

    // commit the transactions, and set up callbacks that are called when
    // the transaction was successful or not
    channel.commitTransaction()
        .onSuccess([]() {
            // all messages were successfully published
        })
        .onError([](const char* msg) {
            // none of the messages were published
            // now we have to do it all over again
            std::cout << "error occurred while committing rabbitmq transaction. description: " << msg << '\n';
        });
}

void addConsumers(AMQP::Channel& channel)
{
    // callback function that is called when the consume operation starts
    auto startCb = [](const std::string &consumertag) {

        std::cout << "consume operation started" << std::endl;
    };

    // callback function that is called when the consume operation failed
    auto errorCb = [](const char *message) {

        std::cout << "consume operation failed" << std::endl;
    };

    // callback operation when a message was received
    auto messageCb = [&channel](const AMQP::Message &message, uint64_t deliveryTag, bool redelivered) {

        std::cout << "message received" << std::endl;

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
    // create an instance of your own connection handler
    rabbitmq::TcpHandler myHandler;

    // address of the server
    AMQP::Address address("amqp://guest:guest@localhost:5672/");

    // create a AMQP connection object
    AMQP::TcpConnection connection(&myHandler, address);

    // and create a channel
    AMQP::TcpChannel channel(&connection);

    channel.onError([](const char *message) {
        std::cout << "channel error: " << message << std::endl;
    });

    channel.onReady([]() {
        std::cout << "channel is ready" << std::endl;
    });

    volatile bool isCompleted = false;
    // use the channel object to call the AMQP method you like
    channel.declareExchange("my-exchange", AMQP::fanout)
        .onSuccess([&isCompleted]() {
            isCompleted = true;
        })

        .onError([](const char *message) {
            // something went wrong creating the exchange
            std::cout << "channel error: " << message << std::endl;
            exit(2);
        });

    while (!isCompleted)
    {
        std::cout << "waiting for exchange\n";
        sleep(1);
    }
    channel.declareQueue("my-queue");
    channel.bindQueue("my-exchange", "my-queue", "my-routing-key");

    addConsumers(channel);

    writeTestData(channel);

    sleep(10);

    return 0;
}