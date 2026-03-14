import sys
import time
import serial
import serial.tools.list_ports


def pick_port() -> str:
    ports = list(serial.tools.list_ports.comports())
    if not ports:
        print("No serial ports found. Plug in the Arduino and try again.")
        sys.exit(1)

    print("Available serial ports:")
    for i, p in enumerate(ports):
        print(f"  [{i}] {p.device}  ({p.description})")

    while True:
        s = input("Select port index: ").strip()
        if s.isdigit() and 0 <= int(s) < len(ports):
            return ports[int(s)].device
        print("Invalid selection.")


def cmd_to_log_basename(cmd: str) -> str | None:
    """
    If cmd is 'st_test1' -> returns 'test1'
    Otherwise returns None.
    """
    cmd = cmd.strip()
    if not cmd.startswith("st_"):
        return None
    name = cmd[3:].strip()
    return name if name else None


def main():
    port = pick_port()
    baud = 9600  # keep exactly like your working version
    READ_TIMEOUT_SEC = 0.1

    FIRE_DURATION_SEC = 40.0  # <-- YOU control this (seconds)

    print(f"\nConnecting to {port} @ {baud}...")
    ser = serial.Serial(port, baudrate=baud, timeout=READ_TIMEOUT_SEC)

    # Give Arduino time to reset when serial opens (common on Uno/Nano)
    time.sleep(2.0)
    ser.reset_input_buffer()

    print("\nType a command like: st_test1")
    print(f"If command starts with st_, logs to <name>.txt for {FIRE_DURATION_SEC} seconds.")
    print("Type 'quit' to exit.\n")

    try:
        while True:
            cmd = input("> ").strip()
            if not cmd:
                continue
            if cmd.lower() in ("q", "quit", "exit"):
                break

            # Send command terminated by newline (receiver expects \n)
            ser.write((cmd + "\n").encode("utf-8"))
            ser.flush()
            print(f"[sent] {cmd}")

            test_name = cmd
            log_fp = None
            if test_name is not None:
                log_name = f"{test_name}.txt"
                log_fp = open(log_name, "w", encoding="utf-8")
                log_fp.write(f"# cmd={cmd}\n")
                log_fp.write(f"# start_unix={time.time()}\n")
                log_fp.flush()
                print(f"[log] saving to {log_name}")

            # Read for a fixed duration (instead of silence-based)
            run_duration = FIRE_DURATION_SEC if test_name is not None else 3.0
            start_time = time.monotonic()
            end_time = start_time + run_duration

            while True:
                remaining = end_time - time.monotonic()
                if remaining <= 0:
                    break
                
                ser.timeout = min(READ_TIMEOUT_SEC, remaining)

                line = ser.readline().decode("utf-8", errors="replace").strip()
                if not line:
                    continue
                
                print(f"[stand] {line}")
                if log_fp is not None:
                    log_fp.write(line + "\n")
                    log_fp.flush()

    finally:
        ser.close()
        print("Disconnected.")


if __name__ == "__main__":
    main()
