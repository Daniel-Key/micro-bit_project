const bcrypt = require('bcrypt');

let patternLock = exports;

let con = undefined;

patternLock.init = conarg => {
    con = conarg;
};

patternLock.setPattern = async (req, res) => {
    let user = req.session.username;
    let pattern = String(req.body.pattern);

    let pattern_hash = await bcrypt.hash(pattern, 10);

    await con.asyncQuery(
        'UPDATE login SET unlock_pattern=? WHERE username=?',
        [pattern_hash, user]);

    res.json("pattern set");
};
