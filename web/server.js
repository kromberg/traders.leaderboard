var express = require('express');
var session = require('client-sessions');
var pug = require('pug');


var app = express();

app.use(session({
  cookieName: 'session',
  secret: 'asdfasfbmanui32412tma9',
  duration: 30 * 60 * 1000,
  activeDuration: 5 * 60 * 1000,
}));

// process static files
app.use(express.static('public'));
app.set('views', __dirname + '/public');
app.set('view engine', 'pug');

// process index.html
app.get('/', function (req, res) {
    // render
    res.render('index', {})
})

var server = app.listen(8080, function () {

    var host = server.address().address
    var port = server.address().port

    console.log("App is listening at http://%s:%s", host, port)

})