#!/usr/bin/python3
"""
Remote access to XPack QEMU Washing Machine Simulator

Connect opens connection to `host:8888` for diagnostic
messages.
- Disconnect disconnects but leaves QEMU running.
- Halt stops QEMU and disconnects.
- Quit/Window close halts QEMU before closing.

Timer task sends query commands to poll the status of
GPIOD enabled bit; if enabled then polls `moder` and `idr`
registers to and displays their contents.

Accepts asynchronous `odr` register changes and updates
graphic display accordingly. On reconnect to QEMU will use
the polled values to resync the display with the current
GPIOD status.

ALl buttons are supported with correct latched behaviour for
the PS keys and the door open button toggles on and off. The
motor sensor is simulated by clicking on the motor wheel.

The default sate for the QEMU emulator is all output pins are
zero which means at startup the graphic display does not match
the real hardware where all pins are pulled high. This is not
normally a problem but be aware that:
   * writing a zero to the 7-Segment display as the first
     output value will not be displayed as the emulator does
     not detect an output pin change
   * latch is off so PS keys do not stay high
   * Logic of door open pin is inverted as high is considered open

See project README.md for disclaimer and additional information.
Feabhas Ltd
"""
import configparser
import socket
import threading
import zipfile
from collections import namedtuple
from dataclasses import dataclass
from enum import Enum
import tkinter as tk
from datetime import datetime
from pathlib import Path
import select
from tkinter import ttk, messagebox, simpledialog
from typing import AnyStr, Optional, List, Dict

__VERSION__ = '3.0.0'

# timeout on socket - devcontainer ports seem to be open after client shuts down
QEMU_TIMEOUT = 3.0
# polling interval in ms
POLL = 100
# how often in secs to check if devices still enabled
DEVICE_POLL = 5000
# time in secs to display warnings
DISPLAY_WARN = 5000
# window size
GEOMETRY = ('946x494', '946x682')
# virtual event for listeners
QEMU_MESSAGE = '<<qemu>>'
# settings file
SETTINGS_INI = Path('~/qemu_gui.ini').expanduser()
SETTINGS = 'Settings'

SCRIPT_HOME = Path(__file__).parent

Connect = Enum('Connect', ['disconnected', 'connecting', 'connected'])


class ButtonStyle:
    """ Identifies different button behaviours"""
    plain = 1
    latch = 2
    toggle = 3


Button = namedtuple('Button', 'name pin, style down up x y radius')
Overlay = namedtuple('Overlay', 'id x y images')
Animation = namedtuple('Animation', 'pin_map_name, poll, direction overlay', defaults=(None, None))


class BoardError(Exception):
    """ Used to wrap errors for popup messages"""
    pass


class PinMap:
    def __init__(self, pins: List[int], reset: int, image: Optional[str]):
        self.pins = pins
        self.reset = reset
        self.image = image
        self.base_pin = pins[0]
        self.state = 0
        self.sprite = 0
        self.poll = 0

    def update_board(self, display, overlays: Dict[str, Overlay], pin: int, level: int, show: bool):
        bit = pin - self.base_pin
        if level:
            self.state |= 1 << bit
        else:
            self.state &= ~(1 << bit)
            self.sprite = self.reset
        index = self.state if self.state else self.reset
        if self.image and show:
            display.update_image(overlays[self.image], index)

    def animate_board(self, display, overlays: Dict[str, Overlay], update: int, backwards: bool=False, overlay: Optional['PinMap']=None):
        if not self.state:
            return
        # print('anim', self.image)
        self.poll += 1
        if self.poll < update:
            return
        self.poll = 0
        if not overlays:
            return
        limit = len(overlays[self.image].images)
        if not backwards:
            self.sprite = (self.sprite + 1) % limit
        elif not self.sprite:
            self.sprite = limit - 1
        else:
            self.sprite = self.sprite - 1
        display.update_image(overlays[self.image], self.sprite)
        if overlay:
            display.update_image(overlays[overlay.image], overlay.state)


class STM32:
    rcc_ahbenr = b'M40023830? '
    rcc_apb1enr = b'M40023840? '

    led_overlays = {
        # LED keys must match the led_names
        'led-0': Overlay(0, 207, 298, ['led-0.png', 'led-1.png']),
        'led-1': Overlay(1, 217, 298, ['led-0.png', 'led-1.png']),
        'led-2': Overlay(2, 227, 298, ['led-0.png', 'led-1.png']),
        'led-3': Overlay(3, 237, 298, ['led-0.png', 'led-1.png']),
    }

