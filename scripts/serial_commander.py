#!/usr/bin/env python3
"""
STM32 CAN Board Serial Commander TUI
Usage: python3 serial_commander.py [--port /dev/ttyACM0] [--baud 115200]

Keys:
  [1]  send speed <erpm>
  [2]  send stop
  [3]  send profile <erpm> <accel> <time_ms>
  [e]  edit erpm
  [a]  edit accel
  [t]  edit time_ms
  [q]  quit
"""

import argparse
import collections
import curses
import serial
import serial.tools.list_ports
import sys
import textwrap
import threading

STM32_VID = 0x0483


def auto_detect_port():
    matches = [p for p in serial.tools.list_ports.comports() if p.vid == STM32_VID]
    if len(matches) == 0:
        print("No STM32 device found (VID 0x0483). Use --port.")
        sys.exit(1)
    if len(matches) > 1:
        print("Multiple STM32 devices:")
        for i, p in enumerate(matches):
            print(f"  [{i}] {p.device} — {p.description}")
        idx = int(input("Select: "))
        return matches[idx].device
    return matches[0].device


def serial_reader(ser, rx_log, stop_event):
    while not stop_event.is_set():
        try:
            line = ser.readline()
            if line:
                rx_log.append(line.decode("utf-8", errors="replace").rstrip())
        except Exception:
            break


def send(ser, msg, tx_log):
    ser.write((msg + "\n").encode())
    tx_log.append(f">> {msg}")


def edit_field(stdscr, row, col, current, w):
    curses.echo()
    curses.curs_set(1)
    stdscr.addstr(row, col, " " * 12)
    stdscr.addstr(row, col, current)
    stdscr.move(row, col)
    stdscr.refresh()
    buf = ""
    while True:
        ch = stdscr.getch()
        if ch in (10, 13):
            break
        if ch == 27:
            buf = current
            break
        if ch in (127, curses.KEY_BACKSPACE, 8):
            buf = buf[:-1]
        elif 32 <= ch < 127:
            buf += chr(ch)
        stdscr.addstr(row, col, " " * 12)
        stdscr.addstr(row, col, buf)
        stdscr.move(row, col + len(buf))
        stdscr.refresh()
    curses.noecho()
    curses.curs_set(0)
    return buf.strip() or current


def draw(stdscr, port, baud, params, tx_log, rx_log, status):
    stdscr.erase()
    h, w = stdscr.getmaxyx()
    mid = w // 2

    # ── header ─────────────────────────────────────────────────
    header = f" STM32 Serial Commander  |  {port}  |  {baud} baud "
    stdscr.attron(curses.A_REVERSE)
    stdscr.addstr(0, 0, header.ljust(w - 1))
    stdscr.attroff(curses.A_REVERSE)

    # ── left: commands ─────────────────────────────────────────
    stdscr.attron(curses.A_BOLD)
    stdscr.addstr(2, 2, "COMMANDS")
    stdscr.attroff(curses.A_BOLD)
    stdscr.addstr(4, 2, "[1]  speed")
    stdscr.addstr(5, 2, "[2]  stop")
    stdscr.addstr(6, 2, "[3]  profile")
    stdscr.addstr(8, 2, "[q]  quit")

    # ── right: params ──────────────────────────────────────────
    px = w // 2
    stdscr.attron(curses.A_BOLD)
    stdscr.addstr(2, px, "PARAMETERS")
    stdscr.attroff(curses.A_BOLD)

    for row, key, label, edit_key in [
        (4, "erpm",    "erpm    :", "e"),
        (5, "accel",   "accel   :", "a"),
        (6, "time_ms", "time_ms :", "t"),
    ]:
        stdscr.addstr(row, px, label + " ")
        stdscr.attron(curses.color_pair(1) | curses.A_BOLD)
        stdscr.addstr(params[key].ljust(10))
        stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
        stdscr.attron(curses.A_DIM)
        stdscr.addstr(f" [{edit_key}]")
        stdscr.attroff(curses.A_DIM)

    # ── horizontal divider ─────────────────────────────────────
    div_y = 10
    stdscr.addstr(div_y - 1, 0, "─" * (mid - 1) + "┬" + "─" * (w - mid - 1))

    # ── bottom-left: TX log ────────────────────────────────────
    stdscr.attron(curses.A_BOLD)
    stdscr.addstr(div_y, 2, "TX LOG")
    stdscr.attroff(curses.A_BOLD)

    log_start = div_y + 1
    log_end = h - 3
    log_h = log_end - log_start
    visible_tx = list(tx_log)[-log_h:]
    for i, entry in enumerate(visible_tx):
        if log_start + i >= log_end:
            break
        try:
            stdscr.attron(curses.color_pair(2))
            stdscr.addstr(log_start + i, 2, entry[:mid - 3])
            stdscr.attroff(curses.A_COLOR)
        except curses.error:
            break

    # ── vertical divider in log area ───────────────────────────
    for row in range(div_y - 1, h - 2):
        try:
            ch = curses.ACS_LTEE if row == div_y - 1 else curses.ACS_VLINE
            stdscr.addch(row, mid, ch)
        except curses.error:
            pass

    # ── bottom-right: RX incoming ──────────────────────────────
    stdscr.attron(curses.A_BOLD)
    stdscr.addstr(div_y, mid + 2, "INCOMING")
    stdscr.attroff(curses.A_BOLD)

    rx_w = w - mid - 3

    def rx_color(line):
        if "[ERROR]" in line:
            return curses.color_pair(6)
        if "[WARNING]" in line or "[WARN]" in line:
            return curses.color_pair(5)
        if "[INFO]" in line:
            return curses.color_pair(4)
        return curses.color_pair(7)

    # wrap all lines, keep color tied to original line
    wrapped = []
    for line in rx_log:
        color = rx_color(line)
        for wline in textwrap.wrap(line, rx_w) or [line]:
            wrapped.append((wline, color))

    visible_rx = wrapped[-log_h:]
    for i, (line, color) in enumerate(visible_rx):
        if log_start + i >= log_end:
            break
        try:
            stdscr.attron(color)
            stdscr.addstr(log_start + i, mid + 2, line)
            stdscr.attroff(curses.A_COLOR)
        except curses.error:
            break

    # ── status bar ─────────────────────────────────────────────
    stdscr.addstr(h - 2, 0, "─" * (w - 1))
    stdscr.attron(curses.A_DIM)
    stdscr.addstr(h - 1, 2, status[:w - 3])
    stdscr.attroff(curses.A_DIM)

    stdscr.refresh()


