#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct
import threading
import serial

try:
    from codes import MSG, ETAT_PROD, ETAT_CALIBRATION
except (ImportError, AttributeError):
    MSG = {}
    ETAT_PROD = {}
    ETAT_CALIBRATION = {}

DEFAULT_BAUDRATE = 115200
SERIAL_TIMEOUT = 0.1

EVENT_BINARY_ID = 0x00
MESURES_BINARY_ID = 0x01
MESURES_BN0055_BINARY_ID = 0x02

BINARY_FRAME_SPECS = {
    EVENT_BINARY_ID: (
        "Event",
        struct.Struct("<BBi"),
    ),
    MESURES_BINARY_ID: (
        "Mesures",
        struct.Struct("<IHhiiHhi"),
    ),
    MESURES_BN0055_BINARY_ID: (
        "MesuresBN0055",
        struct.Struct("<Ihhhiiiiii"),
    ),
}


class SerialPortHandler:
    """Handles serial port I/O, buffering and decoding of frames.

    Pushes raw tuples to an external `rx_queue` for the UI to display.
    """

    def __init__(self, rx_queue, default_baud=DEFAULT_BAUDRATE):
        self.rx_queue = rx_queue
        self.default_baud = default_baud
        self.serial_port = None
        self.reader_thread = None
        self.reader_running = False
        self.rx_buffer = bytearray()
        self.show_hex = False

    def connect(self, port, baud):
        self.disconnect()
        self.serial_port = serial.Serial(port=port, baudrate=baud, timeout=SERIAL_TIMEOUT)
        self.reader_running = True
        self.reader_thread = threading.Thread(target=self._reader, daemon=True)
        self.reader_thread.start()

    def disconnect(self):
        self.reader_running = False

        if self.serial_port is not None:
            try:
                self.serial_port.close()
            except Exception:
                pass

        self.serial_port = None

    def is_connected(self):
        return self.serial_port is not None and self.serial_port.is_open

    def set_show_hex(self, show: bool):
        self.show_hex = bool(show)

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
                self.rx_queue.put(("info", f"--- Erreur série : {exc} ---"))
                break

    def _consume_rx_buffer(self):
        while self.rx_buffer:
            first_byte = self.rx_buffer[0]

            if first_byte in BINARY_FRAME_SPECS:
                message_name, payload_struct = BINARY_FRAME_SPECS[first_byte]
                frame_size = 1 + payload_struct.size

                if len(self.rx_buffer) < frame_size:
                    return

                frame = bytes(self.rx_buffer[:frame_size])
                del self.rx_buffer[:frame_size]

                # enqueue raw binary frame
                self.rx_queue.put(("bin", frame, message_name, payload_struct))
                continue

            newline_index = self.rx_buffer.find(b"\n")

            if newline_index == -1:
                return

            line = bytes(self.rx_buffer[:newline_index + 1])
            del self.rx_buffer[:newline_index + 1]

            # enqueue raw line
            self.rx_queue.put(("line", line))

    # Decoding helpers
    def decode_frame(self, frame, message_name, payload_struct):
        payload = frame[1:]

        try:
            values = payload_struct.unpack(payload)
        except struct.error:
            return f"--- Trame binaire invalide ({message_name}) : {self._format_hex(frame)} ---"

        if message_name == "Event":
            return self._decode_event(values, frame)

        if message_name == "Mesures":
            return self._decode_mesures(values, frame)

        if message_name == "MesuresBN0055":
            return self._decode_accelerometre(values, frame)

        return f"--- Trame binaire inconnue ({message_name}) : {self._format_hex(frame)} ---"

    def _decode_event(self, values, frame):
        if len(values) < 3:
            return f"--- Trame Event invalide : {self._format_hex(frame)} ---"

        message_type, code, value = values
        message_type_text = self._display_ascii_byte(message_type)
        event_label = self._event_label(message_type_text)

        if message_type_text == 'S':
            if code == 250:
                state_name = ETAT_PROD.get(value, str(value))
            elif code == 251:
                state_name = ETAT_CALIBRATION.get(value, str(value))
            else:
                state_name = str(value)

            code_text = self._message_name(code)
            return f"{event_label} : type={message_type_text} code={code} ({code_text}) val={state_name}"

        template = MSG.get(code, None)
        if template is None:
            formatted = str(value)
        else:
            formatted = self._format_template(template, value)

        code_text = self._message_name(code)
        return f"Event {event_label} | type={message_type_text} code={code} ({code_text}) val={formatted}"

    def _decode_mesures(self, values, frame):
        if len(values) != 8:
            return f"--- Trame Mesures invalide : {self._format_hex(frame)} ---"

        (
            time_ms,
            tension,
            courant_moyen,
            angle_moteur,
            vitesse_moteur,
            angle_porte,
            pwm,
            consigne,
        ) = values

        return (
            "Mesures | "
            f"t={time_ms}ms "
            f"tension={tension / 100:.2f}V "
            f"courant={courant_moyen / 100:.2f}A "
            f"angle_moteur={angle_moteur / 100:.2f}° "
            f"vitesse_moteur={vitesse_moteur / 100:.2f}rad/s "
            f"angle_porte={angle_porte / 100:.2f}° "
            f"pwm={pwm / 100:.2f} "
            f"consigne={consigne / 100:.2f}"
        )

    def _decode_accelerometre(self, values, frame):
        if len(values) != 10:
            return f"--- Trame MesuresBN0055 invalide : {self._format_hex(frame)} ---"

        (
            accel_x,
            accel_y,
            accel_z,
            time_ms,
            gyro_x,
            gyro_y,
            gyro_z,
            heading,
            roll,
            pitch,
        ) = values

        return (
            "Accéléro | "
            f"t={time_ms}ms "
            f"accel=({accel_x / 100:.2f},{accel_y / 100:.2f},{accel_z / 100:.2f}) "
            f"gyro=({gyro_x / 100:.2f},{gyro_y / 100:.2f},{gyro_z / 100:.2f}) "
            f"heading={heading / 100:.2f}° "
            f"roll={roll / 100:.2f}° "
            f"pitch={pitch / 100:.2f}°"
        )

    # Utility formatting functions used by the handler
    @staticmethod
    def _format_hex(data):
        return " ".join(f"{byte_value:02X}" for byte_value in data)

    @staticmethod
    def _display_ascii_byte(value):
        if 32 <= value <= 126:
            return chr(value)

        return f"0x{value:02X}"

    @staticmethod
    def _message_name(code):
        return MSG.get(code, "inconnu")

    @staticmethod
    def _event_label(message_type_text):
        return {
            "E": "Erreur",
            "W": "Warning",
            "I": "Info",
            "O": "OK",
            "N": "NOK",
            "S": "State",
            "R": "Réponse",
            "T": "Test",
        }.get(message_type_text, "Event")

    def _format_template(self, template, value):
        import re

        def repl_choice(match):
            options = match.group(1).split("|")
            try:
                idx = int(value)
                return options[idx] if 0 <= idx < len(options) else str(value)
            except Exception:
                return str(value)

        result = re.sub(r"\$\{([^}]+)\}", repl_choice, template)

        def repl_scale(match):
            nd = int(match.group(1))
            try:
                scaled = value / (10 ** nd)
                return f"{scaled:.{nd}f}"
            except Exception:
                return str(value)

        result = re.sub(r"\$\.(\d+)", repl_scale, result)
        result = re.sub(r"\$(?![\.\{])", str(value), result)
        return result