class WmsBoard:
    """ Static class for GUI layout configuration """
    graphics_lib = 'graphics'
    graphics_zip = 'qemu-wms-graphics.zip'
    board_image = 'feabhas-wms-768.png'
    icon = 'qemu-wms-icon.png'
    image_x = 768
    image_y = 356

    checkbox_labels = 'Led A', 'Led B', 'Led C', 'Led D', 'Motor', 'Dir', 'Latch'

    buttons = [
        Button('reset', None, ButtonStyle.plain, '', 'reset ', 32, 200, 8),
        Button('door', 0, ButtonStyle.toggle, 'D0L0 ', 'D0d0 ', 730, 310, 15),
        Button('PS1', 1, ButtonStyle.latch, 'D0l1 ', 'D0d1 ', 730, 130, 12),
        Button('PS2', 2, ButtonStyle.latch, 'D0l2 ', 'D0d2 ', 695, 130, 12),
        Button('PS3', 3, ButtonStyle.latch, 'D0l3 ', 'D0d3 ', 665, 130, 12),
        Button('cancel', 4, ButtonStyle.latch, 'D0l4 ', 'D0d4 ', 615, 130, 12),
        Button('accept', 5, ButtonStyle.latch, 'D0l5 ', 'D0d5 ', 575, 130, 12),
        Button('motor', 6, ButtonStyle.plain, 'D0l6 ', 'D0d6 ', 650, 230, 50),
    ]


    overlays = STM32.led_overlays | {
        # latch key names must match button names
        'PS1': Overlay(4, 702, 26, ['ps1-0.png', 'ps1-1.png']),
        'PS2': Overlay(5, 667, 26, ['ps2-0.png', 'ps2-1.png']),
        'PS3': Overlay(6, 632, 26, ['ps3-0.png', 'ps3-1.png']),
        'sseg': Overlay(7, 550, 289, [
            'sseg-0.png', 'sseg-1.png', 'sseg-2.png', 'sseg-3.png',
            'sseg-4.png', 'sseg-5.png', 'sseg-6.png', 'sseg-7.png',
            'sseg-8.png', 'sseg-9.png', 'sseg-10.png', 'sseg-11.png',
            'sseg-12.png', 'sseg-13.png', 'sseg-14.png', 'sseg-15.png'
        ]),
        'motor': Overlay(8, 592, 171, ['motor-00.png', 'motor-30.png', 'motor-60.png']),
        'spinner': Overlay(9, 627, 203, ['motor-cw.png', 'motor-acw.png', 'motor-stop.png']),
        'door': Overlay(10, 711, 301, ['door-closed.png', 'door-open.png']),
    }

    pin_map = {
        'sseg': PinMap([8, 9, 10, 11], 0,'sseg'),
        'motor': PinMap([12], 0, 'motor'),
        'spinner': PinMap([13], 2, 'spinner'),
        # there can only be one latch key for the whole board
        'latch': PinMap([14], 0, None),
    }

    animation = [
        Animation('motor', 1, 'spinner', 'spinner'),
    ]


class QEmuTag:
    """ Tags to identify response message types"""
    qemu_shutdown = 1
    qemu_warning = 2
    pin_low = 4
    pin_high = 5
    gpiod_enabled = 11
    command = 12
    moder = 13
    idr = 14
    warning = 15
    usart3_enabled = 21
    sr = 22
    cr1 = 23


REPLY_MAP = {
    '=m40023830': QEmuTag.gpiod_enabled, '=m40023840': QEmuTag.usart3_enabled,
    '=d0': QEmuTag.moder, '=d4': QEmuTag.idr,
    '=u0': QEmuTag.sr,    '=u3': QEmuTag.cr1,
}

class Config:
    host: str = 'localhost'
    diag_port: int = 8888
    serial_port: int = 7777


class Settings:
    host: str = 'localhost'
    diag_port: int = 8888
    serial_port: int = 7777

    def __init__(self):
        self.parser = configparser.ConfigParser(interpolation=None)
        if SETTINGS_INI.exists():
            self.parser.read(SETTINGS_INI)
            self.host = self.parser[SETTINGS].get('host', Settings.host)
            self.diag_port = int(self.parser[SETTINGS].get('diag_port', str(Settings.diag_port)))
            self.serial_port = int(self.parser[SETTINGS].get('serial_port', str(Settings.serial_port)))

    def save(self):
        self.parser[SETTINGS] = dict(
            host=self.host,
            diag_port=self.diag_port,
            serial_port=self.serial_port,
        )
        with open(SETTINGS_INI, 'w') as fp:
            self.parser.write(fp)

    def reset(self):
        self.host = Settings.host
        self.diag_port = Settings.diag_port
        self.serial_port = Settings.serial_port
        self.save()


