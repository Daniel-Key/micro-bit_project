const {
    spawn
} = require('child_process');

const {
    Buffer
} = require('buffer');
const WaitQueue = require('wait-queue');

let serial_computer_received = new WaitQueue();

const meshInterface = exports;

const DEBUG = false;

// a child process to communicate with via messages in the format:
// "[type]\n[length]\n[byte0decimal],[byte1decimal],[byte2decimal],[...]\n"
class ChildProcess {
    constructor(executable, args, options) {
        this.executable = executable;
        this.args = args;
        this.options = options;


        this.connect();
    }

    connect() {
        this.process = spawn(this.executable, this.args, this.options);

        let buf = Buffer.alloc(0);
        this.process.stdout.on('data', data => {
            // concatenate data into buffer
            buf = Buffer.concat([buf, data]);

            while (true) {
                // if there is a complete message with 3 lines in it
                let newLine1 = buf.indexOf('\n');
                let newLine2 = buf.indexOf('\n', newLine1 + 1);
                let newLine3 = buf.indexOf('\n', newLine2 + 1);
                if (!new Set([newLine1, newLine2, newLine3]).has(-1)) {
                    // get the entire message
                    data = buf.slice(0, newLine3);
                    // get the indavidual lines
                    let [type, len, msg] = data.toString().split('\n');
                    type = Number(type);
                    // convert the bytes string into an array
                    let msg_bytes = JSON.parse(`[${msg}]`);
                    if (DEBUG) {
                        console.log(`READ ${this.executable}:\n${type}\n${msg_bytes.length}\n${msg}`);
                    }
                    // call callback with the data
                    this.onDataCallback(type, msg, msg_bytes);
                    // remove the message from the buffer
                    buf = buf.slice(newLine3 + 1, buf.length);
                } else {
                    break;
                }
            }

        });
    }
    // data callback to be overriden by subclasses
    onDataCallback(type, msg, msg_bytes) {

    }

    // write a message in the specified format
    write(type, msg) {
        if (DEBUG) {
            console.log(`WRITE ${this.executable}:`);
            console.log(`${type}`);
            console.log(`${msg.length}`);
        }
        // convert buffer to array of numbers
        if (Buffer.isBuffer(msg)) {
            let newMsg = msg.values();
            msg = [];
            for (let val of newMsg) {
                msg.push(val);
            }
        }
        // prevent 0-length messages
        if (msg.length === 0) {
            msg = [32];
        }
        if (DEBUG) {
            console.log(msg.join());
        }
        try {
            // write data
            this.process.stdin.write(`${type}\n`);
            this.process.stdin.write(`${msg.length}\n`);
            msg = msg.join();
            this.process.stdin.write(`${msg}\n`);
        } catch (e) {
            console.log('write error!');
            // restart upon error
            this.connect();
        }
    }
}

// simulate the microbit radio protocol on the server
class MockMesh extends ChildProcess {
    constructor() {
        super('./data/mock_mesh');
    }

    onDataCallback(type, msg, payload) {
        // on data:
        if (type === 3) {
            // if a message is received from a microbit:
            this.onMeshMessage(payload).catch(console.log);
        } else {
            // if a radio message should be sent to microbits, relay over websockets.
            for (let ws of connected) {
                ws.send(msg);
            }
        }
    }

    // respond to a mesh message
    async onMeshMessage(payload) {
        // gets the decoded packet
        let {
            username,
            pattern,
            type,
            inner_payload,
            sender
        } = this.decodePacket(payload);

        // debug print
        if (DEBUG) {
            console.log({
                username,
                pattern,
                type,
                inner_payload
            });
        }

        // login to the serial_computer
        serial_computer.write(type, Buffer.from(username));
        serial_computer.write(0, Buffer.from(String(pattern)));
        // get login success response
        let [success] = await serial_computer_received.shift();

        if (DEBUG) {
            console.log(`login ${success?'success':'fail'}`);
        }
        if (success) {
            // if login success, then write message payload
            serial_computer.write(0, Buffer.from(inner_payload));
            // get response
            let [sc_msg_type, msg] = await serial_computer_received.shift();
            // encode response
            let sender_byte = Buffer.from([sender, 1]);
            let msg_bytes = Buffer.from(msg);
            // send over mesh network
            mock_mesh.write(4, Buffer.concat([sender_byte, msg_bytes]));
        } else {
            // if login failed, then respond with failure message
            mock_mesh.write(4, Buffer.from([sender, 0, 0]));
        }
    }

    // decode a request packet
    decodePacket(payload) {
        payload = Buffer.from(payload);

        // read sender id
        let ptr = 0;
        let sender = payload.readInt32LE(ptr);
        ptr += 4;
        // read username followed by \n
        let end = payload.indexOf('\n', ptr);
        let username = payload.slice(ptr, end).toString();
        ptr = end;
        ptr++;
        // read pattern
        let pattern = payload.readInt32LE(ptr);
        ptr += 4;
        // read type
        let type = payload.readInt32LE(ptr);
        ptr += 4;
        end = payload.indexOf(0, ptr);
        if (end == -1) {
            end = payload.length;
        }
        // read inner payload
        let inner_payload = payload.slice(ptr, end).toString();

        return {
            username,
            pattern,
            type,
            inner_payload,
            sender
        };
    }
}

// simulate python program to repurpose serial connnection logic to work from the server
class SerialComputer extends ChildProcess {
    constructor() {
        let options = {
            cwd: '..'
        };
        super('python3', ['-m', 'serial_comms.serial_computer', 'noserial'], options);
    }
    onDataCallback(type, msg, msg_bytes) {
        // on receiving data, if it is of type 'log message', then log it
        if (type === 9) {
            console.log(`SERIAL LOG: ${Buffer.from(msg_bytes).toString()}`);
        } else {
            // otherise enqueue it for use elsewhere
            serial_computer_received.push([type, msg_bytes]);
        }
    }
}

const mock_mesh = new MockMesh();
const serial_computer = new SerialComputer();

meshInterface.mock_mesh = mock_mesh;

// the set of connected websockets
let connected = new Set();

// get database connection from parent module and get encryption keys
meshInterface.init = con => {
    async function init(con) {
        let query = con.asyncQuery('SELECT * FROM encryption_keys WHERE id=4');
        let [{
            n,
            e,
            d
        }] = await query;

        mock_mesh.process.stdin.write(`4\n${d}\n${e}\n${n}\n`);
    }
    init(con).catch(console.log);
}

// when a websocket connects
meshInterface.getWs = (ws, req) => {
    // add it to the set
    connected.add(ws);
    ws.on('message', msg => {
        // get websocket messages and send them to the mesh protocol simulator
        // as incoming radio
        msg = JSON.parse(msg).msg;

        msg.unshift(0);
        mock_mesh.write(1, msg);
    });
    ws.on('close', () => {
        connected.delete(ws);
    });
};
