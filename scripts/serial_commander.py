#!/usr/bin/env python3
"""
STM32 CAN Board Serial Commander TUI
Usage: python3 serial_commander.py [--port /dev/ttyACM0] [--baud 115200]
"""

import argparse
import collections
import curses
import serial
import serial.tools.list_ports
import subprocess
import sys
import textwrap
import threading

STM32_VID = 0x0483

# ── colour pair IDs ────────────────────────────────────────────────
P_HEADER   = 1   # header bar          white on purple
P_PARAM    = 2   # param values        bold yellow
P_KEY      = 3   # key badges          white on purple
P_TX       = 4   # TX log              green
P_RX_INFO  = 5   # incoming [INFO]     cyan
P_RX_WARN  = 6   # incoming [WARNING]  yellow
P_RX_ERR   = 7   # incoming [ERROR]    red
P_RX_OTHER = 8   # incoming unknown    white
P_SECTION  = 9   # section labels      purple/bold
P_STATUS   = 10  # status bar          white on purple

C_PURPLE = 16    # custom colour slot for #5a2273


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


def pio_runner(target, env, rx_log):
    cmd = ["pio", "run", "-t", target, "-e", env]
    rx_log.append(f"[INFO] PIO: running {' '.join(cmd)}")
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        for line in proc.stdout:
            rx_log.append(line.decode("utf-8", errors="replace").rstrip())
        proc.wait()
        rx_log.append(f"[INFO] PIO: done (exit {proc.returncode})")
    except FileNotFoundError:
        rx_log.append("[ERROR] PIO: 'pio' not found in PATH")
    except Exception as e:
        rx_log.append(f"[ERROR] PIO: {e}")


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
    tx_log.append(msg)


def key_badge(stdscr, row, col, label):
    stdscr.attron(curses.color_pair(P_KEY) | curses.A_BOLD)
    stdscr.addstr(row, col, f" {label} ")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
    return col + len(label) + 2


def edit_field(stdscr, row, col, current, w):
    curses.echo()
    curses.curs_set(1)
    stdscr.addstr(row, col, " " * 14)
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
        stdscr.addstr(row, col, " " * 14)
        stdscr.addstr(row, col, buf)
        stdscr.move(row, col + len(buf))
        stdscr.refresh()
    curses.noecho()
    curses.curs_set(0)
    return buf.strip() or current


def hline(stdscr, row, w, left="─", mid_char=None, mid_col=None, right="─"):
    line = left * (w - 1)
    if mid_char and mid_col:
        line = line[:mid_col] + mid_char + line[mid_col + 1:]
    try:
        stdscr.addstr(row, 0, line[:w - 1])
    except curses.error:
        pass


