#ifndef BTHOME_TEMP_H
#define BTHOME_TEMP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* BTHome v2 constants (https://bthome.io/format/) */
#define BTHOME_UUID_LSB              0xD2u
#define BTHOME_UUID_MSB              0xFCu
#define BTHOME_V2_INFO_UNENCRYPTED   0x40u

/* BTHome v2 Object IDs (must be serialized in ascending numerical order) */
#define BTHOME_OBJ_PACKET_ID         0x00u /* uint8  (1 byte) */
#define BTHOME_OBJ_BATTERY_PCT       0x01u /* uint8  (1 byte, 1 %) */
#define BTHOME_OBJ_TEMPERATURE       0x02u /* int16  (2 bytes, factor 0.01 deg C) */
#define BTHOME_OBJ_VOLTAGE_MV        0x0Cu /* uint16 (2 bytes, factor 0.001 V = 1 mV) */

#define BTHOME_TEMP_ADV_LEN          26u   /* 3 (Flags) + 15 (BTHome Service Data) + 8 (Name "TMP-xx") */

/* CR2032 coin-cell voltage range for 0..100% state-of-charge mapping */
#define CR2032_MIN_MV                2000u
#define CR2032_MAX_MV                3000u

typedef struct {
    uint8_t  packet_id;
    uint8_t  battery_pct;
    int16_t  temp_001c;   /* Temperature in 0.01 deg C (e.g. 2345 = 23.45 C) */
    uint16_t battery_mv;  /* Supply voltage in mV (e.g. 3020 = 3.020 V) */
} bthome_temp_meas_t;

/* Convert an accumulated sum of `sample_count` CH592 CH_INTE_VTEMP ADC readings
 * into 0.01 deg C units using the factory ROM_CFG_TMP_25C calibration word
 * (stored at 0x7F014 on CH592F) plus an optional user trim `offset_001c`.
 *
 * WCH slope: 2.83 ADC LSB / deg C (i.e. 100 / 283 deg C per LSB).
 * Keeping the calculation in 0.01 deg C across the N-sample sum preserves
 * sub-LSB precision (~0.02 C resolution for 16 averaged samples).
 */
int16_t ch592_adc_to_temp_001c(int32_t adc_sum,
                               uint8_t sample_count,
                               uint32_t rom_cfg_tmp_25c,
                               int16_t offset_001c);

/* Map CR2032 voltage in mV (2000..3000 mV) to 0..100% */
uint8_t bthome_cr2032_mv_to_pct(uint16_t vbat_mv);

/* Build a 24-byte BTHome v2 BLE advertisement payload with name "TMP-xx" */
uint8_t bthome_temp_build_adv_packet(const bthome_temp_meas_t *m,
                                     uint8_t beacon_id,
                                     uint8_t out_adv[BTHOME_TEMP_ADV_LEN]);

#ifdef __cplusplus
}
#endif

#endif /* BTHOME_TEMP_H */
