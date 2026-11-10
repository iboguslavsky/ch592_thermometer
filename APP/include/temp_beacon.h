#ifndef TEMP_BEACON_H
#define TEMP_BEACON_H

#ifdef __cplusplus
extern "C" {
#endif

/* TMOS task event flags */
#define SBP_START_DEVICE_EVT         0x0001
#define SBP_PERIODIC_EVT             0x0002
#define SBP_MANUAL_TRIG_EVT          0x0004
#define SBP_STOP_BURST_EVT           0x0008
#define SBP_LED_OFF_EVT              0x0010
#define SBP_BTN_UNLOCK_EVT           0x0020

#ifndef BEACON_ID
#define BEACON_ID                    1
#endif

/* Default RTC wake & broadcast interval in seconds (default: 60s) */
#ifndef REPORT_INTERVAL_SEC
#define REPORT_INTERVAL_SEC          60
#endif

/* Optional 1-point temperature calibration trim in 0.01 deg C
 * (e.g. -150 = -1.50 C, +80 = +0.80 C) */
#ifndef TEMP_OFFSET_001C
#define TEMP_OFFSET_001C             0
#endif

#define BEACON_REPORT_TICKS          MS1_TO_SYSTEM_TIME((uint32_t)(REPORT_INTERVAL_SEC) * 1000u)

/* 30 ms advertising interval (48 * 0.625 ms), 300 ms burst (~10 packets per 60s report)
 * ~180 uC per minute = ~3 uA average radio current (+ ~4 uA sleep = ~7 uA total). */
#define BEACON_ADV_INTERVAL          48
#define BEACON_BURST_TICKS           480   /* 480 * 0.625 ms = 300 ms */
#define BEACON_BTN_LOCKOUT_TICKS     640   /* 640 * 0.625 ms = 400 ms touch/button debounce */

/* Hardware pins matching the 50mm CH592F Clicker PCB (TTP223 removed on thermometer;
 * PB14 parked GPIO_ModeIN_PD for 0.0 uA sleep, PB22 BOOT active-LOW with PU) */
#define TOUCH_BUTTON_PIN             GPIO_Pin_14 /* PB14 (QFN28 Pin 9, unused/parked PD) */
#define BOOT_BUTTON_PIN              GPIO_Pin_22 /* PB22 (QFN28 Pin 26, manual trigger) */
#define BEACON_INPUT_PINS            (BOOT_BUTTON_PIN)
#define BEACON_LED_PIN               GPIO_Pin_15 /* PB15 (QFN28 Pin 8, active-HIGH) */

extern void TempBeacon_Init(void);
extern uint16_t TempBeacon_ProcessEvent(uint8_t task_id, uint16_t events);
extern void TempBeacon_PollConsole(void);

#ifdef __cplusplus
}
#endif

#endif /* TEMP_BEACON_H */
