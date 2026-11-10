#!/usr/bin/env bash
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BIN="$DIR/build/ch592_thermometer.bin"
make -C "$DIR"

# Check if WCH ISP device is already present in USB topology (unless a specific CDC port $1 was requested)
if [ -n "$1" ] || ! lsusb 2>/dev/null | grep -q -E "4348:55e0|1a86:55e0"; then
    CDC_PORT=""
    if [ -n "$1" ]; then
        if [ -e "$1" ]; then
            CDC_PORT="$1"
        else
            CDC_PORT=$(ls /dev/serial/by-id/*"$1"* 2>/dev/null | head -n 1 || true)
        fi
        if [ -z "$CDC_PORT" ] || [ ! -e "$CDC_PORT" ]; then
            echo "Error: Specified device '$1' not found."
            exit 1
        fi
    else
        mapfile -t PORTS < <(ls /dev/serial/by-id/*CH592* 2>/dev/null || true)
        if [ "${#PORTS[@]}" -eq 1 ]; then
            CDC_PORT="${PORTS[0]}"
        elif [ "${#PORTS[@]}" -gt 1 ]; then
            echo "Multiple CH592 CDC devices found:"
            for i in "${!PORTS[@]}"; do
                TTY=$(readlink -f "${PORTS[$i]}")
                ID=$(basename "${PORTS[$i]}")
                echo "  [$((i + 1))] $TTY  ($ID)"
            done
            read -r -p "Select device to flash [1-${#PORTS[@]}]: " choice
            if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt "${#PORTS[@]}" ]; then
                echo "Invalid selection. Aborting."
                exit 1
            fi
            CDC_PORT="${PORTS[$((choice - 1))]}"
        fi
    fi

    if [ -n "$CDC_PORT" ] && [ -e "$CDC_PORT" ]; then
        echo "Found active CH592 CDC console at $CDC_PORT. Requesting buttonless ISP reboot..."
        python3 -c "
import serial, time
try:
    s = serial.Serial('$CDC_PORT', 1200, timeout=0.2)
    s.close()
    time.sleep(0.1)
except Exception:
    pass
try:
    s = serial.Serial('$CDC_PORT', 115200, timeout=0.2)
    s.write(b'isp\r\n')
    s.flush()
    s.close()
except Exception:
    pass
" 2>/dev/null || true

        for _ in {1..30}; do
            if lsusb 2>/dev/null | grep -q -E "4348:55e0|1a86:55e0"; then
                break
            fi
            sleep 0.1
        done
    else
        echo "Waiting for WCH USB ISP device (hold BOOT while plugging in, or touch pad after plugging in USB)..."
        for _ in {1..50}; do
            if lsusb 2>/dev/null | grep -q -E "4348:55e0|1a86:55e0"; then
                break
            fi
            sleep 0.1
        done
    fi
fi

wchisp flash -V "$BIN"
