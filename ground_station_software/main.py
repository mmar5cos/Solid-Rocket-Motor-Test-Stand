#!/usr/bin/env python3
"""
Ground station (Mac) for Arduino over USB serial.

- Opens serial port
- Sends sequential test strings
- Reads and prints anything received (echo)
"""

import sys
import time
from datetime import datetime

import serial
from serial.tools import list_ports


BAUD = 115200
SEND_INTERVAL_S = 1.0   # seconds between messages
NUM_MESSAGES = 50       # how many to send (set to None for infinite)

# If you know your port, set it here, e.g.:
# PORT = "/dev/tty.usbmodem1101"   # common on Mac for Arduino
# PORT = "/dev/tty.usbserial-xxxx" # some adapters
PORT = "/dev/tty.usbserial-0001"

def pick_port_auto() -> str:
    ports = list(list_ports.comports())
    if not ports:
        raise RuntimeError("No serial ports found. Plug in Arduino via USB.")

    # Prefer typical Arduino-ish names on macOS
    preferred = []
    for p in ports:
        dev = p.device
        desc = (p.description or "").lower()
        manu = (p.manufacturer or "").lower()

        score = 0
        if "usbmodem" in dev.lower():
            score += 5
        if "usbserial" in dev.lower():
            score += 4
        if "arduino" in desc or "arduino" in manu:
            score += 3
        if "wch" in desc or "ch340" in desc or "silicon labs" in desc:
            score += 2

        preferred.append((score, dev, p.description))

    preferred.sort(reverse=True)
    best = preferred[0]
    return best[1]

def ts() -> str:
    return datetime.now().strftime("%H:%M:%S.%f")[:-3]

def main():
    port = PORT or pick_port_auto()
    print(f"[{ts()}] Using port: {port}")

    try:
        ser = serial.Serial(port, BAUD, timeout=0.05)  # short timeout for polling
    except Exception as e:
        print(f"Failed to open {port}: {e}")
        print("\nAvailable ports:")
        for p in list_ports.comports():
            print(f"  {p.device}  ({p.description})")
        sys.exit(1)

    # Arduino often resets on serial open; give it a moment
    time.sleep(2.0)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    msg_idx = 0
    next_send = time.time()

    print(f"[{ts()}] Connected at {BAUD} baud.")
    print("Press Ctrl+C to quit.\n")

    try:
        while True:
            now = time.time()

            # Send sequential test messages
            if now >= next_send:
                msg_idx += 1
                payload = f"TEST {msg_idx:04d}\n"   # newline helps readability
                ser.write(payload.encode("utf-8"))
                print(f"[{ts()}] TX: {payload.strip()}")

                next_send = now + SEND_INTERVAL_S

                if NUM_MESSAGES is not None and msg_idx >= NUM_MESSAGES:
                    print(f"\n[{ts()}] Sent {NUM_MESSAGES} messages. Switching to read-only...")
                    # Just keep reading after sending N messages
                    next_send = float("inf")

            # Read any available incoming data
            n = ser.in_waiting
            if n:
                data = ser.read(n)
                try:
                    text = data.decode("utf-8", errors="replace")
                except Exception:
                    text = repr(data)

                # print raw RX stream (could contain partial lines)
                print(f"[{ts()}] RX: {text}", end="")

            # small sleep to keep CPU chill
            time.sleep(0.01)

    except KeyboardInterrupt:
        print(f"\n[{ts()}] Exiting...")

    finally:
        try:
            ser.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()
