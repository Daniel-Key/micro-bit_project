import time

import math


def receive(reader:callable)->(int,bytes):
    """
    Receive a serial message
    :param reader: a function which reads data from serial
    :return: a tuple of the data
    """
    protocol_num = b''
    c = reader(1)
    if c == b'\xff':
        return 255, None
    while c != b'\n':
        protocol_num += c
        c = reader(1)

    length = b''
    c = reader(1)
    while c != b'\n':
        length += c
        c = reader(1)
    return int(protocol_num), reader(int(length))


def send(writer, protocol_num, msg):
    """
    Sends a message over serial
    :param writer: a function which writes to serial
    :param protocol_num: the type of message
    :param msg: the message
    :return:
    """
    writer(str(protocol_num).encode())
    writer(b"\n")
    writer(str(len(msg)).encode())
    writer(b"\n")
    time.sleep(0.1)
    writer(msg)
