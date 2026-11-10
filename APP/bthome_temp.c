#include "bthome_temp.h"

int16_t ch592_adc_to_temp_001c(int32_t adc_sum,
                               uint8_t sample_count,
                               uint32_t rom_cfg_tmp_25c,
                               int16_t offset_001c)
{
    if (sample_count == 0u) {
        return 2500 + offset_001c;
    }

    int32_t base_c  = (int32_t)((rom_cfg_tmp_25c >> 16) & 0xFFFFu);
    int32_t cal_adc = (int32_t)(rom_cfg_tmp_25c & 0xFFFFu);
    if (base_c <= 0 || base_c > 60) {
        base_c = 25;
    }

    /* Delta across N samples: (adc_sum - N * cal_adc).
     * Each LSB is (100 / 283) deg C = (10000 / 283) in 0.01 deg C units.
     */
    int32_t delta_sum = adc_sum - (cal_adc * (int32_t)sample_count);
    int32_t denom     = 283 * (int32_t)sample_count;
    int32_t num       = delta_sum * 10000;

    /* Round to nearest 0.01 deg C */
    if (num >= 0) {
        num += denom / 2;
    } else {
        num -= denom / 2;
    }

    int32_t temp_001c = (base_c * 100) + (num / denom) + (int32_t)offset_001c;
    if (temp_001c < -5500) {
        temp_001c = -5500;
    } else if (temp_001c > 12500) {
        temp_001c = 12500;
    }
    return (int16_t)temp_001c;
}

uint8_t bthome_cr2032_mv_to_pct(uint16_t vbat_mv)
{
    if (vbat_mv <= CR2032_MIN_MV) {
        return 0u;
    }
    if (vbat_mv >= CR2032_MAX_MV) {
        return 100u;
    }
    return (uint8_t)(((uint32_t)(vbat_mv - CR2032_MIN_MV) * 100u) /
                     (CR2032_MAX_MV - CR2032_MIN_MV));
}

uint8_t bthome_temp_build_adv_packet(const bthome_temp_meas_t *m,
                                     uint8_t beacon_id,
                                     uint8_t out_adv[BTHOME_TEMP_ADV_LEN])
{
    uint8_t idx = 0u;

    /* AD #1: BLE Flags (3 bytes) */
    out_adv[idx++] = 0x02u;
    out_adv[idx++] = 0x01u; /* GAP_ADTYPE_FLAGS */
    out_adv[idx++] = 0x06u; /* General Discoverable + BR/EDR Not Supported */

    /* AD #2: BTHome v2 Service Data (13 bytes) */
    uint8_t svc_len_idx = idx++;
    out_adv[idx++] = 0x16u; /* GAP_ADTYPE_SERVICE_DATA (16-bit UUID) */
    out_adv[idx++] = BTHOME_UUID_LSB;
    out_adv[idx++] = BTHOME_UUID_MSB;
    out_adv[idx++] = BTHOME_V2_INFO_UNENCRYPTED;

    /* Object 0x00: Packet ID (uint8) */
    out_adv[idx++] = BTHOME_OBJ_PACKET_ID;
    out_adv[idx++] = m->packet_id;

    /* Object 0x01: Battery % (uint8) */
    out_adv[idx++] = BTHOME_OBJ_BATTERY_PCT;
    out_adv[idx++] = m->battery_pct;

    /* Object 0x02: Temperature in 0.01 deg C (int16 LE) */
    out_adv[idx++] = BTHOME_OBJ_TEMPERATURE;
    out_adv[idx++] = (uint8_t)((uint16_t)m->temp_001c & 0xFFu);
    out_adv[idx++] = (uint8_t)(((uint16_t)m->temp_001c >> 8) & 0xFFu);

    /* Object 0x0C: Battery Voltage in mV / 0.001 V (uint16 LE) */
    out_adv[idx++] = BTHOME_OBJ_VOLTAGE_MV;
    out_adv[idx++] = (uint8_t)(m->battery_mv & 0xFFu);
    out_adv[idx++] = (uint8_t)((m->battery_mv >> 8) & 0xFFu);

    out_adv[svc_len_idx] = (uint8_t)(idx - svc_len_idx - 1u);

    /* AD #3: Complete Local Name "TMP-01" .. "TMP-99" (8 bytes) */
    out_adv[idx++] = 0x07u;
    out_adv[idx++] = 0x09u; /* GAP_ADTYPE_LOCAL_NAME_COMPLETE */
    out_adv[idx++] = 'T';
    out_adv[idx++] = 'M';
    out_adv[idx++] = 'P';
    out_adv[idx++] = '-';
    out_adv[idx++] = (uint8_t)('0' + ((beacon_id / 10u) % 10u));
    out_adv[idx++] = (uint8_t)('0' + (beacon_id % 10u));

    return idx;
}
