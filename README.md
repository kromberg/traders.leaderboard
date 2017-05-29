# Leaderboard application
## Pre-requisites
This application depends on the following packages:
 * [CMake](https://cmake.org/)
 * [AMQP-CPP](https://github.com/CopernicaMarketingSoftware/AMQP-CPP)
 * [mongo-c-driver](https://github.com/mongodb/mongo-c-driver)
 * [mongo-cxx-driver](https://github.com/mongodb/mongo-cxx-driver)
 * [libconfig++](http://www.hyperrealm.com/libconfig/)
 * [Google Test](https://github.com/google/googletest) (for tests only)
## Installation
```bash
mkdir build
cd build
cmake ..
make or make <target>
./leaderboard_app.out app.cfg
or
./traffic_generator.out tg.cfg
```
## Tests
```bash
mkdir build
cd build
cmake ..
make or make <target>
./leaderboard_test.out
make coverage
<browser you like> coverage/index.html
make clean-coverage
```
## Initialization
Logger must be configured and started before the application [Logger information](Logger.md)
```c++
app::Application& application = app::Application::instance();
Result res = application.initialize();
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Configuration.
For more informartion, please, see [Configuration information](Config.md)
```c++
Result res = application.configure(cfgName);
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Start
```c++
Result res = application.start();
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Stop
```c++
application.stop();
```
## Deinitialization
```c++
application.deinitialization();
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
# TBD