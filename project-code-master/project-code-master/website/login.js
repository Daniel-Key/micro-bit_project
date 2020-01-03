const bcrypt = require('bcrypt');

let con = undefined;

let login = exports;

// get database connection from parent process
login.init = (conarg) => {
    con = conarg;
};

// register a user account
login.register = async (req, res) => {
    // case insensitive
    let username = req.body.username.toLowerCase();
    let password = req.body.password;

    // disallow too short username or password
    if (username.length < 3) {
        res.status(401).json("Bad username: too short");
        return;
    }

    if (password.length < 3) {
        res.status(401).json("Bad password: too short");
        return;
    }

    // disallow non-alphabet characters
    for (let letter of username) {
        letter = letter.toLowerCase();
        if (letter < 'a' || letter > 'z') {
            res.status(401).json("Bad username: can only contain alphabet letters.");
            return;
        }
    }

    // get the number of users with this username
    let query = con.asyncQuery(
        "SELECT count(*) as num_matches FROM login WHERE username = ?;",
        username);

    let [{
        num_matches
    }] = await query;

    // if no other user has claimed this username
    if (num_matches === 0) {
        // hash the username and a default unlock pattern
        let hash = await bcrypt.hash(password, 10);
        let pattern_hash = await bcrypt.hash('14563', 10);
        // store in the database
        try {
            // store login info
            await con.asyncQuery(
                `INSERT INTO login (username,password,unlock_pattern)
			VALUES (?,?,?);`,
                [username, hash, pattern_hash]);

            // default twitter subscription
            await con.asyncQuery('INSERT INTO twitter_subscribers VALUES (?, ?)',
                [username, 'univofstandrews']);

            res.json("Registration successful");
        } catch {
            res.status(401).json("Invalid username: possibly too long");
        }
    } else {
        res.status(401).json("Username taken");
    }
};

// sign a user into their account
login.signIn = async (req, res) => {
    // case insensitive
    let username = req.body.username.toLowerCase();
    let password = req.body.password;

    // get hashed password
    let [result] = await con.asyncQuery(
        "SELECT password FROM login WHERE username = ?;", username);

    // if no such user
    if (!result) {
        res.status(401).json("Incorrect username or password");
        return;
    }

    // check if the password is correct
    let passwordCorrect = await bcrypt.compare(password, result.password);
    if (passwordCorrect) {
        // if correct, attach the username to the session object which will be
        //preseved for other api calls
        req.session.username = username;
        res.json("Correct");
    } else {
        res.status(401).json("Incorrect username or password");
    }
};

// login using the lock pattern
login.loginPattern = async (req, res) => {
    try {
        // case insensitive
        let username = req.body.username.toLowerCase();
        let pattern = req.body.pattern;

        // get hashed lock pattern
        let [result] = await con.asyncQuery(
            "SELECT unlock_pattern FROM login WHERE lower(username) = ?;", username);

        // if no such user
        if (!result) {
            res.status(401).json("Authentication error");
            return;
        }

        // if password correct
        let passwordCorrect = await bcrypt.compare(pattern, result.unlock_pattern);
        if (passwordCorrect) {
            // if correct, attach the username to the session object which will be
            //preseved for other api calls
            req.session.username = username;
            res.json("Session number?");
        } else {
            res.status(401).json("Authentication error");
        }
    } catch (e) {
        res.status(501).json("Server error");
    }
}

// check if logged in
login.loggedin = (req, res) => {
    if (req.session.username) {
        res.json(true);
    } else {
        res.json(false);
    }
};

// sign out
login.signout = (req, res) => {
    delete req.session.username;
    res.status(200).json("Logged out");
};
