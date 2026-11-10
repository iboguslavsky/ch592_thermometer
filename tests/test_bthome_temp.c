#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "bthome_temp.h"

static void test_temp_conversion(void)
{
    /* Factory word: 25 C in upper 16 bits, 1380 ADC counts in lower 16 bits */
    uint32_t rom_c25 = (25u << 16) | 1380u;

    /* Exact 25.00 C across 16 samples */
    assert(ch592_adc_to_temp_001c(1380 * 16, 16, rom_c25, 0) == 2500);

    /* +2.83 LSB/C => +283 counts per sample = +100.00 C => 125.00 C */
    assert(ch592_adc_to_temp_001c((1380 + 283) * 16, 16, rom_c25, 0) == 12500);

    /* -28.3 LSB => -10.00 C => 15.00 C (16 * 28.3 = 452.8 -> 453 counts below) */
    int16_t t_15c = ch592_adc_to_temp_001c((1380 * 16) - 453, 16, rom_c25, 0);
    assert(t_15c >= 1499 && t_15c <= 1500);

    /* Sub-zero freezer temperature (-18.00 C = 25 - 43 C => -43 * 2.83 = -121.69 LSB/sample) */
    int16_t t_freezer = ch592_adc_to_temp_001c((1380 * 16) - 1947, 16, rom_c25, 0);
    assert(t_freezer >= -1801 && t_freezer <= -1799);

    /* User 1-point trim offset (-1.50 C = -150) */
    assert(ch592_adc_to_temp_001c(1380 * 16, 16, rom_c25, -150) == 2350);
}

static void test_battery_pct(void)
{
    assert(bthome_cr2032_mv_to_pct(1800) == 0u);
    assert(bthome_cr2032_mv_to_pct(2000) == 0u);
    assert(bthome_cr2032_mv_to_pct(2500) == 50u);
    assert(bthome_cr2032_mv_to_pct(3000) == 100u);
    assert(bthome_cr2032_mv_to_pct(3250) == 100u);
}

static void test_bthome_packet(void)
{
    bthome_temp_meas_t m = {
        .packet_id   = 0x2Au,
        .battery_pct = 86u,
        .temp_001c   = 2345,  /* 23.45 C = 0x0929 */
        .battery_mv  = 2860u  /* 2860 mV = 0x0B2C */
    };
    uint8_t adv[BTHOME_TEMP_ADV_LEN];
    uint8_t len = bthome_temp_build_adv_packet(&m, 1u, adv);

    assert(len == BTHOME_TEMP_ADV_LEN);
    const uint8_t expected[BTHOME_TEMP_ADV_LEN] = {
        0x02, 0x01, 0x06,                   /* Flags */
        0x0E, 0x16, 0xD2, 0xFC, 0x40,       /* BTHome v2 Service Data header (len=14) */
        0x00, 0x2A,                         /* Packet ID = 42 */
        0x01, 86,                           /* Battery = 86% */
        0x02, 0x29, 0x09,                   /* Temp = 23.45 C (0x0929 LE) */
        0x0C, 0x2C, 0x0B,                   /* Voltage = 2860 mV (0x0B2C LE) */
        0x07, 0x09, 'T', 'M', 'P', '-', '0', '1'
    };
    assert(memcmp(adv, expected, BTHOME_TEMP_ADV_LEN) == 0);
}

int main(void)
{
    test_temp_conversion();
    test_battery_pct();
    test_bthome_packet();
    printf("All ch592_thermometer unit tests PASSED.\n");
    return 0;
}
