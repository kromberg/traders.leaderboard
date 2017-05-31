# Leaderboard application
## Pre-requisites
This application depends on the following packages:
 * [CMake](https://cmake.org/)
 * [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP)
 * [mongo-c-driver](https://github.com/mongodb/mongo-c-driver)
 * [mongo-cxx-driver](https://github.com/mongodb/mongo-cxx-driver)
 * [libconfig++](http://www.hyperrealm.com/libconfig/)
 * [Node.js](https://nodejs.org/) (for web-app only)
 * [NPM](https://www.npmjs.com/) (for web-app only)
 * [Google Test](https://github.com/google/googletest) (for tests only)
 * [Fantasyname](https://github.com/skeeto/fantasyname) (no need to install, it is embedded)
## Installation
### Application and Traffic Generator
```bash
mkdir build
cd build
cmake ..
make or make <target>
./leaderboard_app.out app.cfg
or
./traffic_generator.out tg.cfg
```

For more information about Application, please, see [Application information](Application.md)
### Simple web-application
```bash
cd web
npm install
npm start
```

For more information about Web-Application, please, see [Web-Application information](WebApplication.md)
## Tests
```bash
mkdir build
cd build
cmake ..
make or make <target>
./leaderboard_test.out
or
./leaderboard_test.out.coverage
make coverage
<browser you like> coverage/index.html
make clean-coverage
```

# Incoming Messages
Application is a service that handles the following messages:

## user_registered(id,name)
 * id - unique user identifier
 * name - user name

Registers the user in an internal storage (database), otherwise return an error

Example: user_registered(666,Devil)

## user_renamed(id,name)
 * id - unique user identifier
 * name - user name

Rename the user in an internal storage (database), otherwise return an error

Example: user_renamed(666,Angel)

## user_deal(id,time,amount)
 * id - unique user identifier
 * time - time with the following format: "%Y-%m-%dT%H:%M:%S"
 * amount - amount of money earned from the deal

Store the information about the user deal in an internal storage (database), otherwise return an error

Example: user_deal(666,2006-06-06T06:06:06,9)

## user_deal_won(id,time,amount)
 * Same as user_deal
## user_connected(id)
 * id - unique user identifier

Connect the user to the service. If user is connected, then the service will send leaderboards (Top-10 and +/- 10 users for the curren one) every application.loop-interval (configurable parameter) seconds

Example: user_connected(666)

## user_disconnected(id)
 * id - unique user identifier

Disconnect the user from the service.

Example: user_disconnected(666)

# Outgoing Messages
## leaderboard
```json
{
    "id" : <user id>,
    "name" : "<user name>",
    "leaderboard" :
    {
        "time" : "<time in the format %Y-%m-%dT%H:%M:%S>",
        "scores" :
        [
            "position" : <user position>,
            "id" : <user id>,
            "name" : "<user name>",
            "score" : <user score>,
        ]
    }
}
```
For all users connected and top-10 (id: -1) the service will send such messages to RabbitMQ server.

# Application architecture
## Logger
It implements basic facilities for logging to console and file
## Application
The Application sigleton is responsible for the service lifecycle, initialization, configuration, start, stop, deinitialization
## RabbitMQ Event Loop
This part of application is needed to handle socket events and pass sockets and events to the RabbitMQ library.
## RabbitMQ handler
Basic handler that wraps AMQP-CPP channel methods
## RabbitMQ consumer and publisher
They are needed for messages consuming and publishing respectively
## Databases
The application implements facilities to store/retreive/update users, connected users and user scores
### In-Memory
It is an embedded database. All data will be lost after service restart
### [MongoDB](https://www.mongodb.com/)
## Logic
### Consumer
This is a RabbitMQ Consumer. All received messages are pushed to the queue.
### Processor
The processor pops the message from the queue and process it. Also, it sends an ack or nack back to the RabbitMQ server.

One can configure the number of processors
### Logic loop
Every X seconds logic loop reads the information about connected users from the database, retreive the information about leaderboard (top-X and for each connected user) and publishes these messages to the RabbitMQ.
# Bugs and Action Points
- [ ] It seems that AMQP-CPP library is not stable in some cases (often, on starting and committing transactions), so, I have an action point to rewrite this part using rabbitmq C API [link](https://github.com/alanxz/rabbitmq-c)
- [ ] I think that it is better to split this application into two: one will handle all requests and the other will read data from Database and send leaderboards information
- [ ] Rework processing queues. We can define own queue for each processor