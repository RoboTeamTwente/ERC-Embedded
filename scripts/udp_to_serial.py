#!/usr/bin/env python3
"""
UDP → Serial bridge for can_board STM32.

Listens for UDP datagrams and forwards them as newline-terminated
ASCII commands to the STM over serial.

STM commands (SerialCommandTask):
  speed <int>              - set motor speed (ERPM)
  stop                     - stop all motors
  profile <erpm> <accel> <time_s>  - run s-curve speed profile

Usage:
  python3 udp_to_serial.py [--port /dev/ttyACM0] [--baud 115200] [--udp-port 5000]
"""

import argparse
import socket
import serial
import serial.tools.list_ports
import sys
import time
import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(message)s",
)
log = logging.getLogger(__name__)

VALID_COMMANDS = ("speed ", "stop", "profile ")

STM32_VID = 0x0483


def auto_detect_port() -> str:
    while True:
        matches = [p for p in serial.tools.list_ports.comports() if p.vid == STM32_VID]
        if len(matches) == 1:
            port = matches[0].device
            log.info("Auto-detected STM32 on %s (%s)", port, matches[0].description)
            return port
        if len(matches) > 1:
            ports = ", ".join(f"{p.device} ({p.description})" for p in matches)
            log.error("Multiple STM32 devices found: %s. Use --port to specify.", ports)
            sys.exit(1)
        log.info("Waiting for STM32 (VID 0x%04X)...", STM32_VID)
        time.sleep(1)


def wait_for_port(port: str) -> None:
    while True:
        available = [p.device for p in serial.tools.list_ports.comports()]
        if port in available:
            return
        log.info("Waiting for %s to appear...", port)
        time.sleep(1)


def is_valid_command(msg: str) -> bool:
    return any(msg.startswith(c) for c in VALID_COMMANDS)


def run(serial_port: str, baud: int, udp_host: str, udp_port: int) -> None:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((udp_host, udp_port))
    log.info("UDP listening on %s:%d", udp_host, udp_port)

    ser = serial.Serial(serial_port, baud, timeout=1)
    log.info("Serial open %s @ %d baud", serial_port, baud)

    try:
        while True:
            data, addr = sock.recvfrom(256)
            msg = data.decode("utf-8", errors="replace").strip()
            if not msg:
                continue

            log.info("UDP %s → %r", addr, msg)

            if not is_valid_command(msg):
                log.warning("Unknown command, ignoring: %r", msg)
                continue

            ser.write((msg + "\n").encode("utf-8"))
            log.info("Serial TX: %r", msg)

    except KeyboardInterrupt:
        log.info("Shutting down")
    finally:
        ser.close()
        sock.close()


def main() -> None:
    parser = argparse.ArgumentParser(description="UDP to serial bridge for can_board")
    parser.add_argument("--port", default=None, help="Serial port (default: auto-detect STM32 by USB VID)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--udp-host", default="0.0.0.0", help="UDP bind host (default: 0.0.0.0)")
    parser.add_argument("--udp-port", type=int, default=5000, help="UDP port to listen on (default: 5000)")
    args = parser.parse_args()

    if args.port:
        wait_for_port(args.port)
        port = args.port
    else:
        port = auto_detect_port()

    try:
        run(port, args.baud, args.udp_host, args.udp_port)
    except serial.SerialException as e:
        log.error("Serial error: %s", e)
        sys.exit(1)


if __name__ == "__main__":
    main()
