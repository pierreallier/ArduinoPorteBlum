#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import queue
import threading
import tkinter as tk
from tkinter import ttk, messagebox
import serial

from SerialPortHandler import SerialPortHandler

DEFAULT_BAUDRATE = 115200

class SerialTerminalGui(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("Terminal Arduino")
        self.geometry("900x600")
        self.minsize(650, 400)

        self.rx_queue = queue.Queue()
        self.handler = SerialPortHandler(self.rx_queue)

        self.autoscroll = True
        self.show_hex = False

        self._create_widgets()
        self._refresh_ports()

        # Poll rx_queue
        self.after(50, self._process_rx_queue)

        # Try auto connect shortly after startup
        self.after(200, self._auto_connect)

        self.protocol("WM_DELETE_WINDOW", self._close)

    # --- GUI building (same as before) ---
    def _create_widgets(self):
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

        # Sent commands (tx) align to the right; received frames (rx) align to the left
        self.terminal.tag_configure("tx", justify="right")
        self.terminal.tag_configure("rx", justify="left")
        # Severity/color tags for received messages
        self.terminal.tag_configure("err", foreground="#C00000", justify="left")
        self.terminal.tag_configure("warn", foreground="#D28C00", justify="left")
        self.terminal.tag_configure("info", foreground="#0066CC", justify="left")
        self.terminal.tag_configure("state", foreground="#007A29", justify="left")
        # Centered informational tag
        self.terminal.tag_configure("info_center", foreground="#FFFFFF", justify="center", background="#727171")

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

    # Ports helpers
    def _refresh_ports(self):
        import serial.tools.list_ports

        ports = list(serial.tools.list_ports.comports())

        port_names = [port.device for port in ports]
        self.port_combo["values"] = port_names

        current = self.port_var.get()

        if current in port_names:
            return

        # Find Arduino-like port
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

        arduino_port = None
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

            if any(k in text for k in keywords):
                arduino_port = port.device
                break

        if arduino_port is not None:
            self.port_var.set(arduino_port)
        elif port_names:
            self.port_var.set(port_names[0])
        else:
            self.port_var.set("")

    def _auto_connect(self):
        self._refresh_ports()

        if self.port_var.get():
            self._connect()

    # Connection management (delegate to handler)
    def _toggle_connection(self):
        if self.handler.is_connected():
            self._disconnect()
        else:
            self._connect()

    def _connect(self):
        port = self.port_var.get().strip()

        if not port:
            messagebox.showwarning("Port série", "Aucun port série sélectionné.")
            return

        try:
            baudrate = int(self.baud_var.get())
        except ValueError:
            messagebox.showerror("Vitesse", "La vitesse doit être un nombre.")
            return

        try:
            self.handler.connect(port, baudrate)
        except Exception as exc:
            self.status_var.set("Connexion impossible")
            messagebox.showerror("Erreur de connexion", f"Impossible d'ouvrir {port} :\n\n{exc}")
            return

        self.connect_button.configure(text="Déconnecter")
        self.status_var.set(f"Connecté à {port} @ {baudrate}")
        self._write_terminal(f"--- Connexion à {port} @ {baudrate} ---", "info")
        self.command_entry.focus_set()

    def _disconnect(self):
        try:
            self.handler.disconnect()
        except Exception:
            pass

        self.connect_button.configure(text="Connecter")

    # Reception processing
    def _process_rx_queue(self):
        try:
            while True:
                item = self.rx_queue.get_nowait()

                if isinstance(item, tuple):
                    kind = item[0]

                    if kind == "info":
                        _, text = item
                        self._write_terminal(text, "info")
                    elif kind == "bin":
                        _, frame, message_name, payload_struct = item

                        if self.show_hex:
                            self._write_terminal(self.handler._format_hex(frame), "rx")
                        else:
                            decoded = self.handler.decode_frame(frame, message_name, payload_struct)
                            self._write_terminal(decoded, "rx")
                    elif kind == "line":
                        _, line_bytes = item

                        if self.show_hex:
                            self._write_terminal(self.handler._format_hex(line_bytes), "rx")
                        else:
                            text = line_bytes.decode("utf-8", errors="replace").rstrip("\r\n")
                            self._write_terminal(text, "rx")
                    else:
                        self._write_terminal(str(item), "rx")
                else:
                    self._write_terminal(item, "rx")
        except queue.Empty:
            pass

        self.after(50, self._process_rx_queue)

    # Sending
    def _send_command(self, event=None):
        command = self.command_var.get()

        if not command:
            return "break"

        if not self.handler.is_connected():
            self._write_terminal("--- Non connecté ---", "info")
            return "break"

        try:
            self.handler.serial_port.write((command + "\n").encode("utf-8"))
        except (serial.SerialException, OSError) as exc:
            self._write_terminal(f"--- Erreur d'envoi : {exc} ---", "info")
            return "break"

        self._write_terminal(command, "tx")
        self.command_var.set("")
        self.command_entry.focus_set()
        return "break"

    # Display helpers
    def _write_terminal(self, text, tag):
        self.terminal.configure(state="normal")

        out_tag = tag

        # For received frames, determine severity-based color tag
        if tag == "rx":
            out_tag = self._severity_tag(text)

        # Keep existing centered info behavior
        if tag == "info":
            out_tag = "info_center"

        self.terminal.insert("end", text + "\n", out_tag)
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
            self.scroll_button.configure(text="Défilement : ON")
            self.terminal.see("end")
        else:
            self.scroll_button.configure(text="Défilement : OFF")

        self.command_entry.focus_set()

    def _toggle_show_hex(self):
        self.show_hex = not self.show_hex

        if self.show_hex:
            self.show_hex_button.configure(text="Afficher HEX : ON")
            self._write_terminal("--- Affichage HEX : ON ---", "info")
        else:
            self.show_hex_button.configure(text="Afficher HEX : OFF")
            self._write_terminal("--- Affichage HEX : OFF ---", "info")

        # inform handler
        try:
            self.handler.set_show_hex(self.show_hex)
        except Exception:
            pass

        self.command_entry.focus_set()

    def _close(self):
        self._disconnect()
        self.destroy()

    def _severity_tag(self, text: str) -> str:
        """Return a tag name based on the message content for coloring.

        Mapping:
        - Error / NOK -> 'err' (red)
        - Warning -> 'warn' (yellow)
        - Info / OK -> 'info' (blue)
        - State -> 'state' (green)
        - Otherwise -> 'rx' (default)
        """
        if not text:
            return "rx"

        t = text.strip().lower()

        # Check for explicit event labels produced by the decoder
        if t.startswith("erreur") or " nok" in t or t.endswith(" nok") or "nok" in t:
            return "err"

        if t.startswith("warning") or "warning" in t or "warn" in t:
            return "warn"

        if t.startswith("state") or "state" in t or "etat" in t:
            return "state"

        if t.startswith("info") or t.startswith("ok") or " ok" in t or "(ok)" in t:
            return "info"

        return "rx"
