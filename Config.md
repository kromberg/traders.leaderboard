# Application Configuration
```
version = "1.0";

logger:
{
    // file, stderr, stdout
    type = "file";
    // error, warn, info, debug
    level = "debug";
    // path to file if type is file
    filename = "app.log";
    categories =
    (
        {
            category = "category";
            // error, warn, info, debug
            level = "debug";
        }
    );
};
rabbitmq:
{
    // address of rabbitmq node
    address = "amqp://guest:guest@localhost:5672/";
};
db:
{
    // mongo, in-memory
    type = "mongo";
    // the followin options are applicable for mongodb only
    // address of the mongodb node
    address = "mongodb://localhost:27017";
    // database name
    db_name = "leaderboard_db";
    // collection to store users information
    users_collection_name = "users";
    // collection to store connected users
    connected_users_collection_name = "connected_users";
};
application:
{
    // count of message processors
    processors-count = 2;
    // loop interval in seconds
    loop-interval = 60;
    // consumer configuration
    consumer:
    {
        exchange = "leaderboard-users";
        queue = "users-events-queue";
        routing-key = "user-key";
    };
    // publisher configuration
    publisher:
    {
        exchange = "leaderboard";
        routing-key = "leaderboard-key";
    };
};
```

# Traffic Generator Configuration
```
version = "1.0";

logger:
{
    // file, stderr, stdout
    type = "file";
    // error, warn, info, debug
    level = "debug";
    // path to file if type is file
    filename = "app.log";
    categories =
    (
        {
            category = "category";
            // error, warn, info, debug
            level = "debug";
        }
    );
};
rabbitmq:
{
    // address of rabbitmq node
    address = "amqp://guest:guest@localhost:5672/";
};
tg:
{
    // count of users generated
    users-count = 1000;
    // user id offset
    user-offset = 0;
    // count of deals per user
    deals-per-user = 10;
    // rabbitmq transaction size
    transaction-size = 100;
    publisher:
    {
        exchange = "leaderboard-users";
        routing-key = "user-key";
    };
};
```