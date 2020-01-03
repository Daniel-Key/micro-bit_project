// get program args
const PORT = process.argv.length >= 3 ? Number(process.argv[2]) : 8080;
const UNSECURE_COOKIE = process.argv.length >= 4 ? process.argv[3] === 'UNSECURE_COOKIE' : false;
if(UNSECURE_COOKIE){
    console.log('UNSECURE_COOKIE');
}

// import modules
const express = require('express');
const expressWs = require('express-ws');
const bodyParser = require('body-parser');
const mysql = require('mysql');
const session = require('express-session');
const {
    promisify
} = require('util');

// import other server code files
const weather = require('./weather.js');
const login = require('./login.js');
const messageLog = require('./message-log.js');
const patternLock = require('./pattern-lock.js');
const twitter = require('./twitter.js');
const meshInterface = require('./mesh-interface.js');

// init express
const app = express();
app.use(bodyParser.json());
// init websockets
expressWs(app);

// make the website accessible via /JH-project/ and the api via /JH-project/api/
let router = express.Router();
let api = express.Router();

app.use('/', router);
app.use('/JH-project', router);

router.use('/api', api);

// init session, faciliating login
app.set('trust proxy', 1);
api.use(session({
    secret: 'MicroB1tsecret',
    resave: false,
    saveUninitialized: true,
    cookie: {
        secure: !UNSECURE_COOKIE,
        maxAge: 1200000
    }
}));

// login api
api.get('/loggedin', login.loggedin);
api.post("/credentials", login.signIn);
api.post("/login_pattern", login.loginPattern);
api.post("/register", login.register);
api.ws('/mesh_interface', meshInterface.getWs);

// only allow logged in users past this point
api.use((req, res, next) => {
    if (req.session.username) {
        // console.log(`you shall pass, ${req.session.username}`);
        next();
    } else {
        res.status(401).send('Unauthorised')
        // console.log("you shall not pass");
    }
});

api.post('/signout', login.signout);

// message log
api.post('/message-log', messageLog.logMessage);
api.get('/message-log', messageLog.getMessages);
api.delete('/message-log', messageLog.clear);

api.ws('/message-ws', messageLog.getWs);

// weather
api.get('/weather', weather.getFormatted);
api.ws('/city-autocomplete', weather.autoCompleteCityWS);

//  Twitter
api.post('/twitter', twitter.sendTweet);
api.get('/twitter', twitter.getTweet);
api.post('/twitter-subscribe', twitter.subscribe);
api.get('/twitter-subscribe', twitter.getSubscriptions);
api.delete('/twitter-subscribe/:twitter_handle', twitter.unSubscribe);

api.post('/pattern', patternLock.setPattern);

// static files
router.use('/', express.static('static'));

// catch errors
app.use((err, req, res, next) => {
    console.log(`error caught by express at ${new Date()}:`);
    console.error(err);
    res.status(500).send('Server error');
});

// connect to the database
let con;

function connect() {
    // sql connection
    con = mysql.createPool({
        host: "dgk2.host.cs.st-andrews.ac.uk",
        user: "dgk2",
        password: "jh2834WE.K!2eV",
        database: "dgk2_cs3099_JH_db"
    });
    con.asyncQuery = promisify(con.query);
    // pass sql connection to other code files
    login.init(con);
    weather.init(con);
    messageLog.init(con);
    patternLock.init(con);
    twitter.init(con);
    meshInterface.init(con);
}

async function main() {
    // load the city list from the file
    await weather.loadCities();
    // connect to the database
    connect();

    // start the sever
    app.listen(PORT);
    console.log(`Server listening on port: ${PORT} on ${new Date()}`);
};

main().catch(console.log);
