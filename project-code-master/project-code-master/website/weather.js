const fs = require('fs');
const trie = require('./trie.js');
const fetch = require("node-fetch");

const openweathermapKey = 'appid=6791fe98de0de300cf111839f27e1971';
const endpoint = 'http://api.openweathermap.org'

const request = require('request');

let weather = exports;

// a trie of city names and IDs for autocomplete
let cityTrie = new trie.Trie();

// the sql connection
let con = undefined;

// get the sql connection from parent module
weather.init = conarg => {
    con = conarg;
};

// get the forecase
async function getForecast(cityId) {
    // forecast API call
    let w = await apiCall(cityId, 'forecast');
    // transform into chosen format
    let forecast = [];
    for (let w_i of w.list) {
        let weather_at = {
            description: w_i.weather[0].description,
            time: new Date(w_i.dt_txt)
        };
        forecast.push(weather_at)
    }

    return forecast;
};

// get current weather desciption
async function getCurrent(cityId, callback) {
    let w = await apiCall(cityId, 'weather');

    return w.weather[0].description;
};

// get current weather
weather.getCurrent = async (req, res) => {
    let current = await getCurrent(cityId);
    res.send(current);
};

// get formatted weather string
weather.getFormatted = async (req, res) => {
    let username = req.session.username;
    let cityId = await getCityId(username);

    let current = await getCurrent(cityId);

    let forecast = await getForecast(cityId);

    // format as a list of "time:desciption:time:description:..."
    let result = `C:${current}:`;
    let i = 0;
    for (let fc of forecast) {
        let time = fc.time.getHours();
        time = ((time - 1) % 12) + 1;
        result += `${time}:${fc.description}:`;
        if (i > 10) {
            break;
        }
        i++;
    }
    res.send(result);
};

// load the list of cities into the trie
weather.loadCities = () => {
    return new Promise(function(resolve, reject) {
        fs.readFile('./data/city.list.json', (err, data) => {
            if (err) {
                reject(err);
                return;
            }


            let cityList = JSON.parse(data);

            for (let city of cityList) {
                cityTrie.add(city.name.toLowerCase(), {
                    id: city.id,
                    country: city.country,
                    name: city.name,
                    coord: city.coord
                });
            }
            resolve();
        });
    });;
};

// autocomplete API call
weather.autoCompleteCity = (search, limit) => {
    if (limit === undefined) {
        limit = 10;
    }
    return cityTrie.getAll(search.toLowerCase(), {
        limit: limit
    });
};

// autocomplete websocket
weather.autoCompleteCityWS = (ws, req) => {
    let node = cityTrie.root;
    let num = 0;
    let word = [];

    ws.on('message', msg => {
        let respond = true;
        let obj = JSON.parse(msg);
        switch (obj.type) {
            case 'char':
                // when the user types a new letter on the end
                let char = obj.char.toLowerCase()
                if (num === 0 && char in node.children) {
                    // descent down the trie
                    node = node.children[char];
                    word.push(char);
                } else {
                    num++;
                }
                break;
            case 'back':
                // when the user backspaces
                if (num > 0) {
                    num--;
                } else if (node.parent !== undefined) {
                    // go up the trie
                    node = node.parent;
                    word.pop();
                }
                break;
            case 'setCity':
                // when the user selects a city, set it
                setCityId(req.session.username, obj.id).catch(console.log);
                respond = false;
                break;
            case 'word':
                // when the user makes change to the word in the search box
                // other than adding a letter to the end or backspace
                // then reset the trie position
                node = cityTrie.root;
                num = 0;
                word = [];
                for (let char of obj.word) {
                    char = char.toLowerCase();
                    if (num === 0 && char in node.children) {
                        node = node.children[char];
                        word.push(char);
                    } else {
                        num++;
                    }
                }
                break;
        }
        if (respond) {
            // respond to the user with an updated list of autocomplete options
            let res = {};
            if (num == 0 && word.length != 0) {
                node.getAll("", res, word.join(''), 10);
            }
            ws.send(JSON.stringify(res));
        }
    });
};

// make a weather API call
async function apiCall(cityId, type, callback) {
    let req_url = `${endpoint}/data/2.5/${type}?id=${cityId}&${openweathermapKey}`;

    let req = await fetch(req_url);
    return req.json();
}

// set the city ID in the database
async function setCityId(username, id, callback) {
    return con.asyncQuery("UPDATE login SET city_id = '?' WHERE username = ? ;",
        [id, username]);
}

// get the city ID from the database
async function getCityId(username) {
    let query = con.asyncQuery(
        "SELECT city_id FROM login WHERE username = ? ;", username);

    let [{
        city_id
    }] = await query;

    if (city_id) {
        return city_id;
    } else {
        throw Error("expected 1 row");
    }
}
