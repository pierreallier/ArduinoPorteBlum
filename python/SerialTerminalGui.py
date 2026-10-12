#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import queue
import threading
import tkinter as tk
from tkinter import ttk, messagebox
import serial

from SerialPortHandler import SerialPortHandler
from frames import list_events, Event, GuiEvent, RequestEvent

DEFAULT_BAUDRATE = 115200


class SerialTerminalGui(tk.Tk):
    """Serial terminal GUI that reads Event objects from `rx_queue`.

    The GUI expects `rx_queue` to contain instances of `Event` and displays them.
    """

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
        self._refresh_ports() # populate ports via handler

        # Poll rx_queue periodically
        self.after(50, self._process_rx_queue)

        # Try auto connect shortly after startup
        self.after(200, self._auto_connect)

        self.protocol("WM_DELETE_WINDOW", self._close)

    def _create_widgets(self):
        command_frame = ttk.Frame(self, padding=(8, 8, 8, 4))
        command_frame.pack(fill="x")

        self.command_var = tk.StringVar()

        self.command_entry = ttk.Entry(command_frame, textvariable=self.command_var)
        self.command_entry.pack(side="left", fill="x", expand=True)
        self.command_entry.bind("<Return>", self._send_command)

        self.send_button = ttk.Button(command_frame, text="Envoyer", command=self._send_command)
        self.send_button.pack(side="left", padx=(6, 0))

        self.show_hex_button = ttk.Button(command_frame, text="Afficher HEX : OFF", command=self._toggle_show_hex)
        self.show_hex_button.pack(side="left", padx=(6, 0))

        self.scroll_button = ttk.Button(command_frame, text="Défilement : ON", command=self._toggle_autoscroll)
        self.scroll_button.pack(side="left", padx=(6, 0))

        self.clear_button = ttk.Button(command_frame, text="Effacer", command=self._clear_terminal)
        self.clear_button.pack(side="left", padx=(6, 0))

        terminal_frame = ttk.Frame(self, padding=(8, 4, 8, 4))
        terminal_frame.pack(fill="both", expand=True)

        self.terminal = tk.Text(terminal_frame, wrap="none", state="disabled", font=("Consolas", 10), padx=6, pady=6)
        self.terminal.pack(side="left", fill="both", expand=True)

        scrollbar_y = ttk.Scrollbar(terminal_frame, orient="vertical", command=self.terminal.yview)
        scrollbar_y.pack(side="right", fill="y")
        self.terminal.configure(yscrollcommand=scrollbar_y.set)

        self.terminal.configure(selectbackground="#D3D3D3", selectforeground="black")
        
        valid_keys = {"foreground", "background", "justify", "font", "underline", "overstrike"}
        for ev in list_events:
            name = ev.get_tag()
            cfg = ev.get_tag_config() or {}
            kwargs = {k: v for k, v in cfg.items() if v is not None and k in valid_keys}
            self.terminal.tag_configure(name, **kwargs)

        bottom_frame = ttk.Frame(self, padding=(8, 4, 8, 8))
        bottom_frame.pack(fill="x")

        ttk.Label(bottom_frame, text="Port :").pack(side="left")

        self.port_var = tk.StringVar()
        self.port_combo = ttk.Combobox(bottom_frame, textvariable=self.port_var, width=12, state="readonly")
        self.port_combo.pack(side="left", padx=(4, 12))

        self.refresh_button = ttk.Button(bottom_frame, text="Actualiser", command=self._refresh_ports)
        self.refresh_button.pack(side="left", padx=(0, 12))

        ttk.Label(bottom_frame, text="Vitesse :").pack(side="left")

        self.baud_var = tk.StringVar(value=str(DEFAULT_BAUDRATE))
        self.baud_combo = ttk.Combobox(
            bottom_frame,
            textvariable=self.baud_var,
            width=10,
            values=("9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"),
        )
        self.baud_combo.pack(side="left", padx=(4, 12))

        self.connect_button = ttk.Button(bottom_frame, text="Connecter", command=self._toggle_connection)
        self.connect_button.pack(side="left")

        self.status_var = tk.StringVar(value="Recherche de la carte...")
        ttk.Label(bottom_frame, textvariable=self.status_var).pack(side="right")

        self.command_entry.focus_set()

    def _refresh_ports(self):
        try:
            ports, suggested = self.handler.list_ports()
        except Exception:
            ports, suggested = [], None

        self.port_combo["values"] = ports

        current = self.port_var.get()
        if current in ports:
            return

        if suggested and suggested in ports:
            self.port_var.set(suggested)
        elif ports:
            self.port_var.set(ports[0])
        else:
            self.port_var.set("")

    def _auto_connect(self):
        self._refresh_ports()
        if self.port_var.get():
            self._connect()

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
        self._write_terminal(GuiEvent(f"--- Connexion à {port} @ {baudrate} ---"))
        self.command_entry.focus_set()

    def _disconnect(self):
        try:
            self.handler.disconnect()
        except Exception:
            pass
        self.connect_button.configure(text="Connecter")

    def _process_rx_queue(self):
        try:
            while True:
                item = self.rx_queue.get_nowait()

                self._write_terminal(item)
        except queue.Empty:
            pass
        self.after(50, self._process_rx_queue)

    def _send_command(self, event=None):
        command = self.command_var.get()
        if not command:
            return "break"
        if not self.handler.is_connected():
            self._write_terminal(GuiEvent("--- Non connecté ---"))
            return "break"
        try:
            self.handler.serial_port.write((command + "\n").encode("utf-8"))
        except (serial.SerialException, OSError) as exc:
            self._write_terminal(GuiEvent(f"--- Erreur d'envoi : {exc} ---"))
            return "break"
        self._write_terminal(RequestEvent(command))
        self.command_var.set("")
        self.command_entry.focus_set()
        return "break"

    def _write_terminal(self, item : Event):
        self.terminal.configure(state="normal")
        if self.show_hex:
            self.terminal.insert("end", item.get_hex() + "\n", item.get_tag())
        else:
            self.terminal.insert("end", item.get_prefix() + " " + item.get_string() + " " + item.get_suffix() + "\n", item.get_tag())
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
            self._write_terminal(GuiEvent("--- Affichage HEX : ON ---"))
        else:
            self.show_hex_button.configure(text="Afficher HEX : OFF")
            self._write_terminal(GuiEvent("--- Affichage HEX : OFF ---"))
        self.command_entry.focus_set()

    def _close(self):
        self._disconnect()
        self.destroy()


if __name__ == "__main__":
    app = SerialTerminalGui()
    app.mainloop()
