var express = require('express');
var session = require('client-sessions');
var pug = require('pug');
var amqp = require('amqplib/callback_api');
var config = require('config');
var path = require('path');

var app = express();
app.use(session({
    cookieName: 'session',
    secret: 'asdfasfbmanui32412tma9',
    duration: 30 * 60 * 1000,
    activeDuration: 5 * 60 * 1000,
}));

// process static files
app.use('/listjs', express.static(path.join(__dirname, 'node_modules/list.js/dist')));
app.use('/material-css', express.static(path.join(__dirname, 'node_modules/materialize-css/dist')));

app.use(express.static('public'));
app.set('views', __dirname + '/public');
app.set('view engine', 'pug');

var leaderboards = {
    obj: {},

    add: function(leaderboard_obj, limit) {
        function parseISOLocal(s) {
            var b = s.split(/\D/);
            return new Date(b[0], b[1]-1, b[2], b[3], b[4], b[5]);
        }
        if (!this.obj.hasOwnProperty(leaderboard_obj.id)) {
            this.obj[leaderboard_obj.id] = {}
            tmp_leaderboard_obj = this.obj[leaderboard_obj.id]
            tmp_leaderboard_obj.id = leaderboard_obj.id
            tmp_leaderboard_obj.name = leaderboard_obj.name
            tmp_leaderboard_obj.trend = []
        }
        else {
            tmp_leaderboard_obj = this.obj[leaderboard_obj.id]
        }
        leaderboard_obj.leaderboard.time = parseISOLocal(leaderboard_obj.leaderboard.time)
        leaderboard_obj.leaderboard.time_str = leaderboard_obj.leaderboard.time.toString()
        tmp_leaderboard_obj.trend.push(leaderboard_obj.leaderboard)
        tmp_leaderboard_obj.trend = tmp_leaderboard_obj.trend.sort(function (lhv, rhv) {
            return lhv.time > rhv.time;
        })

        while (tmp_leaderboard_obj.trend.length > limit) {
            tmp_leaderboard_obj.trend.shift()
        }
    }
}

function leaderboards_req(req, res) {
    // render
    res.render('index', { 'leaderboards_obj' : leaderboards.obj})
}

// process index.html
app.get('/', leaderboards_req)
app.get('/leaderboards', leaderboards_req)
app.get('/leaderboard/:user_id', function (req, res, next) {
    var user_id = req.params.user_id
    if (!leaderboards.obj.hasOwnProperty(user_id)) {
        return next("Cannot find leaderboard for user " + user_id)
    }
    // render
    res.render('index', { 'leaderboards_obj' : leaderboards.obj, 'user_id' : user_id, 'trend_idx' : 0})
})
app.get('/leaderboard/:user_id/:trend_idx', function (req, res, next) {
    var user_id = req.params.user_id
    var trend_idx = req.params.trend_idx
    if (!leaderboards.obj.hasOwnProperty(user_id)) {
        return next("Cannot find leaderboard for user " + user_id)
    }
    if (trend_idx >leaderboards.obj[user_id].trend.length) {
        return next("Cannot find leaderboard for user " + user_id + " with trend index " + trend_idx)
    }
    // render
    res.render('index', { 'leaderboards_obj' : leaderboards.obj, 'user_id' : user_id, 'trend_idx' : trend_idx})
})

function guid() {
    function S4() {
        return (((1+Math.random())*0x10000)|0).toString(16).substring(1); 
    }
     
    // then to call it, plus stitch in '4' in the third group
    guid = (S4() + S4() + "-" + S4() + "-4" + S4().substr(0,3) + "-" + S4() + "-" + S4() + S4() + S4()).toLowerCase();
    return guid
}

function read_amqp_config() {
    var amqp_cfg = {}
    amqp_cfg.address = 'amqp://guest:guest@localhost:5672/'
    if (config.has('amqp.address')) {
        amqp_cfg.address = config.get('amqp.address');
    }
    console.info('amqp.address = %s', amqp_cfg.address)

    amqp_cfg.exchange = 'leaderboard'
    if (config.has('amqp.exchange')) {
        amqp_cfg.exchange = config.get('amqp.exchange');
    }
    console.info('amqp.exchange = %s', amqp_cfg.exchange)

    amqp_cfg.routing_key = 'leaderboard-ley'
    if (config.has('amqp.routing-key')) {
        amqp_cfg.routing_key = config.get('amqp.routing-key');
    }
    console.info('amqp.routing-key = %s', amqp_cfg.routing_key)

    // generate queue name
    amqp_cfg.queue = guid()
    console.info('amqp.queue = %s (generated)', amqp_cfg.queue)

    return amqp_cfg
}

var queue_declared = false
var channel = null
var amqp_cfg = null
function amqp_start(amqp_cfg, msg_callback) {
    function handle_error(err) {
        if (err != null) {
            console.error(err)
            process.exit(1)
        }
    }

    // the power of callbacks!!!
    amqp.connect(amqp_cfg.address, function(err, conn) {
        handle_error(err)
        conn.createChannel(function (err, ch) {
            handle_error(err)

            channel = ch
            channel.assertExchange(amqp_cfg.exchange, 'fanout', { durable : false }, function(err, ok) {
                handle_error(err)

                channel.assertQueue(amqp_cfg.queue, { autoDelete : true }, function(err, ok) {
                    handle_error(err)

                    queue_declared = true

                    channel.bindQueue(amqp_cfg.queue, amqp_cfg.exchange, amqp_cfg.routing_key, null, function(err, ok) {
                        handle_error(err)

                        channel.consume(amqp_cfg.queue, msg_callback)
                    });
                });
            });
        });
    });
}

process.on('uncaughtException', function(err) {
    console.log(err);
    process.exit(1)
})

var server = app.listen(8080, function () {
    amqp_cfg = read_amqp_config()
    amqp_start(amqp_cfg, function(msg) {
        if (msg !== null) {
            leaderboard_obj = JSON.parse(msg.content.toString())
            //console.log(leaderboard_obj)

            leaderboards.add(leaderboard_obj, 10)

            channel.ack(msg);
        }
    });

    var host = server.address().address
    var port = server.address().port

    console.log("App is listening at http://%s:%s", host, port)

})