import json
import threading
import urllib
from urllib import request
import enum
import asyncio
import time
import websockets
import sys
import serial.tools.list_ports

import io

import serial_comms.encoding as encoding

noserial = False

BAUDRATE = 9600

disable_mesh = False


class RequestType(enum.Enum):
    RETURN_PLAIN = 0
    RETURN_JSON = 1
    RETURN_RES = 2


class ProtocolType(enum.Enum):
    """
    The different types of message. See microbit/source/Serial.h for descriptions of each type.
    """
    DEBUG_PRINT = 0
    LOG_MESSAGE = 1
    LOG_RADIO = 2
    REQUEST_WEATHER = 3
    LOGIN = 4
    SEND_TWEET = 5
    REQUEST_TWEET = 6
    PING = 7
    MESH_FORWARD = 8
    DEBUG_PRINT_SERVER = 9


mbs = None


def tell_user(*args, **kwargs):
    """
    Similar to print, but
    :param args:
    :param kwargs:
    :return:
    """
    global mbs
    if noserial:
        if mbs is None:
            mbs = MicroBitSerial('noserial')

        i = io.StringIO()
        print(*args, *kwargs, file=i)
        mbs.send(9, i.getvalue(), False)
    else:
        print(*args, **kwargs)


class PortException(Exception):
    pass


class ComputerSerial:
    """
    Manages serial connections
    """

    def __init__(self, domain, baudrate=BAUDRATE):
        self.baudrate = baudrate
        self.domain = domain

        self.protocol_callbacks = []
        self.connected_ports = set()
        self.serial_connections = set()

    def communicate_one(self, port, s=None):
        """
        Connect to a port and receive messages from it
        :param port:
        :param s:
        :return:
        """
        if s is None:
            try:
                # wait for proper connection to start
                time.sleep(5)
                if port not in self.connected_ports:
                    # connect
                    s = MicroBitSerial(port, self.baudrate)
                    tell_user("microb:bit connect", port)
                    self.connected_ports.add(port)
                    self.serial_connections.add(s)
                else:
                    return
            except serial.serialutil.SerialException:
                return

        while True:
            try:
                # get a message
                protocol_num, msg = s.receive()

                # if it is a reset message, ignore it
                if protocol_num == 255:
                    tell_user("micro:bit reset")
                else:
                    # if it a real message, all the appropriate callback
                    self.call_protocol_callback(msg, s, protocol_num)
            except (serial.serialutil.SerialException, OSError):
                tell_user("microb:bit disconnect")
                self.connected_ports.remove(port)
                self.serial_connections.remove(s)

                break

    def communicate(self):
        """
        Connect to ports and communicate with them
        :return:
        """
        if noserial:
            # if communicating with a parent process over stdin/stdout, make a
            # dummy serial connection which actually uses stdin/stdout
            port = 'noserial'
            s = MicroBitSerial(port)
            self.communicate_one(port, s)

        while True:
            comports = serial.tools.list_ports.comports()
            ports = set(port.device for port in comports)
            ports.difference_update(self.connected_ports)

            for port in ports:
                try:

                    threading.Thread(target=lambda: self.communicate_one(port)).start()

                except serial.serialutil.SerialException:
                    pass

            time.sleep(0.1)

    def call_protocol_callback(self, msg, s, protocol_num):
        """
        Call a callback based on a message type number.
        :param msg: The message data
        :param s: the serial port object
        :param protocol_num: the message type
        :return: None
        """

        # get the callback
        callback, login = self.protocol_callbacks[protocol_num]
        call = False
        # if it requires login
        if login:
            # login
            username = msg.decode()

            _, pattern = s.receive()
            pattern = pattern.decode()

            success, sid = self.login(username, pattern)

            # send success to the microbit
            s.send(int(success), ' ')

            if success:
                # if login succeeded, call protocol method
                s.sid = sid

                _, msg = s.receive()
                call = True
        else:
            call = True

        if call:
            callback(msg, s)

    def login(self, username: str, pattern: str) -> (bool, str):
        """
        Sends a login request to the server
        :param username: the username with which to login
        :param pattern: the login pattern
        :return: A tuple of a a boolean for success (true is login worked) and the session cookie
        """
        data = {'username': username, 'pattern': pattern}
        data = json.dumps(data).encode('utf-8')
        req = request.Request(
            f"{self.domain}/JH-project/api/login_pattern",
            data=data)
        req.add_header("Content-Type", "application/json; charset=utf-8")

        try:
            # attempt to contact server
            res = request.urlopen(req)
            # get cookie
            cookie = res.info()['set-cookie']
            return True, cookie
        except request.HTTPError:
            return False, None

    def api_post(self, path_end, s, data, return_type=RequestType.RETURN_JSON):
        """
        Send a POST request to the API
        :param path_end: the end of the API path
        :param s: the serial object
        :param data: the data to post
        :param return_type: the type of data to return
        :return: the response from the POST request, in the requested type
        """
        session_id = s.sid

        if data is not None:
            data = json.dumps(data).encode('utf-8')

        req = request.Request(
            f"{self.domain}/JH-project/api/{path_end}",
            data=data)
        req.add_header("Content-Type", "application/json; charset=utf-8")
        req.add_header("Cookie", session_id)
        res = request.urlopen(req)
        if return_type == RequestType.RETURN_JSON:
            return json.loads(res.read(), encoding='utf-8')
        elif return_type == RequestType.RETURN_PLAIN:
            return res.read()
        elif return_type == RequestType.RETURN_RES:
            return res

    def api_get(self, path_end, s, return_type=RequestType.RETURN_JSON):
        """
        Send a GET request to the API
        :param path_end: the end of the API path
        :param s: the serial object
        :param return_type: the type of data to return
        :return: the response from the GET request, in the requested type
        """
        return self.api_post(path_end, s, None, return_type)


