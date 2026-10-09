#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Petit terminal série pour Arduino.

Dépendance externe :
    pyserial

"""

from SerialTerminalGui import SerialTerminalGui


def main():
    app = SerialTerminalGui()
    app.mainloop()


if __name__ == "__main__":
    main()