def draw(stdscr, port, baud, params, tx_log, rx_log, editing):
    stdscr.erase()
    h, w = stdscr.getmaxyx()
    mid = w // 2

    # ╔══ HEADER ════════════════════════════════════════════════════╗
    title = "STM32 SERIAL COMMANDER"
    conn  = f"{port}  ·  {baud} baud"
    stdscr.attron(curses.color_pair(P_HEADER) | curses.A_BOLD)
    stdscr.addstr(0, 2, title)
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
    stdscr.attron(curses.A_DIM)
    try:
        stdscr.addstr(0, w - len(conn) - 2, conn)
    except curses.error:
        pass
    stdscr.attroff(curses.A_DIM)

    # ── top section divider ────────────────────────────────────────
    stdscr.attron(curses.color_pair(P_HEADER))
    hline(stdscr, 1, w)
    stdscr.attroff(curses.A_COLOR)

    # ── COMMANDS (left) ───────────────────────────────────────────
    stdscr.attron(curses.color_pair(P_SECTION) | curses.A_BOLD)
    stdscr.addstr(2, 2, "COMMANDS")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)

    cmds = [("1", "speed"), ("2", "stop"), ("3", "profile"), ("b", "pio build"), ("q", "quit")]
    for i, (k, label) in enumerate(cmds):
        row = 4 + i
        if k == "q":
            row += 1
        c = key_badge(stdscr, row, 2, k)
        stdscr.addstr(row, c + 1, label)

    # ── vertical divider (top section) ────────────────────────────
    for row in range(1, 13):
        try:
            stdscr.addch(row, mid, curses.ACS_VLINE)
        except curses.error:
            pass
    try:
        stdscr.addch(1, mid, curses.ACS_TTEE)
    except curses.error:
        pass

    # ── PARAMETERS (right) ────────────────────────────────────────
    px = mid + 2
    stdscr.attron(curses.color_pair(P_SECTION) | curses.A_BOLD)
    stdscr.addstr(2, px, "MOTOR")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)

    param_defs = [
        (4, "erpm",    "erpm   ", "e"),
        (5, "accel",   "accel  ", "a"),
        (6, "time_ms", "time ms", "t"),
    ]
    for row, key, label, edit_key in param_defs:
        stdscr.attron(curses.A_DIM)
        stdscr.addstr(row, px, label + "  ")
        stdscr.attroff(curses.A_DIM)
        stdscr.attron(curses.color_pair(P_PARAM) | curses.A_BOLD)
        stdscr.addstr(params[key].ljust(8))
        stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
        key_badge(stdscr, row, px + len(label) + 10, edit_key)

    stdscr.attron(curses.color_pair(P_SECTION) | curses.A_BOLD)
    stdscr.addstr(8, px, "PIO")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)

    pio_defs = [
        (9,  "pio_target", "target ", "p"),
        (10, "pio_env",    "env    ", "n"),
    ]
    for row, key, label, edit_key in pio_defs:
        stdscr.attron(curses.A_DIM)
        stdscr.addstr(row, px, label + "  ")
        stdscr.attroff(curses.A_DIM)
        stdscr.attron(curses.color_pair(P_PARAM) | curses.A_BOLD)
        stdscr.addstr(params[key].ljust(8))
        stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
        key_badge(stdscr, row, px + len(label) + 10, edit_key)

    # ── LOG SECTION DIVIDER ────────────────────────────────────────
    div_y = 13
    try:
        stdscr.addstr(div_y, 0,
            "─" * (mid) + "┼" + "─" * (w - mid - 2))
    except curses.error:
        pass

    # ── TX LOG (bottom-left) ───────────────────────────────────────
    stdscr.attron(curses.color_pair(P_SECTION) | curses.A_BOLD)
    stdscr.addstr(div_y + 1, 2, "TX")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)

    log_start = div_y + 2
    log_end   = h - 2
    log_h     = log_end - log_start

    visible_tx = list(tx_log)[-log_h:]
    for i, entry in enumerate(visible_tx):
        if log_start + i >= log_end:
            break
        try:
            stdscr.attron(curses.color_pair(P_TX))
            stdscr.addstr(log_start + i, 2, ("▸ " + entry)[:mid - 3])
            stdscr.attroff(curses.A_COLOR)
        except curses.error:
            break

    # ── vertical divider (log section) ────────────────────────────
    for row in range(div_y, h - 1):
        try:
            stdscr.addch(row, mid, curses.ACS_VLINE)
        except curses.error:
            pass

    # ── INCOMING (bottom-right) ────────────────────────────────────
    stdscr.attron(curses.color_pair(P_SECTION) | curses.A_BOLD)
    stdscr.addstr(div_y + 1, mid + 2, "INCOMING")
    stdscr.attroff(curses.A_COLOR | curses.A_BOLD)

    rx_w = w - mid - 3

    def rx_color(line):
        if "[ERROR]" in line:   return curses.color_pair(P_RX_ERR)
        if "[WARNING]" in line or "[WARN]" in line: return curses.color_pair(P_RX_WARN)
        if "[INFO]" in line:    return curses.color_pair(P_RX_INFO)
        return curses.color_pair(P_RX_OTHER)

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

    # ── STATUS BAR ─────────────────────────────────────────────────
    stdscr.attron(curses.color_pair(P_STATUS))
    hline(stdscr, h - 2, w)
    stdscr.attroff(curses.A_COLOR)
    if editing:
        stdscr.attron(curses.color_pair(P_RX_WARN) | curses.A_BOLD)
        try:
            stdscr.addstr(h - 1, 2, f"✎  {editing}"[:w - 3])
        except curses.error:
            pass
        stdscr.attroff(curses.A_COLOR | curses.A_BOLD)
    else:
        stdscr.attron(curses.color_pair(P_STATUS) | curses.A_DIM)
        try:
            stdscr.addstr(h - 1, 2, "1 speed   2 stop   3 profile   b pio build      e/a/t/p/n edit      q quit"[:w - 3])
        except curses.error:
            pass
        stdscr.attroff(curses.A_COLOR | curses.A_DIM)

    stdscr.refresh()