class QEmuListener:
    """ Socket listening class connect to host:8888"""
    def __init__(self, gui, settings: Settings):
        self.gui = gui
        self.recv_buffer = bytearray()
        try:
            self.socket = socket.create_connection(address=(settings.host, settings.diag_port),
                                                   timeout=(2 if settings.host == 'localhost' else 5))
            self.socket.settimeout(0.1)
            self.select_list = [self.socket]
            listener = threading.Thread(target=self.listen)
            listener.daemon = True
            listener.start()
            self.write('noecho ')
            # just works by polling no need to async listener
            # self.write('listen ')
        except socket.gaierror as err:
            raise BoardError(f'Unknown host "{settings.host}":\n{err}')
        except (socket.timeout, ConnectionRefusedError, BrokenPipeError) as err:
            raise BoardError(f'Network error: {err}')

    def close(self):
        self.socket.close()

    def read(self) -> str:
        """
        reads messages from the diagnostic port
        if message is not whitespace terminated trailing part of message is
        saved and prepended to the next message
        """
        try:
            ready, _, _ = select.select(self.select_list, [], [], QEMU_TIMEOUT)
            data = self.socket.recv(15)
            if len(data) == 0:
                raise UserWarning('QEMU shutdown detected')
            data = self.recv_buffer + data
            while data and data[0] == 0xff:
                data = data[3:]
            self.recv_buffer.clear()
            while data and not data[-1:].isspace():
                self.recv_buffer.insert(0, data.pop())
            # print('r', data.strip().decode('ascii'), self.recv_buffer)
            return data.decode('ascii')
        except (socket.timeout, ValueError):
            raise UserWarning('Warning: QEMU receiver timeout')

    def write(self, message: AnyStr):
        """
        Send a message to the diagnostic port
        Each message must be whitespace terminated
        """
        try:
            if isinstance(message, str):
                message = message.encode('ascii')
            # print('w ', message)
            total = 0
            while total < len(message):
                sent = self.socket.send(message[total:])
                if sent == 0:
                    self.gui.qemu_update(QEmuTag.qemu_warning, 0, f'Warning: QEMU connection send error {message}')
                total += sent
        except IOError as ex:
            self.gui.qemu_update(QEmuTag.qemu_warning, 0, str(ex))

    def listen(self):
        """
        Listens for messages sent by the diagnostic port
        Assumes all responses are whitespace (newline etc) terminated
        """
        while True:
            try:
                replies = self.read().split()
                for reply in replies:
                    if reply.startswith('?'):
                        self.gui.qemu_update(QEmuTag.qemu_warning, 0, f'Invalid command response: {reply}')
                    elif reply.startswith('-'):
                        self.gui.qemu_update(QEmuTag.pin_low, int(reply[2], 16))
                    elif reply.startswith('+'):
                        self.gui.qemu_update(QEmuTag.pin_high, int(reply[2], 16))
                    elif reply.startswith('='):
                        cmd, sep, value = reply.partition('?/')
                        if not sep:
                            self.gui.qemu_update(QEmuTag.qemu_warning, 0, f'Memory query missing ?/ separator: "{reply}"')
                            continue
                        tag = REPLY_MAP.get(cmd)
                        if tag is None:
                            self.gui.qemu_update(QEmuTag.qemu_warning, 0, f'Unknown command prefix in response: {reply} "{cmd}"')
                        else:
                            self.gui.qemu_update(tag, int(value, 16))
            except UserWarning as ex:
                self.gui.qemu_update(QEmuTag.qemu_warning, 0, str(ex))
                self.gui.qemu_update(QEmuTag.qemu_shutdown, 0)
                break
            except OSError:
                self.gui.qemu_update(QEmuTag.qemu_warning, 0, 'QEMU connection closed')
                self.gui.qemu_update(QEmuTag.qemu_shutdown, 0)
                break


class QEmuSerial:
    """ Socket polling class connect to host:7777"""
    def __init__(self, settings: Settings):
        try:
            self.socket = socket.create_connection(address=(settings.host, settings.serial_port),
                                                   timeout=(2 if settings.host == 'localhost' else 5))
            self.socket.settimeout(0)
        except socket.gaierror as err:
            raise BoardError(f'Unknown host "{settings.host}":\n{err}')
        except (socket.timeout, ConnectionRefusedError, BrokenPipeError) as err:
            raise BoardError(f'Network error: {err}')

    def close(self):
        self.socket.close()

    def read(self):
        try:
            data = self.socket.recv(12)
            while data and data[0] == 0xff:
                data = data[3:]
            return data.decode('ascii')
        except (socket.timeout, BlockingIOError):
            pass
        return None

    def write(self, message: AnyStr):
        if isinstance(message, str):
            message = message.encode('ascii')
        total = 0
        while total < len(message):
            sent = self.socket.send(message[total:])
            if sent == 0:
                raise UserWarning(f'Warning: QEMU usart send error {message}')
            total += sent


@dataclass
class ImageInfo:
    id: int
    index: int


