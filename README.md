# Leaderboard application
## Initialization
```c++
app::Application& application = app::Application::instance();
Result res = application.initialize();
if (Result::SUCCESS != res)
{
    // handle error
}
```
## Configuration. For more informartion, please, see [Logger.md](Logger.md)
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