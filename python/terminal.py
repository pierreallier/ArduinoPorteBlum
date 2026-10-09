#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Petit terminal série pour Arduino.

Dépendance externe :
    pyserial

Installation si nécessaire :
    python -m pip install pyserial

Fonctions :
- Recherche automatique d'un port Arduino au démarrage
- Vitesse par défaut : 115200 bauds
- Port COM et vitesse configurables en bas
- Commande saisie dans le champ supérieur
- Entrée ou bouton "Envoyer" pour transmettre
- Commandes envoyées alignées à gauche
- Données reçues alignées à droite
- Défilement automatique activable/désactivable
- Bouton "Effacer"
- Lecture série dans un thread séparé
"""

import queue
import struct
import threading
import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports

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


class SerialTerminal(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Terminal Arduino")
        self.geometry("900x600")
        self.minsize(650, 400)

        self.serial_port = None
        self.reader_thread = None
        self.reader_running = False
        self.rx_queue = queue.Queue()
        self.rx_buffer = bytearray()

        self.autoscroll = True
        self.show_hex = False

        self._create_widgets()
        self._refresh_ports()

        # Recherche périodique des données reçues.
        self.after(50, self._process_rx_queue)

        # Tentative de connexion automatique.
        self.after(200, self._auto_connect)

        self.protocol("WM_DELETE_WINDOW", self._close)

    # ------------------------------------------------------------------
    # Interface
    # ------------------------------------------------------------------

    def _create_widgets(self):
        # Ligne de commande en haut
        command_frame = ttk.Frame(self, padding=(8, 8, 8, 4))
        command_frame.pack(fill="x")

        self.command_var = tk.StringVar()

        self.command_entry = ttk.Entry(
            command_frame,
            textvariable=self.command_var
        )
        self.command_entry.pack(side="left", fill="x", expand=True)
        self.command_entry.bind("<Return>", self._send_command)

        self.send_button = ttk.Button(
            command_frame,
            text="Envoyer",
            command=self._send_command
        )
        self.send_button.pack(side="left", padx=(6, 0))

        self.show_hex_button = ttk.Button(
            command_frame,
            text="Afficher HEX : OFF",
            command=self._toggle_show_hex
        )
        self.show_hex_button.pack(side="left", padx=(6, 0))

        self.scroll_button = ttk.Button(
            command_frame,
            text="Défilement : ON",
            command=self._toggle_autoscroll
        )
        self.scroll_button.pack(side="left", padx=(6, 0))

        self.clear_button = ttk.Button(
            command_frame,
            text="Effacer",
            command=self._clear_terminal
        )
        self.clear_button.pack(side="left", padx=(6, 0))

        # Terminal
        terminal_frame = ttk.Frame(self, padding=(8, 4, 8, 4))
        terminal_frame.pack(fill="both", expand=True)

        self.terminal = tk.Text(
            terminal_frame,
            wrap="none",
            state="disabled",
            font=("Consolas", 10),
            padx=6,
            pady=6
        )
        self.terminal.pack(side="left", fill="both", expand=True)

        scrollbar_y = ttk.Scrollbar(
            terminal_frame,
            orient="vertical",
            command=self.terminal.yview
        )
        scrollbar_y.pack(side="right", fill="y")

        self.terminal.configure(yscrollcommand=scrollbar_y.set)

        # Tags pour aligner les lignes
        self.terminal.tag_configure("tx", justify="left")
        self.terminal.tag_configure("rx", justify="right")
        self.terminal.tag_configure("info", justify="center")

        # Barre du bas : port / vitesse / connexion
        bottom_frame = ttk.Frame(self, padding=(8, 4, 8, 8))
        bottom_frame.pack(fill="x")

        ttk.Label(bottom_frame, text="Port :").pack(side="left")

        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(
            bottom_frame,
            textvariable=self.port_var,
            width=12,
            state="readonly"
        )
        self.port_combo.pack(side="left", padx=(4, 12))

        self.refresh_button = ttk.Button(
            bottom_frame,
            text="Actualiser",
            command=self._refresh_ports
        )
        self.refresh_button.pack(side="left", padx=(0, 12))

        ttk.Label(bottom_frame, text="Vitesse :").pack(side="left")

        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUDRATE))
        self.baud_combo = ttk.Combobox(
            bottom_frame,
            textvariable=self.baud_var,
            width=10,
            values=(
                "9600",
                "19200",
                "38400",
                "57600",
                "115200",
                "230400",
                "460800",
                "921600",
            )
        )
        self.baud_combo.pack(side="left", padx=(4, 12))

        self.connect_button = ttk.Button(
            bottom_frame,
            text="Connecter",
            command=self._toggle_connection
        )
        self.connect_button.pack(side="left")

        self.status_var = tk.StringVar(value="Recherche de la carte...")
        ttk.Label(
            bottom_frame,
            textvariable=self.status_var
        ).pack(side="right")

        self.command_entry.focus_set()

    # ------------------------------------------------------------------
    # Ports
    # ------------------------------------------------------------------

    def _refresh_ports(self):
        ports = list(serial.tools.list_ports.comports())

        port_names = [port.device for port in ports]
        self.port_combo["values"] = port_names

        # Si le port actuel existe toujours, on le conserve.
        current = self.port_var.get()

        if current in port_names:
            return

        # Recherche automatique d'un Arduino.
        arduino_port = self._find_arduino_port(ports)

        if arduino_port is not None:
            self.port_var.set(arduino_port)
        elif port_names:
            self.port_var.set(port_names[0])
        else:
            self.port_var.set("")

    @staticmethod
    def _find_arduino_port(ports):
        """
        Essaie d'identifier automatiquement un Arduino.

        On regarde notamment :
        - Arduino
        - Mega
        - Uno
        - Leonardo
        - Micro
        - CH340 / CH341
        - USB Serial
        - CP210
        - FTDI

        Si plusieurs ports correspondent, le premier est choisi.
        Si aucun ne correspond, None est retourné.
        """
        keywords = (
            "arduino",
            "mega",
            "uno",
            "leonardo",
            "micro",
            "ch340",
            "ch341",
            "cp210",
            "ftdi",
            "usb serial",
        )

        for port in ports:
            text = " ".join(
                str(value)
                for value in (
                    port.description,
                    port.manufacturer,
                    port.product,
                    port.hwid,
                )
                if value
            ).lower()

            if any(keyword in text for keyword in keywords):
                return port.device

        return None

    def _auto_connect(self):
        self._refresh_ports()

        if self.port_var.get():
            self._connect()

    # ------------------------------------------------------------------
    # Connexion série
    # ------------------------------------------------------------------

    def _toggle_connection(self):
        if self.serial_port is not None and self.serial_port.is_open:
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        port = self.port_var.get().strip()

        if not port:
            messagebox.showwarning(
                "Port série",
                "Aucun port série sélectionné."
            )
            return

        try:
            baudrate = int(self.baud_var.get())
        except ValueError:
            messagebox.showerror(
                "Vitesse",
                "La vitesse doit être un nombre."
            )
            return

        self._disconnect()

        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=SERIAL_TIMEOUT
            )
        except serial.SerialException as exc:
            self.serial_port = None
            self.status_var.set("Connexion impossible")
            messagebox.showerror(
                "Erreur de connexion",
                f"Impossible d'ouvrir {port} :\n\n{exc}"
            )
            return

        self.reader_running = True
        self.reader_thread = threading.Thread(
            target=self._reader,
            daemon=True
        )
        self.reader_thread.start()

        self.connect_button.configure(text="Déconnecter")
        self.status_var.set(f"Connecté à {port} @ {baudrate}")

        self._write_terminal(
            f"--- Connexion à {port} @ {baudrate} ---",
            "info"
        )

        self.command_entry.focus_set()

    def _disconnect(self):
        self.reader_running = False

        if self.serial_port is not None:
            try:
                self.serial_port.close()
            except serial.SerialException:
                pass

        self.serial_port = None
        self.connect_button.configure(text="Connecter")

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

                # enqueue raw binary frame; formatting/decoding occurs at display time
                self.rx_queue.put(("bin", frame, message_name, payload_struct))
                continue

            newline_index = self.rx_buffer.find(b"\n")

            if newline_index == -1:
                return

            line = bytes(self.rx_buffer[:newline_index + 1])
            del self.rx_buffer[:newline_index + 1]

            # enqueue raw line bytes; formatting occurs at display time
            self.rx_queue.put(("line", line))

    def _decode_binary_frame(self, frame, message_name, payload_struct):
        payload = frame[1:]

        try:
            values = payload_struct.unpack(payload)
        except struct.error:
            return f"--- Trame binaire invalide ({message_name}) : {self._format_hex(frame)} ---"

        # Event message: (type: uint8, code: uint8, value: int32)
        if message_name == "Event":
            if len(values) < 3:
                return f"--- Trame Event invalide : {self._format_hex(frame)} ---"

            message_type, code, value = values
            message_type_text = self._display_ascii_byte(message_type)
            event_label = self._event_label(message_type_text)

            # Special handling for State events
            if message_type_text == 'S':
                if code == 250:  # ETAT_PROD
                    state_name = ETAT_PROD.get(value, str(value))
                elif code == 251:  # ETAT_CALIBRATION
                    state_name = ETAT_CALIBRATION.get(value, str(value))
                else:
                    state_name = str(value)

                code_text = self._message_name(code)
                return (
                    f"{event_label} : type={message_type_text} "
                    f"code={code} ({code_text}) val={state_name}"
                )

            # Non-state events: format the message template using value
            template = MSG.get(code, None)
            if template is None:
                formatted = str(value)
            else:
                formatted = self._format_template(template, value)

            code_text = self._message_name(code)
            return (
                f"Event {event_label} | type={message_type_text} "
                f"code={code} ({code_text}) val={formatted}"
            )

        # Mesures message: expect 8 values
        if message_name == "Mesures":
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
        # MesuresBN0055: expect 10 values (accel x/y/z, time, gyro x/y/z, heading, roll, pitch)
        if message_name == "MesuresBN0055":
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

        return f"--- Trame binaire inconnue ({message_name}) : {self._format_hex(frame)} ---"

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

    def _format_template(self, template, value):
        """Format message templates from `codes.MSG`.

        Rules:
        - `${A|B|C}` -> select option by integer `value` (0->A,1->B,...)
        - `$.N` -> divide `value` by 10**N and format with N decimals
        - `$` -> raw integer value
        """
        import re

        def repl_choice(match):
            options = match.group(1).split("|")
            try:
                idx = int(value)
                return options[idx] if 0 <= idx < len(options) else str(value)
            except Exception:
                return str(value)

        # handle ${A|B|C}
        result = re.sub(r"\$\{([^}]+)\}", repl_choice, template)

        # handle $.N patterns
        def repl_scale(match):
            nd = int(match.group(1))
            try:
                scaled = value / (10 ** nd)
                return f"{scaled:.{nd}f}"
            except Exception:
                return str(value)

        result = re.sub(r"\$\.(\d+)", repl_scale, result)

        # handle bare $ (not followed by . or {)
        result = re.sub(r"\$(?![\.\{])", str(value), result)

        return result

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

    # ------------------------------------------------------------------
    # Réception
    # ------------------------------------------------------------------

    def _process_rx_queue(self):
        try:
            while True:
                item = self.rx_queue.get_nowait()

                # item can be:
                # - ("info", text)
                # - ("bin", frame_bytes, message_name, payload_struct)
                # - ("line", line_bytes)

                if isinstance(item, tuple):
                    kind = item[0]

                    if kind == "info":
                        _, text = item
                        self._write_terminal(text, "info")
                    elif kind == "bin":
                        _, frame, message_name, payload_struct = item

                        if self.show_hex:
                            self._write_terminal(self._format_hex(frame), "rx")
                        else:
                            decoded = self._decode_binary_frame(frame, message_name, payload_struct)
                            self._write_terminal(decoded, "rx")
                    elif kind == "line":
                        _, line_bytes = item

                        if self.show_hex:
                            self._write_terminal(self._format_hex(line_bytes), "rx")
                        else:
                            text = line_bytes.decode("utf-8", errors="replace").rstrip("\r\n")
                            self._write_terminal(text, "rx")
                    else:
                        # Unknown tuple form: stringify
                        self._write_terminal(str(item), "rx")
                else:
                    # legacy string item
                    self._write_terminal(item, "rx")
        except queue.Empty:
            pass

        self.after(50, self._process_rx_queue)

    # ------------------------------------------------------------------
    # Envoi
    # ------------------------------------------------------------------

    def _send_command(self, event=None):
        command = self.command_var.get()

        if not command:
            return "break"

        if self.serial_port is None or not self.serial_port.is_open:
            self._write_terminal(
                "--- Non connecté ---",
                "info"
            )
            return "break"

        try:
            # Envoi avec LF, adapté à un parser Arduino utilisant '\n'.
            self.serial_port.write(
                (command + "\n").encode("utf-8")
            )
        except (serial.SerialException, OSError) as exc:
            self._write_terminal(
                f"--- Erreur d'envoi : {exc} ---",
                "info"
            )
            return "break"

        self._write_terminal(command, "tx")

        self.command_var.set("")
        self.command_entry.focus_set()

        return "break"

    # ------------------------------------------------------------------
    # Affichage
    # ------------------------------------------------------------------

    def _write_terminal(self, text, tag):
        self.terminal.configure(state="normal")
        self.terminal.insert("end", text + "\n", tag)
        self.terminal.configure(state="disabled")

        if self.autoscroll:
            self.terminal.see("end")

    def _clear_terminal(self):
        self.terminal.configure(state="normal")
        self.terminal.delete("1.0", "end")
        self.terminal.configure(state="disabled")
        self.command_entry.focus_set()

    def _toggle_autoscroll(self):
        self.autoscroll = not self.autoscroll

        if self.autoscroll:
            self.scroll_button.configure(
                text="Défilement : ON"
            )
            self.terminal.see("end")
        else:
            self.scroll_button.configure(
                text="Défilement : OFF"
            )

        self.command_entry.focus_set()

    def _toggle_show_hex(self):
        self.show_hex = not self.show_hex

        if self.show_hex:
            self.show_hex_button.configure(text="Afficher HEX : ON")
            self._write_terminal("--- Affichage HEX : ON ---", "info")
        else:
            self.show_hex_button.configure(text="Afficher HEX : OFF")
            self._write_terminal("--- Affichage HEX : OFF ---", "info")

        self.command_entry.focus_set()

    # ------------------------------------------------------------------
    # Fermeture
    # ------------------------------------------------------------------

    def _close(self):
        self._disconnect()
        self.destroy()


if __name__ == "__main__":
    app = SerialTerminal()
    app.mainloop()