class BoardDisplay:
    """ Maintains state of the graphic board display """
    def __init__(self, canvas: tk.Canvas, board, manager=None):
        self.canvas = canvas
        self.manager = manager
        self.board = board
        self.latch = False
        self.latched = [False] * len(self.board.buttons)
        self.image_info = [ImageInfo(0, 0) for _ in self.board.overlays]
        self.icon = None

    def find_button(self, x: int, y: int):
        for button in self.board.buttons:
            if button.x - button.radius <= x <= button.x + button.radius:
                if button.y - button.radius <= y <= button.y + button.radius:
                    return button
        return None

    def build_overlay(self, root):
        graphics = SCRIPT_HOME / self.board.graphics_lib
        zip = SCRIPT_HOME / self.board.graphics_zip
        if graphics.exists():
            for tag, overlay in self.board.overlays.items():
                for i, name in enumerate(overlay.images):
                    with (graphics / name).open('rb') as file:
                        overlay.images[i] = tk.PhotoImage(master=root, data=file.read())
            with (graphics / self.board.icon).open('rb') as file:
                self.icon = tk.PhotoImage(master=root, data=file.read())
            with (graphics / self.board.board_image).open('rb') as file:
                return tk.PhotoImage(master=root, data=file.read())
        elif zip.exists():
            with zipfile.ZipFile(zip) as archive:
                for tag, overlay in self.board.overlays.items():
                    for i, name in enumerate(overlay.images):
                        with archive.open(name) as file:
                            overlay.images[i] = tk.PhotoImage(master=root, data=file.read())
                with archive.open(self.board.icon) as file:
                    self.icon = tk.PhotoImage(master=root, data=file.read())
                with archive.open(self.board.board_image) as file:
                    return tk.PhotoImage(master=root, data=file.read())
        else:
            raise BoardError(f'Cannot find graphics folder "{self.board.graphics_lib}" or archive file "{self.board.graphics_zip}"')

    def reset(self):
        for pin in range(8, 15):
            self.update_device(pin, 0, None)
        if self.manager:
            self.manager.reset(self)

    def toggle_init(self):
        for button in self.board.buttons:
            if button.style == ButtonStyle.toggle:
                self.update_button(button, 0)

    def update_image(self, overlay: Overlay, index: int):
        info = self.image_info[overlay.id]
        if info.id:
            if info.index == index:
                return
            self.canvas.delete(info.id)
            info.id = 0
        info.id = self.canvas.create_image(overlay.x, overlay.y, image=overlay.images[index], anchor=tk.NW)
        info.index = index

    def remove_image(self, overlay: Overlay):
        info = self.image_info[overlay.id]
        if info.id:
            self.canvas.delete(info.id)
            info.id = 0

    def update_device(self, pin: int, level: int, qemu: Optional[QEmuListener]):
        if 8 <= pin <= 11:
            self.update_image(STM32.led_overlays[f'led-{pin - 8}'], level)
        for name, map in self.board.pin_map.items():
            if pin in map.pins:
                show = self.manager.show(map.image) if self.manager else True
                map.update_board(self, self.board.overlays, pin, level, show)
                if name == 'latch':
                    self.update_latch(level, qemu)
                if self.manager:
                    self.manager.set_state(self, map.image, level, map.state)


    def update_latch(self, level: int, qemu: Optional[QEmuListener]):
        self.latch = bool(level)
        if not self.latch:
            for button in self.board.buttons:
                if not button.style == ButtonStyle.latch:
                    continue
                self.latched[button.pin] = False
                if qemu:
                    qemu.write(button.up)
                overlay = self.board.overlays.get(button.name, None)
                if overlay:
                    self.update_image(overlay, level)

    def animate(self):
        for map_name, poll, direction_name, overlay_name in self.board.animation:
            map = self.board.pin_map[map_name]
            # if map.state and self.manager and self.manager.animate:
            if map.state:
                if not self.manager or self.manager.show(map.image):
                    map.animate_board(self, self.board.overlays, poll,
                                      self.board.pin_map[direction_name].state if direction_name else None,
                                      self.board.pin_map[overlay_name] if overlay_name else None)

    def update_button(self, button: Button, level: int):
        if self.manager:
            self.manager.set_button(self, button.name, level)
        overlay = self.board.overlays.get(button.name)
        if overlay:
            self.update_image(overlay, level)

    def button_down(self, button: Button, qemu: Optional[QEmuListener]):
        if button.style == ButtonStyle.latch:
            if self.latch and self.latched[button.pin]:
                return
            self.latched[button.pin] = self.latch
        elif button.style == ButtonStyle.toggle:
            if self.latched[button.pin]:
                self.latched[button.pin] = False
                return
            self.latched[button.pin] = True
        self.update_button(button, 1)
        if qemu and button.down:
            qemu.write(button.down)

    def button_up(self, button: Button, qemu: Optional[QEmuListener]):
        if button.style == ButtonStyle.latch:
            if self.latch and self.latched[button.pin]:
                return
        elif button.style == ButtonStyle.toggle:
            if self.latched[button.pin]:
                return
        self.update_button(button, 0)
        if qemu and button.up:
            qemu.write(button.up)


