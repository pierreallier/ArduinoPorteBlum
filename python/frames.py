#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Frame classes for decoded serial frames.

Each class knows its binary payload `struct` and can via static methods:
 - report its full frame size via `frame_size()`
 - decode a full `frame` bytes via `decode_from_frame()` -> returns an Event subclass instance

 TODO : 
    - le TYPE n'est peut être pas nécessaire d'être stocké dans l'instance, il pourrait être un attribut de classe uniquement.
    - dans MesureEvent et AcceleroEvent, il faudrait stocker les valeurs numériques des mesures dans l'instance pour pouvoir les récupérer plus tard.
"""
from dataclasses import dataclass
from typing import Tuple, Optional
import struct
import re

try:
    from codes import MSG, ETAT_PROD, ETAT_CALIBRATION
except Exception:
    MSG = {}
    ETAT_PROD = {}
    ETAT_CALIBRATION = {}


@dataclass(init=False)
class Event:
    code : Optional[int] = None
    string: str = ""
    frame: Optional[bytes] = None

    TYPE: Optional[str] = None # class-level identifier (single char) for subclasses; None for base
    PAYLOAD_STRUCT = struct.Struct("<BBi") # class-level struct for binary payload (message_type, code, value)

    def __init__(self, code:int, string: str, frame: Optional[bytes] = None):
        """Base Event holds only code value, presentation `string` and raw `frame`.

        Subclasses are responsible for computing `string` before calling super().
        """
        self.code = code
        self.string = string or ""
        self.frame = frame

    def __str__(self) -> str:
        return self.string

    # helper to get printable ASCII char from a byte, or None
    @staticmethod
    def _ascii_char(value: int) -> Optional[str]:
        if 32 <= value <= 126:
            return chr(value)
        return f"0x{value:02X}"

    # Format a template string with a single integer value if $ is present.
    # Supports:
    #   $ -> replaced with value
    #   ${a|b|c} -> replaced with a/b/c based on value as index (starts at 0)
    #   $.N -> replaced with value scaled by 10^N and formatted with N decimals
    @staticmethod
    def _format_template(template: str, value: int) -> str:
        
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

    @staticmethod
    def frame_size() -> int:
        return 1 + Event.PAYLOAD_STRUCT.size

    @staticmethod
    def decode_from_frame(frame: bytes):
        payload = frame[1:]
        try:
            values = Event.PAYLOAD_STRUCT.unpack(payload)
        except struct.error:
            return Event(None, f"--- Trame binaire invalide (Event) : {frame.hex().upper()} ---", frame)
        return Event.decode(values, frame)

    @staticmethod
    def decode(values: Tuple, frame: bytes):
        """Decode Event-type payload (message_type, code, value).

        Returns an instance of Event or a more specific subclass.
        """
        if len(values) < 3:
            return ASCIIEvent(f"--- Trame Event invalide : {frame.hex().upper()} ---", frame)

        message_type, code, value = values
        message_type_text = Event._ascii_char(message_type)

        # State handling and subclass dispatch (subclasses accept code,val,frame)
        if message_type_text == 'S':
            return StateEvent(code, value, frame)
        if message_type_text == 'E':
            return ErrorEvent(code, value, frame)
        if message_type_text == 'W':
            return WarningEvent(code, value, frame)
        if message_type_text == 'I':
            return InfoEvent(code, value, frame)
        if message_type_text in ['R', 'O', 'N']:
            return ResponseEvent(message_type_text, code, value, frame)
        if message_type_text == 'T':
            return TestEvent(code, value, frame)

        # default: create a generic Event with a readable string
        label = message_type_text if isinstance(message_type_text, str) else f"0x{message_type:02X}"
        return ASCIIEvent(f"{label} : code={code} val={value}", frame)


    # Presentation helpers used by GUI
    def get_string(self) -> str:
        return self.string

    def get_hex(self) -> str:
        return self.frame.hex().upper() if self.frame else ""

    @staticmethod
    def get_tag() -> str:
        """Return a display tag name for this event (used by the GUI).

        Tag names are arbitrary identifiers that the GUI will use to apply
        text styling. Use `get_tag_config(name)` to obtain the tag's
        configuration (foreground, justify, background).
        """
        return "rx"

    @staticmethod
    def get_tag_config() -> dict:
        """Return the tag configuration tuple for this event's tag."""
        return {'justify': 'left'}

    @staticmethod
    def get_prefix() -> str:
        return ' '  # No prefix by default; subclasses may override to provide a prefix for display in the GUI.

    @staticmethod
    def get_suffix() -> str:
        return ' '  # No suffix by default; subclasses may override to provide a suffix for display in the GUI.