class MicroBitSerial:
    """
    A wrapper for the serial object which provides a better API when using messages in the chosen format
    """

    def __init__(self, port, baudrate=BAUDRATE):
        if not noserial:
            self.s = serial.Serial(port, baudrate=baudrate)
        self.sid = None

    def receive(self):
        global receive_reader_pos
        if noserial:
            # if communicating with a parent process over stdin/stdout, read from stdin
            t = input()
            l = input()
            msg = input()

            return int(t), bytes(int(b) % 256 for b in msg.split(','))
        else:
            # else read over serial
            res = encoding.receive(self.s.read)
            tell_user('received', res)
            return res

    def send(self, protocol_num, serial_data, tell=True):
        if disable_mesh:
            print('SENT DATA')
        if tell:
            tell_user("sending", (protocol_num, serial_data))
        if isinstance(serial_data, str):
            enc = serial_data.encode()
        else:
            enc = serial_data
        if noserial:
            # if communicating with a parent process over stdin/stdout, write to stdout
            print(protocol_num)
            print(len(serial_data))
            print(','.join(str(b) for b in enc))
        else:
            encoding.send(self.s.write, protocol_num, enc[:100])


class ComputerSerialProtocol(ComputerSerial):
    """
    The protocol for microbit communication
    """
    def __init__(self, domain):
        super().__init__(domain)

        # set the callbacks
        self.protocol_callbacks = [(self.debug_print, False),
                                   (self.log_message, True),
                                   (self.log_radio, False),
                                   (self.request_weather, True),
                                   (self.request_login, True),
                                   (self.log_twitter, True),
                                   (self.request_twitter, True),
                                   (self.ping, False),
                                   (self.mesh_forward, False),
                                   (self.debug_print_server, True)]

        self.mesh_interface = None

    def debug_print(self, msg, s: MicroBitSerial):
        # print something to the console for debugging
        tell_user('DEBUG', msg.decode())

    def debug_print_server(self, msg, s: MicroBitSerial):
        # print something to the console for debugging, following the pattern for server communication
        tell_user('\nDEBUG:', msg.decode())
        s.send(0, ' ')

    def log_message(self, msg, s):
        # log a message to the message history
        msg = msg.decode('utf-8')

        sender_id, msg = msg.split(',')

        data = {
            'msg': msg,
            'sender_microbit_id': int(sender_id)
        }
        self.api_post('message-log', s, data, RequestType.RETURN_RES)

        s.send(0, ' ')

    def log_radio(self, msg, s):
        # print the data as binary bytes - useful for radio debugging
        tell_user([bin(i) for i in msg])

    def request_weather(self, msg, s: MicroBitSerial):
        # when the microbit requests the weather contact the server return the data
        msg = self.api_get('weather', s, RequestType.RETURN_PLAIN).decode()

        msg = msg.split(':')[1]
        s.send(0, msg)

    def request_login(self, msg, s: MicroBitSerial):
        # when the microbit logs in, no additional data needs to be sent
        s.send(0, ' ')

    def log_twitter(self, msg, s: MicroBitSerial):
        # send a twitter message
        msg = msg.decode('utf-8')
        try:
            self.api_post('twitter', s, {'msg': msg}, RequestType.RETURN_PLAIN)
        except urllib.error.HTTPError:
            pass
        s.send(0, ' ')

    def request_twitter(self, msg, s: MicroBitSerial):
        # request a tweet
        msg = self.api_get('twitter', s, RequestType.RETURN_PLAIN).decode()

        s.send(0, msg)

    def ping(self, msg, s: MicroBitSerial):
        # the microbit sends a ping message when it turns on. Reply to this ping message, so that
        # the microbit know that it is connected to serial.
        if not disable_mesh:
            s.send(0, ' ')

    def mesh_forward(self, msg, s: MicroBitSerial):
        # forward radio mesh network data to the server
        if noserial or disable_mesh:
            tell_user('no forward')
            return

        # since this is async, it cannot be directly called from non-async
        async def inner(msg=msg):
            data = {'msg': list(msg)}

            await self.mesh_interface.send(json.dumps(data))

        asyncio.run_coroutine_threadsafe(inner(), loop)

    async def make_ws(self):
        """
        Connect to the mesh_interface websocket.
        :return: False if a fatal error occurred, otherwise True
        """
        if disable_mesh or noserial:
            return True
        isHttps = self.domain.split(':')[0] == 'https'
        protocol = 'wss' if isHttps else 'ws'
        domain = ':'.join(self.domain.split(':')[1:])
        url = f'{protocol}:{domain}/JH-project/api/mesh_interface'
        try:
            self.mesh_interface = await websockets.connect(url)
        except websockets.exceptions.InvalidStatusCode:
            return False
        return True

    async def ws_recv_loop(self):
        """
        Receive messages from the mesh_interface websocket, and forward them over serial
        :return:
        """
        if self.mesh_interface is None:
            tell_user('no recv loop')
            return
        async for msg in self.mesh_interface: # for each incoming websocket message
            # send over serial
            data = (int(b) % 256 for b in msg.split(','))
            data = bytes(data)

            for s in self.serial_connections:
                s.send(ProtocolType.MESH_FORWARD.value, data)


if __name__ == '__main__':
    # process args
    if len(sys.argv) > 1:
        if sys.argv[1] == 'noserial':
            noserial = True
        elif sys.argv[1] == 'nomesh':
            disable_mesh = True
        else:
            sys.exit(1)

    # main function
    async def main():
        # s = ComputerSerialProtocol('http://localhost:8080')
        s = ComputerSerialProtocol('https://oas.host.cs.st-andrews.ac.uk')
        success = await s.make_ws()

        if not success and not noserial:
            tell_user('Error: unable to connect to the server.')
            return
        # start connecting to serial
        threading.Thread(target=s.communicate).start()

        # contact server
        if not noserial:
            await s.ws_recv_loop()

    # start event loop
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