def catch():
    """ Wrapper to display popup dialog for event handler exceptions """
    def decorator(func):
        def wrapper(self, *args, **kwargs):
            try:
                return func(self, *args, **kwargs)
            except UserWarning as ex:
                self.warning(str(ex))
            except Exception as ex:
                if not self.stopping:
                    import traceback, sys
                    traceback.print_exc(file=sys.stderr)
                    messagebox.showerror('Unexpected error', str(ex) + '\n\n' + str(traceback.format_tb(ex.__traceback__, limit=1)),
                                         parent=self.root)
                    messagebox.showerror('Unexpected Error', str(ex))
        return wrapper
    return decorator


def wrap_scroll(parent, widget, **kwargs):
    frame = tk.Frame(parent, bd=1, relief=tk.SUNKEN)
    frame.pack(fill=tk.BOTH, expand=1)
    xsbar_frame = tk.Frame(frame)
    xsbar_frame.pack(fill=tk.X, side=tk.BOTTOM)
    w = widget(frame, **kwargs)
    w.pack(side=tk.LEFT, fill=tk.BOTH, expand=1)
    xscroll = ttk.Scrollbar(xsbar_frame, orient=tk.HORIZONTAL, command=w.xview)
    xscroll.pack(side=tk.BOTTOM, fill=tk.X)
    yscroll = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=w.yview)
    yscroll.pack(side=tk.RIGHT, fill=tk.Y)
    w.configure(xscrollcommand=xscroll.set)
    w.configure(yscrollcommand=yscroll.set)
    return w


def scroll_main(root):
    """ Wrap a scroll bar around the entire root window"""
    canvas = wrap_scroll(root, tk.Canvas)
    canvas.bind("<Configure>", lambda e: canvas.config(scrollregion=canvas.bbox(tk.ALL)))
    frame = tk.Frame(canvas)
    canvas.create_window((0, 0), window=frame, anchor=tk.NW)
    return frame


class CheckBox(tk.Checkbutton):
    """ Customised CheckButton to simplify coding"""
    def __init__(self, parent, text: str, value=0, anchor=tk.W, **kwargs):
        tk.Checkbutton()
        self.var = tk.IntVar(value=value)
        super().__init__(parent, text=text, variable=self.var, anchor=anchor, **kwargs)

    def set(self, value: int):
        self.var.set(value)

    def get(self) -> bool:
        return bool(self.var.get())


DisplayButton = namedtuple('DisplayButton', 'if_connected button')


