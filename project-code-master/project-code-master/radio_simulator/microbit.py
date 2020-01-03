import subprocess
import threading
import os
import time
from enum import Enum, IntEnum
import serial_comms.key_generator as key_generator


# the types of messages sent between the mock microbit and this
class Type(IntEnum):
    EXIT = 0
    RECV = 1
    SEND = 2
    RECV_MSG = 3
    SEND_MSG = 4
    PRINT = 5


executable_name = ""


class Microbit:
    """
    Encapsulates the mock microbit binary
    """

    def __init__(self, _id):
        # get the absolute path to the binary
        path = os.path.join(__file__, '..', 'mock_microbit', executable_name)
        path = os.path.abspath(path)
        # open the binary in a child process
        PIPE = subprocess.PIPE
        self.p = subprocess.Popen([path], stdin=PIPE, stdout=PIPE, stderr=PIPE)

        # tell the microbit its ID
        self.p.stdin.write(f'{_id}\n'.encode())

        keys = key_generator.generate_keys()
        n, private = keys['private']
        n, public = keys['public']

        # tell the microbit its encryption keys
        self.p.stdin.write(f'{private}\n'.encode())
        self.p.stdin.write(f'{public}\n'.encode())
        self.p.stdin.write(f'{n}\n'.encode())
        self.p.stdin.flush()

        self.alive = True
        self.id = _id

        self.write_lock = threading.Lock()

        # start reading from the microbit in another thread
        threading.Thread(target=self.read_loop).start()

    def write(self, _type: Type, msg: bytes) -> None:
        """
        Write a message to the mock microbit
        :param _type: the message type
        :param msg: the msg to send
        :return: None
        """
        with self.write_lock:  # lock with a mutex to prevent data races
            if not all(isinstance(i, int) for i in msg):
                raise TypeError(f'expected a bytes-like object, {type(msg).__name__} found')
            # write a message in the format "[type]\n[length]\n[byte0],[byte1],[...]\n"
            self.p.stdin.write(f'{_type.value}\n'.encode())
            self.p.stdin.write(f'{len(msg)}\n'.encode())

            msg_bytes = b','.join(str(byte).encode() for byte in msg) + b'\n'
            self.p.stdin.write(msg_bytes)
            try:
                self.p.stdin.flush()
            except BrokenPipeError:
                print(f'mb {self.id} died')

    def read(self) -> (Type, bytes):
        """
        Read a message from the microbit
        :return: a type of the message type and content
        """

        # a helper function that reads a line from process stdout
        def read_line():
            m = b''
            while True:
                c = self.p.stdout.raw.read(1)
                if c == b'\n':
                    return m
                else:
                    m += c

        # reads a message in the format "[type]\n[length]\n[byte0],[byte1],[...]\n"
        _type = int(read_line())
        _len = int(read_line())
        if _len > 0:
            msg = read_line()
            try:
                msg = bytes(int(i) % 256 for i in msg.split(b','))
            except Exception as e:
                print('msg:', msg)
                raise e
        else:
            msg = b''

        return Type(_type), msg

    def on_signal(self, msg):
        pass

    def read_loop(self) -> None:
        """
        Reads data from the child process and calls the callback
        :return: None
        """
        while self.alive:
            read = self.read()
            self.on_signal(read)

    def close(self) -> None:
        """
        Terminate the child process
        :return: None
        """
        self.alive = False
        self.write(Type.EXIT, b'')


class RadioMicrobit(Microbit):
    """
    Adds functionality specifically for radio-related communication
    """

    def __init__(self, _id: int, get_in_range: callable):
        """
        Initialiser
        :param _id: the microbit ID
        :param get_in_range: a function which takes the microbit ID and returns an iterable of all in-range microbits
        """
        self.get_in_range = get_in_range
        super().__init__(_id)

    def send_message(self, recipient: int, msg: bytes) -> None:
        """
        Send a message to another microbit using the protocol
        :param recipient: The recipient ID
        :param msg: the message data
        :return: None
        """
        if not isinstance(msg, bytes):
            raise TypeError('excpected bytes, got ' + type(msg).__name__)
        self.write(Type.SEND_MSG, bytes([recipient]) + msg)  # tell the microbit to send a message

    def on_signal(self, msg: (Type, bytes)) -> None:
        """
        When a signal is received from the microbit
        :param msg: The signal
        :return: None
        """
        _type, msg = msg
        if _type == Type.RECV_MSG:
            self.on_message(msg)
            return

        if _type == Type.PRINT:
            print('print', self.id, msg.decode(), end='')
            return

        if _type == Type.EXIT:
            return

        if _type != Type.SEND:
            raise Exception(f"unexpected type: {str(_type)}")

        print(f'mb {self.id} sent: {msg}')

        for mb, strength in self.get_in_range(self.id):
            if mb is self:
                continue
            mb.write(Type.RECV, bytes([strength]) + msg)

    def on_message(self, msg: bytes) -> None:
        """
        When a message addressed to this microbit is received
        :param msg: The raw data from the mock microbit
        :return: None
        """
        sender = msg[0] + (msg[1] << 8) + (msg[2] << 16) + (msg[3] << 24)
        msg = msg[4:].decode().split('\0')[0]
        print(f'mb {self.id} got from {sender}: {msg}')


def set_executable(e):
    """
    Set the mock microbit binary location
    :param e:
    :return:
    """
    global executable_name
    executable_name = e


if __name__ == '__main__':
    # get two microbits which are in range of each other
    mbs = []

    mb1 = RadioMicrobit(1, lambda x: mbs)
    mb2 = RadioMicrobit(2, lambda x: mbs)

    mbs.append((mb1, 0))
    mbs.append((mb2, 0))

    # tell microbit 2 to send a message to microbit 1
    mb1.send_message(2, b'hello')

    # wait for it to complete, then end
    time.sleep(1)

    mb1.close()
    mb2.close()
