// this file allows command line access to the database

const mysql = require('mysql');
const readline = require('readline');

const rl =
    readline.createInterface({
        input: process.stdin,
        output: process.stdout
    });



var con = mysql.createConnection({
    host: "dgk2.host.cs.st-andrews.ac.uk",
    user: "dgk2",
    password: "jh2834WE.K!2eV",
    database: "dgk2_cs3099_JH_db"
});


// does a database query, displays the output
// and prints the next start-of-line prompt
function doQuery(answer) {
    con.query(answer, [], (err, res) => {
        if (err) {
            console.log(err.sqlMessage);
        } else {
            displayData(res);
        }

        process.stdout.write("> ");
    });
}

// display some data from an SQL response
function displayData(data) {
    // if the query was not a SELECT, print number of rows changed
    if (data.constructor.name == 'OkPacket') {
        console.log(`Ok. ${data.affectedRows} rows changed.`);
        console.log();
        return;
    }
    // if it was a SELECT, if there were 0 rows returned, print "0 rows"
    if (data.length == 0) {
        console.log("0 rows.");
        console.log();
        console.log();
        return;
    }
    // if there was data, print it in a table format
    let columns = [];
    for (let i in data[0]) {
        columns.push(i);
    }
    // find the widths of each column
    let colLengths = {};
    for (let column of columns) {
        let colLength = column.length;
        for (let row of data) {
            let len = String(row[column]).length;
            if (len > colLength) {
                colLength = len;
            }
        }
        colLengths[column] = colLength;
    }
    // prints a horizontal line in the table
    function horizontalLine() {
        process.stdout.write('+');
        for (let column of columns) {
            for (var i = 0; i < colLengths[column] + 2; i++) {
                process.stdout.write('-');
            }
            process.stdout.write('+');
        }
        console.log();
    }
    // makes a row in the columns
    function writeRow(row) {
        process.stdout.write('|');
        for (let column of columns) {
            process.stdout.write(' ');
            process.stdout.write(String(row[column]));
            let padding = colLengths[column] - String(row[column]).length;
            for (var i = 0; i < padding; i++) {
                process.stdout.write(' ');
            }
            process.stdout.write(' |');
        }
        console.log();
    }
    // the headers of the table
    horizontalLine();
    let headerRow = {};
    for (let column of columns) {
        headerRow[column] = column;
    }
    writeRow(headerRow);
    horizontalLine();
    // the data in the table
    for (let row of data) {
        writeRow(row);
    }
    horizontalLine();

    console.log(`${data.length} rows in set.`);
    console.log();
}

// connect to the database
con.connect(function(err) {
    if (err) {
        throw err;
    }
    // get stdin, split into queries based on semi-colons, and submit each query
    process.stdout.write("> ");

    let lingeringQuery = '';
    rl.on('line', input => {
        let queries = input.split(';');

        queries[0] = lingeringQuery + queries[0];

        lingeringQuery = queries.pop();

        queries.forEach(query => {
            doQuery(query);
        });
    });
});
