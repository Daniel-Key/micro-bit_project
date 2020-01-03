import tkinter

import radio_simulator.microbit as mb
import radio_simulator.tools as tools

from radio_simulator.config import *
import radio_simulator.visual_microbit as vm

microbits = []
tools.set_microbits(microbits)


class InteractiveRadio(tkinter.Tk):
    """The simulator GUI"""

    def __init__(self):
        super().__init__()
        # create canvas and bind event handlers
        self.canvas = tkinter.Canvas(self, width=800, height=500)
        self.canvas.grid(row=0)

        self.canvas.bind('<Button-1>', self.click)
        self.canvas.bind('<B1-Motion>', self.move)

        frame = tkinter.Frame(self)
        frame.grid(row=1)

        # create buttons for tools
        self.tools = []
        for tool in tools.tools:
            button = tkinter.Button(frame, text=tool.name, command=lambda _tool=tool: self.button_click(_tool))
            button.grid(row=0, column=tool.value)
            self.tools.append(button)

        # set initial tool
        self.button_click(tools.NEW_MB())

        self.draw()
        # close gracefully when close button pressed
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

    def on_closing(self):
        # close gracefully when close button pressed
        for m in microbits:
            m.mb.close()
        self.destroy()

    def button_click(self, tool):
        # when a tool button is clicked, select that tool and update GUI
        for i, button in enumerate(self.tools):
            button['bg'] = '#5555ff' if i == tool.value else 'grey'
        self.tool = tool

    def circle(self, x, y, r, **kwargs):
        # convenience function for circles
        self.canvas.create_oval(x - r, y - r, x + r, y + r, **kwargs)

    def draw(self):
        # re-draw canvas
        canvas = self.canvas
        canvas.delete(tkinter.ALL)
        for m in microbits:
            # draw microbit circle
            self.circle(m.x, m.y, MICROBIT_RADIUS, fill='red')
            # draw radio range circle
            self.circle(m.x, m.y, RADIO_RANGE)
            canvas.create_text(m.x, m.y, text=m.mb.id)

    def click(self, e):
        # when the canvas is clicked, do what the current tool says
        self.tool.click(e, self)

        self.draw()

    def move(self, e):
        # when the mose is dragged, do what the current tool says
        self.tool.move(e, self)
        self.draw()

    def send_message_gui(self, sender: vm.VisualMicrobit):
        # creates the GUI window to send a message to another microbit
        top = tkinter.Toplevel(self)
        top.title(f"Send a message from mb {sender.mb.id}")
        # the labels
        l1 = tkinter.Label(top, text='Recipient')
        l2 = tkinter.Label(top, text='Message')

        l1.grid(row=0, column=0)
        l2.grid(row=1, column=0)

        # the text boxes
        recipient_entry = tkinter.Entry(top)
        recipient_entry.grid(row=0, column=1)

        msg_entry = tkinter.Entry(top)
        msg_entry.grid(row=1, column=1)

        # if something went wrong, an alert label
        alert = tkinter.Label(top, fg='red')
        alert.grid(row=3, column=0, columnspan=2)

        # when the button is clicked...
        def send():
            try:
                recipient = int(recipient_entry.get())
            except ValueError:
                alert['text'] = 'Recipient must be an integer'
                return
            # tell the microbit to send a message
            sender.mb.send_message(recipient, msg_entry.get().encode())
            top.destroy()

        send_button = tkinter.Button(top, text='Send', command=send)
        send_button.grid(row=2, column=0, columnspan=2, sticky='EW')


if __name__ == '__main__':
    mb.executable_name = 'mock_mesh'
    app = InteractiveRadio()

    app.mainloop()