class TestEvent(Event):
    TYPE = "T"

    def __init__(self, code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
        template = MSG.get(code, None)
        if template is None:
            s = f"Test : code={code} val={value}"
        else:
            try:
                s = f"Test : {Event._format_template(template, value)}"
            except Exception:
                s = f"Test : code={code} val={value}"
        super().__init__(code, s, frame)



class ErrorEvent(Event):
    TYPE = "E"

    def __init__(self, code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
        template = MSG.get(code, None)
        if template is None:
            s = f"Erreur : code={code} val={value}"
        else:
            try:
                s = f"Erreur : {Event._format_template(template, value)}"
            except Exception:
                s = f"Erreur : code={code} val={value}"
        super().__init__(code, s, frame)

    @staticmethod
    def get_tag() -> str:
        return "err"

    @staticmethod
    def get_tag_config() -> dict:
         return {'foreground': '#C00000', 'justify': 'left'}


class WarningEvent(Event):
    TYPE = "W"

    def __init__(self, code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
        template = MSG.get(code, None)
        if template is None:
            s = f"Warning : code={code} val={value}"
        else:
            try:
                s = f"Warning : {Event._format_template(template, value)}"
            except Exception:
                s = f"Warning : code={code} val={value}"
        super().__init__(code, s, frame)

    @staticmethod
    def get_tag() -> str:
        return "warn"

    @staticmethod
    def get_tag_config() -> dict:
        return {'foreground': '#D28C00', 'justify': 'left'}


class InfoEvent(Event):
    TYPE = "I"

    def __init__(self, code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
            template = MSG.get(code, None)
            if template is None:
                s = f"Info : code={code} val={value}"
            else:
                try:
                    s = f"Info : {Event._format_template(template, value)}"
                except Exception:
                    s = f"Info : code={code} val={value}"
            super().__init__(code, s, frame)

    @staticmethod
    def get_tag() -> str:
        return "info"

    @staticmethod
    def get_tag_config() -> dict:
        return {'foreground': '#0066CC', 'justify': 'left'}


class ResponseEvent(Event):
    TYPE = "R"

    def __init__(self, msg_type: Optional[str], code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
        label = {"R": "Réponse", "O": "OK", "N": "NOK"}.get(msg_type, "Réponse")
        template = MSG.get(code, None)
        if template is None:
            s = f"{label} : code={code} val={value}"
        else:
            try:
                s = f"{label} : {Event._format_template(template, value)}"
            except Exception:
                s = f"{label} : code={code} val={value}"
        super().__init__(code, s, frame)
        
    @staticmethod
    def get_tag() -> str:
        return "rx"

    @staticmethod
    def get_tag_config() -> dict:
        return {'justify': 'left'}

    @staticmethod
    def get_prefix() -> str:
            return '◀'


class StateEvent(Event):
    TYPE = "S"

    def __init__(self, code: Optional[int] = None, value: int = 0, frame: Optional[bytes] = None):
        if code == 250:
            machine_name = "PRODUCTION"
            state_name = ETAT_PROD.get(value, str(value))
        elif code == 251:
            machine_name = "CALIBRATION"
            state_name = ETAT_CALIBRATION.get(value, str(value))
        else:
            machine_name = str(code)
            state_name = str(value)
        s = f"State de la machine {machine_name} : {state_name}"
        super().__init__(value, s, frame)

    @staticmethod
    def get_tag() -> str:
        return "state"

    @staticmethod
    def get_tag_config() -> dict:
        return {'foreground': '#007A29', 'justify': 'left'}

class ASCIIEvent(Event):
    TYPE = "X"

    def __init__(self, text: str, frame: Optional[bytes] = None):
        # ASCIIEvent is created from a text payload
        super().__init__(None, text.replace("\r", " ").replace("\n", " "), frame)

    @staticmethod
    def decode_from_frame(frame: bytes):
        payload = frame[0:]
        try:
            text = payload.decode("utf-8", errors="replace").rstrip("\r\n")
        except Exception:
            text = payload.hex().upper()
        return ASCIIEvent(text, frame)
    


class RequestEvent(Event):
    TYPE = "Q"

    def __init__(self, string):
        super().__init__(None, f"Envoi : {string}", None)

    @staticmethod
    def get_tag() -> str:
        return "tx"

    @staticmethod
    def get_tag_config() -> dict:
        return {'justify': 'left', 'underline': True}

    @staticmethod
    def get_prefix() -> str:
        return '▶'  # No prefix for centered/info messages


class MesureEvent(Event):
    TYPE = "M"
    PAYLOAD_STRUCT = struct.Struct("<BIhhiiHi")

    @staticmethod
    def frame_size() -> int:
        return 1 + MesureEvent.PAYLOAD_STRUCT.size

    @staticmethod
    def decode_from_frame(frame: bytes):
        payload = frame[1:]
        try:
            values = MesureEvent.PAYLOAD_STRUCT.unpack(payload)
        except struct.error:
            return Event(None, f"--- Trame binaire invalide (Mesures) : {frame.hex().upper()} ---", frame)
        return MesureEvent.decode(values, frame)

    @staticmethod
    def decode(values: Tuple, frame: bytes):
        if len(values) != 8:
            return Event(None, f"--- Trame Mesures invalide : {frame.hex().upper()} ---", frame)

        (type, time_ms, tension, courant_moyen, angle_moteur, vitesse_moteur, angle_porte, consigne) = values
        s = (
            "Mesures : "
            f"t={time_ms}ms "
            f"tension={tension / 100:.2f}V "
            f"courant={courant_moyen / 100:.2f}A "
            f"angle_moteur={angle_moteur / 100:.2f}° "
            f"vitesse_moteur={vitesse_moteur / 100:.2f}rad/s "
            f"angle_porte={angle_porte / 100:.2f}° "
            f"consigne={consigne / 100:.2f}"
        )
        inst = MesureEvent(None, s, frame)
        return inst

    @staticmethod
    def get_tag() -> str:
        return "mesures"

    @staticmethod
    def get_tag_config() -> dict:
        return {'justify': 'right'}

    


class AccelEvent(Event):
    TYPE = "A"
    PAYLOAD_STRUCT = struct.Struct("<BIhhhhhhhhh")

    @staticmethod
    def frame_size() -> int:
        return 1 + AccelEvent.PAYLOAD_STRUCT.size

    @staticmethod
    def decode_from_frame(frame: bytes):
        payload = frame[1:]
        try:
            values = AccelEvent.PAYLOAD_STRUCT.unpack(payload)
        except struct.error:
            return Event(None, f"--- Trame binaire invalide (Accelero) : {frame.hex().upper()} ---", frame)
        return AccelEvent.decode(values, frame)

    @staticmethod
    def decode(values: Tuple, frame: bytes):
        if len(values) != 11:
            return Event(None, f"--- Trame Accelero invalide : {frame.hex().upper()} ---", frame)

        (type, time_ms, accel_x, accel_y, accel_z, gyro_x, gyro_y, gyro_z, heading, roll, pitch) = values
        s = (
            "Accéléro : "
            f"t={time_ms}ms "
            f"accel=({accel_x / 100:.2f},{accel_y / 100:.2f},{accel_z / 100:.2f}) m/s² "
            f"gyro=({gyro_x / 16:.2f},{gyro_y / 16:.2f},{gyro_z / 16:.2f})°/s "
            f"euler=({heading / 100:.2f},{roll / 100:.2f},{pitch / 100:.2f})°"
        )
        inst = AccelEvent(None, s, frame)
        return inst

    @staticmethod
    def get_tag() -> str:
        return "accel"

    @staticmethod
    def get_tag_config() -> dict:
        return {'foreground': '#4F4F4F', 'justify': 'right'}

    @staticmethod
    def get_suffix() -> str:
        return '◀'



class GuiEvent(Event):
    TYPE = "G"

    def __init__(self, string: str):
        super().__init__(None, string, None)

    @staticmethod
    def get_tag() -> str:
        return "info_center"

    @staticmethod
    def get_tag_config() -> dict:
        return {'foreground': '#FFFFFF', 'justify': 'center', 'background': '#727171'}


list_events = [Event, ErrorEvent, WarningEvent, InfoEvent, ResponseEvent, StateEvent, ASCIIEvent, RequestEvent, MesureEvent, AccelEvent, GuiEvent]