class BoardGui:
    """ Builds and manages the GUI """
    def __init__(self, root: tk.Tk, board, manager=None):
        self.root = root
        self.manager= manager
        self.ticker = None
        self.stopping = False
        self.button = None
        self.listener = None
        self.serial = None
        self.device_ticks = 0
        self.warn_ticks = 0
        self.connected = Connect.disconnected
        self.reconnect = True
        self.settings = Settings()
        # self.host = Config.host
        # self.diag_port = Config.diag_port
        # self.serial_port = Config.serial_port
        self.style = ttk.Style()
        self.style.configure('.', sticky=(tk.N, tk.W), font=('Sans Serif', 10), padding=5)
        self.style.configure('warning.TLabel', font=('Sans Serif', 11, 'italic'), padding=5)

        root.geometry(GEOMETRY[0])
        root.title("Feabhas STM32F407")
        base = scroll_main(root)
        root.protocol("WM_DELETE_WINDOW", self.on_quit)

        self.menubar = tk.Menu(root)
        root.config(menu=self.menubar)
        settings = tk.Menu(self.menubar, name='settings')
        self.menubar.insert_cascade(index=1, label='settings', menu=settings)
        settings.add_command(label='host name', command=self.on_change_host)
        settings.add_command(label='diagnostic port', command=self.on_change_diag_port)
        settings.add_command(label='serial port', command=self.on_change_serial_port)
        settings.add_separator()
        settings.add_command(label='save settings', command=self.on_save_settings)
        settings.add_command(label='default settings', command=self.on_default_settings)

        # root.config(menu=self.menubar)
        # # self.menubar.add_cascade(label=" "*230)
        # self.menubar.add_command(label="<Change host>", command=self.on_change_host)
        # self.menubar.add_command(label="<Change diagnostic port>", command=self.on_change_diag_port)
        # self.menubar.add_command(label="<Change serial port>", command=self.on_change_host)

        self.status = ttk.LabelFrame(base, text=f'QEMU {self.settings.host}:{self.settings.diag_port}')
        self.status.pack(anchor=tk.W, padx=5, fill=tk.X)
        self.feedback = tk.Frame(self.status)
        self.feedback.pack(anchor=tk.W, side=tk.TOP, padx=5, fill=tk.X)
        self.warn_field = ttk.Label(self.feedback, text='', style='warning.TLabel')
        self.warn_field.pack(side=tk.LEFT)
        ttk.Button(self.feedback, text='Quit', command=self.on_quit).pack(side=tk.RIGHT, padx=5)
        self.buttons = [
            DisplayButton(False, ttk.Button(self.feedback, text='Connect+Serial', command=self.on_connect_serial)),
            DisplayButton(False, ttk.Button(self.feedback, text='Connect', command=self.on_connect)),
            DisplayButton(True, ttk.Button(self.feedback, text='Halt', command=self.on_halt)),
            DisplayButton(True, ttk.Button(self.feedback, text='Disconnect', command=self.on_disconnect)),
        ]
        self.do_enable_buttons(False)

        stm32 = ttk.LabelFrame(base, text="STM32F407")
        stm32.pack(fill=tk.BOTH, pady=5, padx=5)
        frame = tk.Frame(stm32)
        frame.pack(side=tk.RIGHT, pady=0, padx=0)
        border = 3
        canvas = tk.Canvas(frame, width=board.image_x - border, height=board.image_y - border,
                           borderwidth=0, highlightthickness=border, highlightbackground="#444")
        canvas.pack(fill=tk.BOTH, expand=1, pady=5, padx=5)
        canvas.config(scrollregion=canvas.bbox(tk.ALL))
        canvas.bind('<Button-1>', self.on_b1_down)
        canvas.bind('<ButtonRelease-1>', self.on_b1_up)
        canvas.bind('<Motion>', self.on_move)
        self.display = BoardDisplay(canvas, board, manager)
        try:
            self.image = self.display.build_overlay(root)
            canvas.create_image(0, 0, image=self.image, anchor=tk.NW)
            if self.display.icon is not None:
                root.iconphoto(False, self.display.icon)
            self.display.toggle_init()
        except Exception as err:
            messagebox.showerror('Startup error', f'Error or missing graphics file:\n{err}')
            raise KeyboardInterrupt(err)

        frame = tk.Frame(stm32)
        frame.pack(anchor=tk.N, side=tk.LEFT, padx=0, pady=0)
        self.mode = ttk.Label(frame, text='mode: 00000000')
        self.mode.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        self.idr = ttk.Label(frame, text='     idr: 0000')
        self.idr.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        ttk.Label(frame, text='').pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        self.gpiod = CheckBox(frame, text='GPIOD', state=tk.DISABLED)
        self.gpiod.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        self.pins = [
            CheckBox(frame, text=text, state=tk.DISABLED)
            for text in board.checkbox_labels
        ]
        for pin in self.pins:
            pin.pack(anchor=tk.W, side=tk.TOP, padx=0)

        self.usart_frame = ttk.LabelFrame(base, text=f'Serial port :{self.settings.serial_port}')
        # self.usart_frame.pack(anchor=tk.W, padx=5, pady=0, fill=tk.X) # only show if connected to usart

        frame = tk.Frame(self.usart_frame)
        frame.pack(side=tk.LEFT, padx=5, fill=tk.Y)
        self.cr1 = ttk.Label(frame, text=f'cr1: 0000')
        self.cr1.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        self.sr = ttk.Label(frame, text=f'sr: 0000')
        self.sr.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        ttk.Label(frame, text='').pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)
        self.usart3 = CheckBox(frame, text='USART3', state=tk.DISABLED)
        self.usart3.pack(anchor=tk.W, side=tk.TOP, padx=0, pady=0)

        frame = tk.Frame(self.usart_frame)
        frame.pack(side=tk.RIGHT, padx=5, fill=tk.BOTH)
        self.putty = wrap_scroll(frame, tk.Text, height=8, width=95, wrap='none', state=tk.DISABLED)
        self.putty.pack(anchor=tk.NW, expand=True, fill=tk.BOTH)
        self.putty.bind('<KeyPress>', self.on_putty_key)

        self.root.bind(QEMU_MESSAGE, self.on_qemu_message)
        self.warning('Use "Connect" or "Connect+Serial" to attach to QEMU')

    def do_enable_buttons(self, connected: bool):
        for b in self.buttons:
            if connected == b.if_connected:
                b.button['state'] = tk.NORMAL
                b.button.pack(side=tk.RIGHT, padx=5)
            else:
                b.button['state'] = tk.DISABLED
                b.button.pack_forget()
        self.menubar.entryconfig(1, state=tk.DISABLED if connected else tk.NORMAL)

    def reset_state(self):
        self.mode['text'] = 'mode: 00000000'
        self.idr['text'] = '     idr: 0000'
        self.gpiod.set(0)
        for pin in self.pins:
            pin.set(0)

    def do_disconnect(self):
        if self.ticker:
            self.root.after_cancel(self.ticker)
            self.ticker = None
        self.do_enable_buttons(False)
        self.reset_state()
        self.display.reset()
        if self.listener:
            self.listener.close()
            self.listener = None
            if self.serial:
                self.serial.close()
                self.serial = None
        self.connected = Connect.disconnected
        self.reconnect = True
        self.device_ticks = self.warn_ticks = 0

    @catch()
    def on_disconnect(self):
        self.do_disconnect()

    @catch()
    def on_halt(self):
        if self.listener:
            self.listener.write(b'halt ')
        self.do_disconnect()

    @catch()
    def on_putty_key(self, event):
        if self.serial:
            self.serial.write(event.char)
        return "break"

    def do_warning(self, value: str):
        if value:
            self.warn_field['text'] = f'{datetime.now():%H:%M:%S} {value}'
            self.warn_ticks = POLL
        else:
            self.warn_field['text'] = ''
            self.warn_ticks = 0

    def warning(self, msg: str):
        self.root.after(10, self.do_warning, msg)

    def on_move(self, event):
        button = self.display.find_button(event.x, event.y)
        if button != self.button:
            self.root.config(cursor='hand2' if button else '')
            self.button = button

    @catch()
    def on_b1_down(self, event):
        # print(event.x, event.y)
        if self.button:
            self.display.button_down(self.button, self.listener)

    @catch()
    def on_b1_up(self, _):
        if self.button:
            if self.button.up.startswith('reset'):
                self.gpiod.set(0)
            self.display.button_up(self.button, self.listener)

    def show_connect_error(self, message: str, error=''):
        if error and not error.endswith('\n'):
            error += '\n'
        serial = ' serial' if self.putty['state'] == tk.NORMAL else ''
        messagebox.showerror('QEMU Connect Error',
            f'''{message} (on "{self.settings.host}:{self.settings.diag_port}")
{error}Please start QEMU in your container using:

test task\t"Run QEMU{serial}"
or\t"./run-qemu.sh diag{serial}"''')

    def on_qemu_message(self, event):
        tag = event.state
        value = event.x
        if tag == QEmuTag.gpiod_enabled:
            if self.connected != Connect.connected:
                self.warning('QEMU connected OK')
                self.do_enable_buttons(True)
                self.connected = Connect.connected
            gpiod_on = (value >> 3) & 1
            self.gpiod.set(1 if gpiod_on else 0)
        elif tag == QEmuTag.moder:
            self.mode['text'] = f'mode: {value:08X}'
        elif tag == QEmuTag.idr:
            self.idr['text'] = f'     idr: {value:04X}'
            for bit, pin in enumerate(self.pins, 8):
                state = (value >> bit) & 1
                if not self.reconnect and (bool(state) != pin.get()):
                    self.display.update_device(bit, state, self.listener)
                pin.set(state)
                if self.reconnect:
                    self.display.update_device(bit, state, self.listener)
            if self.reconnect:
                self.reconnect = False
                for button in self.display.board.buttons:
                    if button.pin is None:
                        continue
                    if (value >> button.pin) & 1:
                        self.display.button_down(button, None)
                        self.display.button_up(button, None)
        # pollling only - async listener not started
        # elif tag == QEmuTag.pin_low:
        #     self.display.update_device(value, 0, self.listener)
        # elif tag == QEmuTag.pin_high:
        #     self.display.update_device(value, 1, self.listener)
        elif tag == QEmuTag.usart3_enabled:
            usart3_on = (value >> 18) & 1
            self.usart3.set(1 if usart3_on else 0)
        elif tag == QEmuTag.sr:
            self.sr['text'] = f'sr: {value:04X}'
        elif tag == QEmuTag.cr1:
            self.cr1['text'] = f'cr1: {value:04X}'
        elif tag == QEmuTag.qemu_warning:
            self.warning(event.message)
        elif tag == QEmuTag.qemu_shutdown:
            self.display.reset()
            if self.listener:
                if self.connected == Connect.connected:
                    self.warning(f'QEMU has closed down')
                else:
                    self.show_connect_error('QEMU is not running')
            self.do_disconnect()
        else:
            self.warning(f'Internal error: unknown qemu tag {int(tag)}')

    def qemu_update(self, tag: int, value: int, message = None):
        if message:
            tk.Event.message = message
        self.root.event_generate(QEMU_MESSAGE, when='tail', state=tag, x=value)

    def do_query_qemu(self):
        if not self.listener:
            return
        if self.gpiod.get():
            self.listener.write(b'D0? ')
            self.listener.write(b'D4? ')
        if self.usart3.get():
            self.listener.write(b'U0? ')
            self.listener.write(b'U3? ')

    def do_poll_serial(self):
        while self.serial:
            text = self.serial.read()
            if not text:
                break
            self.putty.insert(tk.END, text)
            self.putty.see("end")

    @catch()
    def on_timer_running(self):
        if self.listener:
            self.do_poll_serial()
            self.do_query_qemu()
            self.display.animate()
            self.listener.write(STM32.rcc_ahbenr)
            if self.serial:
                self.listener.write(STM32.rcc_apb1enr)
            if not self.stopping:
                self.ticker = self.root.after(POLL, self.on_timer_running)

    @catch()
    def do_start_connect(self, message: str):
        self.root.config(cursor='watch')
        for b in self.buttons:
            if not b.if_connected:
                b.button.pack_forget()
        self.do_warning(message)

    def do_connect(self, *args):
        try:
            self.listener = QEmuListener(self, self.settings)
            self.ticker = self.root.after(POLL, self.on_timer_running)
        except BoardError as err:
            self.show_connect_error(f'QEMU diagnostic port {self.settings.host}:{self.settings.diag_port} is not open: {err}')
            self.do_warning(f'Cannot connect to QEMU on {self.settings.host}')
            self.do_disconnect()
        finally:
            if not self.stopping:
                self.root.config(cursor='')

    @catch()
    def on_connect(self):
        self.do_start_connect('Connecting to QEMU diagnostics...')
        self.root.after(50, self.do_connect)

    def do_settings_update(self):
        self.status['text'] = f'QEMU {self.settings.host}:{self.settings.diag_port}'
        self.usart_frame['text'] = f'Serial port :{self.settings.serial_port}'

    def do_ask_setting(self, title, prompt, initial_value, number=False):
        value = simpledialog.askstring(title, prompt, initialvalue=str(initial_value))
        value = value.strip() if value else value
        if not value:
            return None
        if number and not value.isdigit():
            messagebox.showerror('Invalid port', f'Non numeric port number entered: {value}')
            return
        return str(value) if number else value

    @catch()
    def on_change_host(self):
        # host = simpledialog.askstring('Change QEMU host',
        #                               'Enter new hostname?',
        #                               initialvalue=self.settings.host)
        host = self.do_ask_setting('Change QEMU host', 'Enter new hostname?', self.settings.host)
        if not host:
            return
        self.settings.host = host
        self.do_settings_update()

    @catch()
    def on_change_diag_port(self):
        port = self.do_ask_setting('Change QEMU diagnostic port','Enter new diagnostic port number?',
                                   self.settings.diag_port, number=True)
        if not port:
            return
        self.settings.diag_port = port
        self.do_settings_update()

    @catch()
    def on_change_serial_port(self):
        port = self.do_ask_setting('Change QEMU serial port','Enter new serial port number?',
                                   self.settings.serial_port, number=True)
        if not port:
            return
        self.settings.serial_port = port
        self.do_settings_update()

    @catch()
    def on_save_settings(self):
        self.settings.save()

    @catch()
    def on_default_settings(self):
        self.settings.reset()
        self.do_settings_update()

    def do_connect_serial(self):
        try:
            self.serial = QEmuSerial(self.settings)
            if self.putty['state'] != tk.NORMAL:
                self.putty['state'] = tk.NORMAL
                self.putty.insert(tk.END, '''Make sure you start 
test task\t\t"Run QEMU serial" 
or command\t\t"./run-qemu.sh diag serial"
Serial port connecting...
''')
            self.putty.see("end")
            self.putty.focus_set()
            self.usart_frame.pack(anchor=tk.W, padx=5, pady=0, fill=tk.X)
            self.root.geometry(GEOMETRY[1])
            self.do_connect()
        except BoardError as err:
            self.show_connect_error(f'QEMU serial port {self.settings.host}:{self.settings.serial_port} is not open: {str}')
            self.do_warning(f'Cannot connect to {self.settings.host}')
            self.do_disconnect()
        finally:
            if not self.stopping:
                self.root.config(cursor='')

    @catch()
    def on_connect_serial(self):
        self.do_start_connect('Connecting to QEMU serial port...')
        self.root.after(50, self.do_connect_serial)

    def on_quit(self):
        if self.listener:
            self.listener.write(b'halt ')
        self.close()

    def close(self):
        try:
            self.stopping = True
            if self.serial:
                self.serial.close()
            if self.listener:
                self.listener.close()
            self.root.destroy()
            self.root.quit()
        except:
            pass


def start(board, manager=None):
    """ Main method builds GUI and starts TkInter main loop"""
    app = None
    try:
        root = tk.Tk()
        app = BoardGui(root, board, manager)
        root.mainloop()
    except KeyboardInterrupt:
        if app:
            app.close()



def main():
    start(WmsBoard)


if __name__ == "__main__":
    main()
