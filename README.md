# CH592F Zero-Component BLE Thermometer & Battery Gauge (`BTHome v2`)

Ultra-low-power (**< 4 µA** deep sleep, **~300 µC** per 300 ms beacon burst) BLE temperature and CR2032 battery beacon using only the CH592F's built-in silicon temperature sensor (`CH_INTE_VTEMP`) and internal battery supply monitor (`CH_INTE_VBAT`).

Auto-discovered out of the box by **Home Assistant** (via native Bluetooth or ESPHome Bluetooth Proxy) and **Nordic nRF Connect** using the standard **BTHome v2** (`UUID 0xFCD2`) payload format.

---

## Operation & Zero-Self-Heating Architecture

1. **60-Second RTC Deep Sleep (`SBP_PERIODIC_EVT`)**:
   - The CH592F sleeps in `HAL_SLEEP=TRUE` deep sleep (**`< 4 µA`** with the external `TTP223B` touch IC removed; `~7.5 µA` if `TTP223B` remains populated) with `R8_TEM_SENSOR = 0` and `R8_ADC_CFG = 0` (`0.0 µA` sensor leakage), waking every **60 seconds** via the hardware 32 kHz RTC timer.
   - Pressing **`PB22` (`BOOT`)** wakes the MCU immediately (with a `400 ms` contact-bounce lockout), blips the **`PB15` LED** for `5 ms`, and broadcasts an on-demand measurement burst without disturbing the 60-second RTC schedule.
2. **Pre-RF Sampling & Channel Settling (Zero Silicon Self-Heating)**:
   - Because the MCU dissipates only `~12 µW` during the 60-second deep sleep, the silicon die is at exact thermal equilibrium with ambient room air (`0.000 °C` self-heating).
   - Upon wakeup, [`TempBeacon_SampleSensors()`](APP/temp_beacon.c) samples **`CH_INTE_VBAT` (`4×` averaged, `-12 dB` single-ended mode)** first, then switches to **`CH_INTE_VTEMP` (`16×` averaged, differential mode with `50 µs` bandgap settling)** in the first `~250 µs` **before** enabling the BLE radio PA, and immediately powers down both sensors.
3. **300 ms BTHome v2 Broadcast Burst & Power Profile**:
   - Emits a 9–10 packet `ADV_NONCONN_IND` burst (`30 ms` interval, `300 ms` total duration at `+4 dBm`), sleeping at `< 4 µA` between individual `30 ms` advertising packets (`~300 µC` total charge per burst), then returns to deep sleep for 60 seconds.
   - Average radio current at 1 report/min is $\frac{300\text{ µC}}{60\text{ s}} \approx \mathbf{5.0\text{ µA}}$ (**`~8.8 µA` total average current** including `< 4 µA` deep sleep, yielding **~2.5–3 years** on a single CR2032 coin cell).
4. **Hardware Notes (Sharing the 50 mm Clicker PCB)**:
   - Leaving `U2` (`TTP223B`) unpopulated/removed eliminates its `~3.5 µA` quiescent current and reduces idle sleep draw below **`4 µA`**. `PB14` is parked in `GPIO_ModeIN_PD` (`0.0 µA` leakage).
   - The USB CDC ACM console (`PB10`/`PB11`) remains active for `1.5 s` at cold boot or when a USB host is connected, and automatically powers down the USB PHY to `0.0 µA` when running on battery, using a double-sampled passive host detector that is immune to RF TX transients.

---

## BTHome v2 Advertising Payload (24 Bytes)

```text
02 01 06                             Flags AD (General Discoverable, BR/EDR Not Supported)
0C 16 D2 FC 40                       Service Data AD (len=12, UUID 0xFCD2 BTHome v2 unencrypted)
  00 <pkt_id>                        Object 0x00: Packet ID (uint8, monotonic counter)
  01 <batt_pct>                      Object 0x01: Battery % (uint8, 0..100%)
  02 <temp_lsb> <temp_msb>           Object 0x02: Temperature (int16 LE, 0.01 °C)
  0C <vbat_lsb> <vbat_msb>           Object 0x0C: Battery Voltage (uint16 LE, 0.001 V / mV)
07 09 'T' 'M' 'P' '-' '0' '1'        Complete Local Name ("TMP-01" .. "TMP-99")
```

---

## Building, Calibrating & Flashing

The [`Makefile`](Makefile) tracks compiler flags automatically via `build/.cflags`, so changing `BEACON_ID`, `REPORT_INTERVAL_SEC`, or `TEMP_OFFSET_001C` on the `make` command line automatically triggers a rebuild without needing `make clean`.

```bash
# 1. Run host unit tests
make -C tests

# 2. Build firmware (defaults: BEACON_ID=1 -> "TMP-01", REPORT_INTERVAL_SEC=60, TEMP_OFFSET_001C=-230)
make BEACON_ID=1

# Optional: Override 1-point room-temperature calibration offset (in 0.01 °C units, e.g. -230 = -2.30 °C)
make BEACON_ID=1 TEMP_OFFSET_001C=-230 REPORT_INTERVAL_SEC=60

# 3. Flash over USB (supports buttonless CDC reboot or native WCH ISP)
./flash.sh
```

### USB CDC Console Commands (`115200 8N1`)
* `read` (or `now`) — Sample sensors immediately, blip LED, and transmit a 300 ms BTHome v2 burst.
* `status` (or `info`) — Print last temperature, battery voltage/percentage, and interval.
* `isp` (or `boot`) — Reboot into WCH USB ISP bootloader (`1a86:55e0`).
