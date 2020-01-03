let log = $('#log');

// append a message to the DOM
function addMessage(a) {
    let time = new Date(a.time_received);
    let timeStr = time.toLocaleDateString();
    timeStr += ` ${time.getHours()}:${time.getMinutes()}:${time.getSeconds()}`

    let row = $(`<tr></tr>`);
    row.append($(`<td>${a.sender_microbit_id}</td>`));
    row.append($(`<td>${timeStr}</td>`));
    row.append($(`<td class="log-message">${a.message}</td>`));

    log.append(row);
}

// append many messages to the DOM
function updateList(arr) {
    for (let a of arr) {
        addMessage(a);
    }
}

// get the message log from the server and connect to the websocket
function getLog(wsEndpoint) {
    let protocol = window.location.protocol === 'http:' ? 'ws:' : 'wss:';
    let ws = new WebSocket(`${protocol}//${location.host}/${wsEndpoint}`);

    $.ajax({
        url: '/JH-project/api/message-log',
        type: "GET",
        contentType: "application/json; charset=utf-8"
    }).done(updateList).fail(console.log);

    ws.onmessage = (msg) => {
        console.log(msg.data)
        addMessage(JSON.parse(msg.data));
    };
}

// clear all items from the log, both server-side and from the DOM
function clearLog() {
    console.log("clear log clicked");
    $.ajax({
        url: '/JH-project/api/message-log',
        type: 'DELETE',
        contentType: "text/plain; charset=utf-8",
        dataType: "text",
    }).done(() => log.empty()).fail(console.log);
}

getLog('JH-project/api/message-ws');
