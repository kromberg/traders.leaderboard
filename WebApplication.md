# Leaderboard web-application
## Main page of the web-application. At the left side one can see all leaderboards
![Main page](screenshots/main.png "Main page")
## We can filter leaderboard by name or id
![Search](screenshots/search_leaderboard.png "Search")
## This is the Top-10 traders
![Top Leaderboard](screenshots/top_leaderboard.png "Top Leaderboard")
## This is +/-10 position for the selected user
![User Leaderboard](screenshots/user_leaderboard1.png "User Leaderboard")
## The selected user is highlighted
![User Leaderboard](screenshots/user_leaderboard2.png "User Leaderboard")
## One can choose the time of the leaderboard. Currently only 10 previous leaderboards available
![User Leaderboard (Time choice)](screenshots/user_leaderboard_time.png "User Leaderboard (Time choice)")
## Web-application configuration
The configuration is located in web/config/default.json
```javascript
{
    // AMQP module config
    "amqp":
    {
        // RabbitMQ node address
        "address" : "amqp://guest:guest@localhost:5672/",
        // RabbitMQ exchange to receive messages from
        "exchange" : "leaderboard",
        // RabbitMQ messages routing key
        "routing-key" : "leaderboard-key"
    }
}
```