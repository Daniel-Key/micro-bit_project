import math

import radio_simulator.visual_microbit as vm
from radio_simulator.config import *

tools = []


class ToolMeta(type):
    """
    Manages tool numbering
    """
    tool_id = 0

    def __new__(cls, *args, **kwargs):
        _cls = super().__new__(cls, *args, **kwargs)
        _cls._value = ToolMeta.tool_id
        ToolMeta.tool_id += 1
        tools.append(_cls())

        return _cls


class Tool:
    """
    Tool superclass
    """

    @property
    def value(self):
        return type(self)._value

    @property
    def name(self):
        name = type(self).__name__
        return name.lower().replace('_', ' ')

    def click(self, e, app):
        pass

    def move(self, e, app):
        pass


# tool to create new microbits
class NEW_MB(Tool, metaclass=ToolMeta):
    def click(self, e, app):
        m = vm.VisualMicrobit(e.x, e.y)
        microbits.append(m)


# tool to move microbits
class MOVE(Tool, metaclass=ToolMeta):
    def __init__(self):
        super().__init__()
        self.m = None

    def click(self, e, app):
        self.m = get_clicked(e)

    def move(self, e, app):
        if self.m is None:
            return
        self.m.x, self.m.y = e.x, e.y


# tool to send a message from a microbit
class SEND_MESSAGE(Tool, metaclass=ToolMeta):
    def click(self, e, app):
        m = get_clicked(e)
        if m is not None:
            app.send_message_gui(m)


def get_clicked(e):
    """
    Helper function to get the microbit object which has been clicked
    :param e: a click event
    :return: the VisualMicrobit object
    """
    for m in microbits:
        if math.hypot(e.x - m.x, e.y - m.y) < MICROBIT_RADIUS:
            return m


def set_microbits(mbs: list) -> None:
    """
    Set the microbits list
    :param mbs: list of microbits
    :return: None
    """
    global microbits
    microbits = mbs
    vm.set_microbits(microbits)


__all__ = ['NEW_MB', 'MOVE', 'SEND_MESSAGE', 'set_microbits', 'tools']
