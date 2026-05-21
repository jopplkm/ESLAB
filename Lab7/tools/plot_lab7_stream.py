#!/usr/bin/env python3
"""Plot raw vs FIR-filtered accelerometer samples from Lab7 UART CSV stream."""

import argparse
import sys
from collections import deque

try:
    import matplotlib.pyplot as plt
    from matplotlib.animation import FuncAnimation
except ImportError:
    print("Install matplotlib: pip install matplotlib")
    sys.exit(1)

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    print("Install pyserial: pip install pyserial")
    sys.exit(1)


def parse_csv_line(line, scale_mg):
    """Parse 'raw,filt'; firmware sends milli-mg integers by default."""
    parts = line.split(",")
    if len(parts) != 2:
        return None
    try:
        raw = float(parts[0].strip()) / scale_mg
        filt = float(parts[1].strip()) / scale_mg
        return raw, filt
    except ValueError:
        return None


def should_skip_line(line):
    line = line.strip()
    if not line:
        return True
    if line.startswith(("[", "Lab7", "===", "WARN", "LSM6DSL", "Starting")):
        return True
    if "SNR" in line or "FIR" in line or "boot" in line:
        return True
    return False


def list_serial_ports():
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return
    print("Available serial ports:")
    for p in ports:
        print(f"  {p.device}\t{p.description}")


def open_serial(port, baud):
    try:
        return serial.Serial(port, baud, timeout=0.05)
    except serial.SerialException as exc:
        print(f"\nCannot open {port}: {exc}")
        print("\nCommon fixes on Windows:")
        print("  1. Close STM32CubeIDE Serial Monitor (or any terminal on this port).")
        print("  2. Close PuTTY / Tera Term if open.")
        print("  3. Run: python tools/plot_lab7_stream.py --list-ports")
        print("  4. Or save UART log and use: --file uart_log.txt --snapshot")
        sys.exit(1)


def run_snapshot(args):
    raw = []
    filt = []

    def consume(line):
        if should_skip_line(line):
            return
        parsed = parse_csv_line(line, args.scale)
        if parsed:
            raw.append(parsed[0])
            filt.append(parsed[1])

    if args.file:
        with open(args.file, encoding="utf-8", errors="ignore") as f:
            for line in f:
                consume(line)
                if len(raw) >= args.samples:
                    break
    else:
        print(f"Reading {args.samples} samples from {args.port} @ {args.baud} ...")
        print("(Close CubeIDE Serial Monitor first if Permission denied)\n")
        with open_serial(args.port, args.baud) as ser:
            while len(raw) < args.samples:
                line = ser.readline().decode(errors="ignore")
                consume(line)

    if not raw:
        print("No CSV samples parsed.")
        sys.exit(1)

    print(f"Plotting {len(raw)} samples (scale={args.scale} -> mg)")
    fig, ax = plt.subplots(figsize=(10, 4))
    ax.plot(raw, label="raw (mg)", alpha=0.8)
    ax.plot(filt, label="filtered (mg)", alpha=0.8)
    ax.set_title("Lab7 LSM6DSL X-axis (snapshot)")
    ax.set_xlabel("sample")
    ax.set_ylabel("mg")
    ax.legend()
    ax.grid(True)
    plt.tight_layout()
    plt.show()


def run_live(args):
    if args.file:
        print("--live requires serial port; use without --file, or use --snapshot --file ...")
        sys.exit(1)

    window = args.samples
    raw = deque(maxlen=window)
    filt = deque(maxlen=window)

    print(f"Live plot: {args.port} @ {args.baud}, window={window} samples")
    print("Close the plot window to stop.\n")

    ser = open_serial(args.port, args.baud)

    fig, ax = plt.subplots(figsize=(10, 4))
    (line_raw,) = ax.plot([], [], label="raw (mg)", alpha=0.85)
    (line_filt,) = ax.plot([], [], label="filtered (mg)", alpha=0.85)
    ax.set_title("Lab7 LSM6DSL X-axis (live)")
    ax.set_xlabel("sample (rolling window)")
    ax.set_ylabel("mg")
    ax.legend(loc="upper right")
    ax.grid(True)

    def drain_serial():
        while ser.in_waiting:
            line = ser.readline().decode(errors="ignore")
            if should_skip_line(line):
                continue
            parsed = parse_csv_line(line, args.scale)
            if parsed:
                raw.append(parsed[0])
                filt.append(parsed[1])

    def update(_frame):
        drain_serial()
        if len(raw) < 2:
            return line_raw, line_filt

        xs = list(range(len(raw)))
        line_raw.set_data(xs, list(raw))
        line_filt.set_data(xs, list(filt))
        ax.relim()
        ax.autoscale_view()
        return line_raw, line_filt

    ani = FuncAnimation(fig, update, interval=args.interval, blit=True, cache_frame_data=False)
    plt.tight_layout()
    try:
        plt.show()
    finally:
        ser.close()


def main():
    parser = argparse.ArgumentParser(description="Visualize Lab7 UART CSV (live or snapshot).")
    parser.add_argument("--port", default="COM13", help="ST-LINK VCP port")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument(
        "--samples",
        type=int,
        default=300,
        help="Snapshot: total points to collect; Live: rolling window size",
    )
    parser.add_argument(
        "--scale",
        type=float,
        default=1000.0,
        help="Divide UART integers by this to get mg",
    )
    parser.add_argument("--file", help="Input log file (snapshot mode only)")
    parser.add_argument(
        "--snapshot",
        action="store_true",
        help="Collect N samples then show one static plot (old behavior)",
    )
    parser.add_argument(
        "--interval",
        type=int,
        default=50,
        help="Live refresh interval in ms (default 50)",
    )
    parser.add_argument("--list-ports", action="store_true", help="List COM ports and exit")
    args = parser.parse_args()

    if args.list_ports:
        list_serial_ports()
        return

    if args.snapshot or args.file:
        run_snapshot(args)
    else:
        run_live(args)


if __name__ == "__main__":
    main()
