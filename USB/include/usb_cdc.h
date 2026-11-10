#ifndef __USB_CDC_H__
#define __USB_CDC_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize USB CDC ACM hardware peripheral, endpoints, and ring buffers.
 *        Forces a USB bus disconnect/reconnect cycle to trigger host enumeration.
 */
void usb_cdc_init(void);

/**
 * @brief Check if USB device has been enumerated and configured by the host.
 * @return 1 if configured, 0 otherwise.
 */
uint8_t usb_cdc_is_configured(void);

/**
 * @brief Check if terminal emulator is open on the host (DTR line asserted).
 * @return 1 if terminal connected, 0 otherwise.
 */
uint8_t usb_cdc_is_connected(void);

/**
 * @brief Check if MCU should stay awake (inhibit deep sleep) to service USB.
 *        Returns 1 during the initial boot/reset enumeration grace window (3s),
 *        for 3s after any USB bus reset or setup transfer, and continuously
 *        while enumerated by a USB host (until USB bus suspend/unplug).
 *        Returns 0 when running on battery with no active USB host.
 */
uint8_t usb_cdc_keep_awake(void);

/**
 * @brief Get current baud rate requested by host via SET_LINE_CODING.
 * @return Baud rate (e.g. 115200).
 */
uint32_t usb_cdc_get_baud_rate(void);

/**
 * @brief Transmit data over USB CDC ACM (Bulk IN).
 *        Non-blocking where possible; buffers in ring buffer and sends via ISR.
 *        Includes timeout protection so the CPU never hangs if USB is disconnected.
 * @param buf Pointer to data to send.
 * @param len Number of bytes to send.
 * @return Number of bytes actually written.
 */
int usb_cdc_write(const void *buf, int len);

/**
 * @brief Read data from USB CDC ACM receive buffer (Bulk OUT).
 * @param buf Pointer to destination buffer.
 * @param len Maximum bytes to read.
 * @return Number of bytes read.
 */
int usb_cdc_read(void *buf, int len);

/**
 * @brief Get number of bytes available in the receive ring buffer.
 * @return Number of available bytes.
 */
int usb_cdc_available(void);

/**
 * @brief Read a single character from USB CDC.
 * @return Character (0-255) or -1 if no character is available.
 */
int usb_cdc_getchar_nowait(void);

/**
 * @brief Blocking read for a single character (waits until received or timeout).
 * @param timeout_ms Timeout in milliseconds (0 = wait forever).
 * @return Character (0-255) or -1 on timeout/disconnect.
 */
int usb_cdc_getchar_timeout(uint32_t timeout_ms);

/**
 * @brief Send a single character over USB CDC.
 * @param c Character to send.
 * @return Character sent or -1 on failure.
 */
int usb_cdc_putchar(int c);

/**
 * @brief Flush transmit ring buffer (wait for all buffered data to be sent).
 */
void usb_cdc_flush(void);

/**
 * @brief Enable or disable local character echo during input.
 * @param enable 1 to echo characters, 0 for raw input.
 */
void usb_cdc_set_echo(uint8_t enable);

/**
 * @brief Enable or disable line editing mode for stdio _read / scanf.
 *        When enabled (default), typing in a terminal supports backspace and Enter.
 * @param enable 1 to enable line editing, 0 for raw stream.
 */
void usb_cdc_set_line_mode(uint8_t enable);

/**
 * @brief Interactive line reader with echo and backspace handling.
 * @param buf Buffer to store the read string.
 * @param max_len Maximum buffer length (including null terminator).
 * @return Number of characters read (excluding null terminator), or -1 on error.
 */
int usb_cdc_readline(char *buf, int max_len);

/**
 * @brief Reboot MCU into WCH USB ISP bootloader mode without pressing the physical BOOT button.
 *        Executes from RAM (__HIGH_CODE).
 *        1. Disables all interrupts (PFIC_DisableAllIRQ).
 *        2. Erases application flash at 0x0000 (sector 0, 4096 bytes).
 *        3. Resets flash controller (FLASH_ROM_SW_RESET).
 *        4. Writes 0xFFFF magic signature to R16_INT32K_TUNE.
 *        5. Triggers a full software reset (RB_SOFTWARE_RESET).
 *
 *        Upon reboot, the factory ROM bootloader detects erased sector 0 and the 0xFFFF token,
 *        remaining indefinitely in USB ISP bootloader mode (VID:PID 4348:55e0 / 1a86:55e0)
 *        until new firmware is flashed via wchisp.
 */
void usb_cdc_enter_bootloader(void);

/**
 * @brief Backward-compatibility alias for usb_cdc_enter_bootloader().
 */
void enter_isp_bootloader(void);

/**
 * @brief Enable or disable automatic bootloader entry on 1200 bps baud touch.
 *        When enabled (default: 1), opening the CDC COM port at 1200 baud
 *        automatically triggers usb_cdc_enter_bootloader().
 * @param enable 1 to enable (default), 0 to disable.
 */
void usb_cdc_enable_1200bps_bootloader(uint8_t enable);

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_H__ */
