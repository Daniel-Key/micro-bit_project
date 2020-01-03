//  ref: https://www.npmjs.com/package/twitter?fbclid=IwAR1LlesSTqBNwfjgeO4_cPXxt48AL74De7pP5562z-e-TlGN12LKt9qv9uc
const Twitter = require('twitter');
const {
    promisify
} = require('util');

const {
    spawn
} = require('child_process');

const WaitQueue = require('wait-queue');

//  Client details to access Twitter via B-1's developer account.
//  May want to consider environment variables to keep this info safe.
//  Format used for user based authentication, eg user+pass for microbit verification
var client = new Twitter({
    consumer_key: 'VCdvhstRuvP6rcyezRGFz11Ou',
    consumer_secret: 'Zob9WMUprTHwNwQkMz9TrBziyuPAiUmxIIIEsnjVm7IJtAFbfu',
    access_token_key: '1061939344246800384-flQTkjF4QrrE2TMQ74bXjdrksPzdTg',
    access_token_secret: '9ZB8TxxvlvPFtgR12KeRCDsLH9RGcjpj9Jcf2y7k4cS24'
});

// load the spam filter model query script
let spam_filter = spawn('python3', ['modelquery.py'], {
    cwd: 'spam-filter'
});
// a queue in which to put responses from the spam filter
const isSpamQueue = new WaitQueue();

// when the spam filter gives an answer, enqueue it
spam_filter.stdout.on('data', data => {
    // if the spam filter returns 1, it is spam
    isSpamQueue.push(data.toString() == '1\n');
});

// let the index.js pass the sql connection to this module
let con;
exports.init = (conArg) => {
    con = conArg;
}

//  Example tweet. See results here: https://twitter.com/cs3099
//  ref: https://developer.twitter.com/en/docs/tweets/post-and-engage/api-reference/post-statuses-update
exports.sendTweet = async (req, res) => {
    //  Post a tweet.
    try {
        await client.post('statuses/update', {
            status: req.body.msg
        });
        res.json(true);
    } catch {
        res.status(500).json(String(error));
    }
};


//  Get tweets. Returns: <handle>:<text>
//  statuses/user_timeline returns a user's timeline. Use parameters to specify what is wanted from a generic timeline. Will default to user's timeline.
//  ref: https://developer.twitter.com/en/docs/tweets/timelines/api-reference/get-statuses-user_timeline.html
exports.getTweet = async (req, res) => {
    //  Get some tweets.
    let tweets = [];
    try {
        // for every twitter handle you are subscibed to...
        let twitter_handles = await getSubscriptions(req.session.username);

        for (let twitter_handle of twitter_handles) {
            // get the tweet
            let [tweet] = await client.get('statuses/user_timeline', {
                q: 'node.js',
                screen_name: twitter_handle,
                count: 2
            });
            if (tweet) {
                // if the tweet exists, query the spam filter and add it to the list
                tweets.push(tweet);
                spam_filter.stdin.write(JSON.stringify(tweet.text) + '\n');
            }
        }
    } catch (e) {
        res.status(422).send("error");
        return;
    }

    // find the most recent non-spam tweet
    let recentTweetTime = -Infinity;
    let recentTweet = undefined;

    for (let tweet of tweets) {
        let isSpam = await isSpamQueue.shift();
        let tweetTime = new Date(tweet.created_at);
        if (!isSpam && tweetTime > recentTweetTime) {
            recentTweet = tweet;
            recentTweetTime = tweetTime;
        }
    }

    if (recentTweet) {
        //  Send back the first Tweet.
        let firstTweet = `${recentTweet.user.screen_name}: ${recentTweet.text}`;

        res.send(firstTweet);
    } else {
        // if there are no tweets, repond with the text "0 tweets"
        res.send("0 tweets");
    }
};

// add a twitter handle to the list of subscribers
exports.subscribe = async (req, res) => {
    let username = req.session.username;
    let twitter_handle = req.body.twitter_handle;

    // get a tweet to check that this is a real twitter handle
    try {
        await client.get('statuses/user_timeline', {
            q: 'node.js',
            screen_name: twitter_handle,
            count: 1
        });
    } catch {
        res.status(422).send("No such user");
        return;
    }

    try {
        // attempt to insert
        let query = con.asyncQuery('INSERT INTO twitter_subscribers VALUES (?, ?)',
            [username, twitter_handle]);

        await query;
        // if success, respond 'Subscibed'
        res.json('Subscibed');
    } catch {
        // if the query failed, e.g. from the primary key contraint on the table preventing duplicate subcriptions
        res.status(422).send('Already subscibed');
    }
};

// get the list of subscriptions
exports.getSubscriptions = async (req, res) => {
    let username = req.session.username;

    let twitter_handles = await getSubscriptions(username);

    res.json(twitter_handles);
};

// unsubscribe from a twitter handle
exports.unSubscribe = async (req, res) => {
    try {
        let username = req.session.username;
        let twitter_handle = req.params.twitter_handle;


        await con.asyncQuery(`
            DELETE FROM twitter_subscribers
            WHERE username=? AND twitter_handle=?`,
            [username, twitter_handle]);

        res.end();
    } catch {
        res.status(500).text('Unable to unubscribe');
    }
};

// regex for link removal
var uniRegex = new RegExp('[^\u{0000}-\u{007E}]', 'gmu');
var linkRegex = new RegExp('(https?:\/\/|www\.)\\S+', 'gmu');

// Creates a new string from the input string with all non-ASCII characters removed.
// Hex codes included to ensure ASCII formatting characters are not expunged.
// Will probably be expanded to replace hyperlinks too.
function filterForMB(inputString) {
    var newString = inputString.replace(uniRegex, '');
    newString = newString.replace(linkRegex, '<link>');
    return newString;
};

// get the subscriptions from the database 
async function getSubscriptions(username) {
    let results = await con.asyncQuery(`
        SELECT twitter_handle
        FROM twitter_subscribers
        WHERE username = ?`, username);

    let twitter_handles = [];

    for (let result of results) {
        twitter_handles.push(result.twitter_handle);
    }

    return twitter_handles;
}