def tui(stdscr, ser, port, baud):
    curses.start_color()
    curses.use_default_colors()

    # #5a2273 → curses 0-1000 scale: R=353 G=133 B=451
    if curses.can_change_color() and curses.COLORS >= 16:
        curses.init_color(C_PURPLE, 353, 133, 451)
        purple = C_PURPLE
    else:
        purple = curses.COLOR_MAGENTA

    curses.init_pair(P_HEADER,   purple,              -1)
    curses.init_pair(P_PARAM,    curses.COLOR_YELLOW, -1)
    curses.init_pair(P_KEY,      purple,              -1)
    curses.init_pair(P_TX,       curses.COLOR_GREEN,  -1)
    curses.init_pair(P_RX_INFO,  curses.COLOR_CYAN,   -1)
    curses.init_pair(P_RX_WARN,  curses.COLOR_YELLOW, -1)
    curses.init_pair(P_RX_ERR,   curses.COLOR_RED,    -1)
    curses.init_pair(P_RX_OTHER, curses.COLOR_WHITE,  -1)
    curses.init_pair(P_SECTION,  purple,              -1)
    curses.init_pair(P_STATUS,   purple,              -1)
    curses.curs_set(0)
    curses.noecho()
    stdscr.timeout(100)

    params  = {"erpm": "2000", "accel": "1000", "time_ms": "5000",
               "pio_target": "upload", "pio_env": "can_board"}
    tx_log  = collections.deque(maxlen=200)
    rx_log  = collections.deque(maxlen=200)
    tx_log.append(f"connected {port} @ {baud}")
    editing = None

    stop_event = threading.Event()
    reader = threading.Thread(target=serial_reader, args=(ser, rx_log, stop_event), daemon=True)
    reader.start()

    h, w = stdscr.getmaxyx()
    mid   = w // 2

    try:
        while True:
            draw(stdscr, port, baud, params, tx_log, rx_log, editing)
            editing = None
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
                editing = "editing erpm — Enter confirm  ESC cancel"
                draw(stdscr, port, baud, params, tx_log, rx_log, editing)
                val = edit_field(stdscr, 4, mid + 12, params["erpm"], w)
                if val.lstrip('-').isdigit():
                    params["erpm"] = val
                else:
                    tx_log.append(f"!! invalid erpm: {val!r}")
            elif key == ord('a'):
                editing = "editing accel — Enter confirm  ESC cancel"
                draw(stdscr, port, baud, params, tx_log, rx_log, editing)
                val = edit_field(stdscr, 5, mid + 12, params["accel"], w)
                if val.lstrip('-').isdigit():
                    params["accel"] = val
                else:
                    tx_log.append(f"!! invalid accel: {val!r}")
            elif key == ord('t'):
                editing = "editing time_ms — Enter confirm  ESC cancel"
                draw(stdscr, port, baud, params, tx_log, rx_log, editing)
                val = edit_field(stdscr, 6, mid + 12, params["time_ms"], w)
                if val.lstrip('-').isdigit():
                    params["time_ms"] = val
                else:
                    tx_log.append(f"!! invalid time_ms: {val!r}")
            elif key == ord('b'):
                tx_log.append(f"pio run -t {params['pio_target']} -e {params['pio_env']}")
                threading.Thread(
                    target=pio_runner,
                    args=(params["pio_target"], params["pio_env"], rx_log),
                    daemon=True,
                ).start()
            elif key == ord('p'):
                editing = "editing pio target — Enter confirm  ESC cancel"
                draw(stdscr, port, baud, params, tx_log, rx_log, editing)
                val = edit_field(stdscr, 9, mid + 12, params["pio_target"], w)
                if val:
                    params["pio_target"] = val
            elif key == ord('n'):
                editing = "editing pio env — Enter confirm  ESC cancel"
                draw(stdscr, port, baud, params, tx_log, rx_log, editing)
                val = edit_field(stdscr, 10, mid + 12, params["pio_env"], w)
                if val:
                    params["pio_env"] = val
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
