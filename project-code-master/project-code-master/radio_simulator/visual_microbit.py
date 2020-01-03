import math

import radio_simulator.microbit as mb
from radio_simulator.config import *


class VisualMicrobit:
    """
    A radio microbit with a cartesian coordinate
    """
    microbit_id = 1

    def __init__(self, x, y):
        self.x = x
        self.y = y
        self._mb = mb.RadioMicrobit(VisualMicrobit.microbit_id, self.get_in_range)

        VisualMicrobit.microbit_id += 1

    def get_in_range(self, id):
        for m in microbits:
            dist = math.hypot(self.x - m.x, self.y - m.y)
            if dist > RADIO_RANGE:
                continue
            if m.mb.id == self.mb.id:
                continue
            strength = dist / RADIO_RANGE * 255
            yield m.mb, int(strength)

    @property
    def mb(self) -> mb.RadioMicrobit:
        return self._mb


def set_microbits(mbs):
    global microbits
    microbits = mbs
