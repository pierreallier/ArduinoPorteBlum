#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import serial
import threading

from frames import (ErrorEvent, WarningEvent, InfoEvent, StateEvent, ResponseEvent, TestEvent, MesureEvent, AccelEvent, ASCIIEvent, RequestEvent, GuiEvent)
import serial.tools.list_ports

DEFAULT_BAUDRATE = 115200
SERIAL_TIMEOUT = 0.1

def crc8(data: bytes) -> int:
    """CRC-8/SMBUS: poly=0x07, init=0, no reflection, xorout=0."""
    crc = 0
    for value in data:
        crc ^= value
        for _ in range(8):
            crc = ((crc << 1) ^ (0x07 if crc & 0x80 else 0)) & 0xFF
    return crc


class SerialPortHandler:
    """Handles serial port I/O, buffering and decoding of frames.

    Pushes `Event` objects to an external `rx_queue` for the UI to display.
    """

    def __init__(self, rx_queue, default_baud=DEFAULT_BAUDRATE):
        self.rx_queue = rx_queue # The rx_queue will hold decoded Event objects for the GUI to consume.
        self.default_baud = default_baud
        self.serial_port = None
        self.reader_thread = None
        self.reader_running = False
        self.rx_buffer = bytearray()
        self.show_hex = False
        

    def _write(self, data: bytes, attempts: int = 3, delay: float = 0.05) -> bool:
        """Write `data` to serial port silently with retries.

        Returns True on success, False otherwise. Does not raise.
        """
        try:
            ser = self.serial_port
            if ser is None:
                return False

            for i in range(attempts):
                try:
                    if getattr(ser, "is_open", False):
                        ser.write(data)
                        try:
                            ser.flush()
                        except Exception:
                            pass
                        self.rx_queue.put(RequestEvent(f"{data.decode(errors='replace').strip()}"))
                        return True
                    else:
                        # Port closed; nothing to do
                        return False
                except Exception:
                    # Wait briefly and retry
                    time.sleep(delay)

            return False
        except Exception:
            return False
        
    def is_connected(self):
        return self.serial_port is not None and self.serial_port.is_open

    def set_show_hex(self, show: bool):
        self.show_hex = bool(show)

    def list_ports(self):
        """Return a tuple (port_names, suggested_port_or_none)."""
        ports = list(serial.tools.list_ports.comports())
        port_names = [p.device for p in ports]

        # prefer an Arduino-like port
        keywords = ("arduino", "mega", "uno", "leonardo", "micro", "ch340", "ch341", "cp210", "ftdi", "usb serial")
        suggested = None
        for p in ports:
            text = " ".join(str(v) for v in (p.description, p.manufacturer, p.product, p.hwid) if v).lower()
            if any(k in text for k in keywords):
                suggested = p.device
                break

        return port_names, suggested

    def connect(self, port: str, baudrate: int = None):
        """Open the serial port and start the reader thread.

        Raises exceptions from pyserial on failure.
        """
        if baudrate is None:
            baudrate = self.default_baud

        # If already connected, disconnect first
        if self.is_connected():
            try:
                self.disconnect()
            except Exception:
                pass

        ser = serial.Serial(port, baudrate=baudrate, timeout=SERIAL_TIMEOUT)
        self.serial_port = ser

        # start reader thread
        self.reader_running = True
        self.reader_thread = threading.Thread(target=self._reader, daemon=True)
        self.reader_thread.start()

    def disconnect(self):
        """Stop reader, send 'SET MESURES OFF' and close the serial port."""
        # Ask to stop mesures before closing
        try:
            self._write(b"SET MESURES OFF\n")
        except Exception:
            pass

        # stop reader
        self.reader_running = False
        if self.reader_thread is not None:
            try:
                self.reader_thread.join(timeout=5*SERIAL_TIMEOUT)
            except Exception:
                pass
            self.reader_thread = None

        # close port
        if self.serial_port is not None:
            try:
                if getattr(self.serial_port, "is_open", False):
                    try:
                        self.serial_port.flush()
                    except Exception:
                        pass
                    try:
                        self.serial_port.close()
                    except Exception:
                        pass
            finally:
                self.serial_port = None
        self.rx_buffer.clear()

    # Reader & buffer consumption
    def _reader(self):
        while self.reader_running:
            ser = self.serial_port

            if ser is None or not ser.is_open:
                break

            try:
                chunk = ser.read(ser.in_waiting or 1)

                if chunk:
                    self.rx_buffer.extend(chunk)
                    self._consume_rx_buffer()

            except (serial.SerialException, OSError) as exc:
                # Enqueue as an GUIEvent so the GUI receives Event-like objects.
                self.rx_queue.put(GuiEvent(f"--- Erreur série : {exc} ---"))
                break

    def _consume_rx_buffer(self):
        """Decode complete frames, keeping partial frames for the next read."""
        while self.rx_buffer:
            if self.rx_buffer[0] == 0x01:
                if len(self.rx_buffer) < 2:
                    return

                frame_class = {
                    ord('A'): AccelEvent,
                    ord('M'): MesureEvent,
                    ord('E'): ErrorEvent,
                    ord('W'): WarningEvent,
                    ord('I'): InfoEvent,
                    ord('S'): StateEvent,
                    ord('R'): ResponseEvent,
                    ord('O'): ResponseEvent,
                    ord('N'): ResponseEvent,
                    ord('T'): TestEvent,
                }.get(self.rx_buffer[1])
                if frame_class is None:
                    self.rx_queue.put(GuiEvent("--- Type de trame binaire inconnu ---"))
                    del self.rx_buffer[0]
                    continue
                size = frame_class.frame_size()
                if len(self.rx_buffer) < size:
                    return
                if self.rx_buffer[size - 1] != crc8(self.rx_buffer[:size - 1]):
                    # Reject this start marker and search for the next frame.
                    self.rx_queue.put(GuiEvent(
                        "--- Trame binaire invalide : CRC8 incorrect ---"
                    ))
                    del self.rx_buffer[0]
                    continue
            else:
                frame_class = ASCIIEvent
                newline_idx = self.rx_buffer.find(b"\n")
                if newline_idx != -1:
                    size = newline_idx + 1
                else:
                    return

            frame = bytes(self.rx_buffer[:size])
            del self.rx_buffer[:size]
            event = frame_class.decode_from_frame(frame)
            self.rx_queue.put(event)

            if event.code == 71:
                try:
                    self._write(b"SET MESURES ON\n")
                except Exception:
                    pass