def tui(stdscr, ser, port, baud):
    curses.start_color()
    curses.use_default_colors()
    curses.init_pair(1, curses.COLOR_YELLOW, -1)   # param values
    curses.init_pair(2, curses.COLOR_GREEN, -1)    # tx log
    curses.init_pair(3, curses.COLOR_RED, -1)      # error
    curses.init_pair(4, curses.COLOR_CYAN, -1)     # incoming [INFO]
    curses.init_pair(5, curses.COLOR_YELLOW, -1)   # incoming [WARNING]
    curses.init_pair(6, curses.COLOR_RED, -1)      # incoming [ERROR]
    curses.init_pair(7, curses.COLOR_WHITE, -1)    # incoming unknown
    curses.curs_set(0)
    curses.noecho()
    stdscr.timeout(100)

    params = {"erpm": "2000", "accel": "1000", "time_ms": "5000"}
    tx_log = collections.deque(maxlen=200)
    rx_log = collections.deque(maxlen=200)
    tx_log.append(f"Connected: {port} @ {baud}")
    status = "[1] speed  [2] stop  [3] profile  |  [e/a/t] edit params  |  [q] quit"

    stop_event = threading.Event()
    reader = threading.Thread(target=serial_reader, args=(ser, rx_log, stop_event), daemon=True)
    reader.start()

    h, w = stdscr.getmaxyx()
    px = w // 2

    try:
        while True:
            draw(stdscr, port, baud, params, tx_log, rx_log, status)
            key = stdscr.getch()

            if key == -1:
                continue
            elif key == ord('q'):
                break
            elif key == ord('1'):
                send(ser, f"speed {params['erpm']}", tx_log)
            elif key == ord('2'):
                send(ser, "stop", tx_log)
            elif key == ord('3'):
                send(ser, f"profile {params['erpm']} {params['accel']} {params['time_ms']}", tx_log)
            elif key == ord('e'):
                draw(stdscr, port, baud, params, tx_log, rx_log, "Editing erpm — Enter confirm, ESC cancel")
                val = edit_field(stdscr, 4, px + 10, params["erpm"], w)
                if val.lstrip('-').isdigit():
                    params["erpm"] = val
                else:
                    tx_log.append(f"!! Invalid erpm: {val!r}")
            elif key == ord('a'):
                draw(stdscr, port, baud, params, tx_log, rx_log, "Editing accel — Enter confirm, ESC cancel")
                val = edit_field(stdscr, 5, px + 10, params["accel"], w)
                if val.lstrip('-').isdigit():
                    params["accel"] = val
                else:
                    tx_log.append(f"!! Invalid accel: {val!r}")
            elif key == ord('t'):
                draw(stdscr, port, baud, params, tx_log, rx_log, "Editing time_ms — Enter confirm, ESC cancel")
                val = edit_field(stdscr, 6, px + 10, params["time_ms"], w)
                if val.lstrip('-').isdigit():
                    params["time_ms"] = val
                else:
                    tx_log.append(f"!! Invalid time_ms: {val!r}")
    finally:
        stop_event.set()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default=None)
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    port = args.port or auto_detect_port()

    try:
        ser = serial.Serial(port, args.baud, timeout=1)
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        sys.exit(1)

    try:
        curses.wrapper(tui, ser, port, args.baud)
    finally:
        ser.close()


if __name__ == "__main__":
    main()
