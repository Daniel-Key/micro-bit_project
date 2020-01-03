let messageLog = exports;

let con = undefined;

// a map from usernames to sets of websockets
let webSockets = {};

messageLog.init = conArg => {
    con = conArg;
};

// the user is logging a message
messageLog.logMessage = (req, res) => {
    let user = req.session.username;
    let obj = {
        msg: req.body.msg,
        time: new Date()
    };
    // update database
    con.query(
        'INSERT INTO message_log (recipient, sender_microbit_id, message) VALUES (?,?,?)',
        [user, req.body.sender_microbit_id, req.body.msg]
    );
    // notify all connected websockets logged in as this user
    let wsSet = webSockets[req.session.username];
    if (wsSet) {
        for (let ws of wsSet) {
            ws.send(JSON.stringify(obj));
        }
    }

    res.status(201).end();
};

// get a list of all logged messages
messageLog.getMessages = (req, res) => {
    let user = req.session.username;
    con.query(
        'SELECT * FROM message_log WHERE recipient=?',
        user,
        (err, response) => {
            res.status(201).json(response);
        }
    );
};

// get a websocket for realtime updates
messageLog.getWs = (ws, req) => {
    let user = req.session.username;
    if (!webSockets[user]) {
        webSockets[user] = new Set();
    }
    // add the websocket to the set of connected websockets for this user
    let logWs = webSockets[user];

    logWs.add(ws);
    ws.on('close', () => {
        // when the websocket closes, remove it
        logWs.delete(ws);
        if (logWs.size == 0) {
            delete webSockets[user];
        }
    });
};

// delete the messages in the log
messageLog.clear = (req, res) => {
    let user = req.session.username;
    con.query(
        'DELETE FROM message_log WHERE recipient=?',user,
        (err,result) => {
            if(err){
                throw err;
            }

            res.json("");
        });
};
