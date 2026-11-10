/***********************************************
 * temp_beacon_main.c — CH592F internal thermometer + battery gauge entry point
 ***********************************************/
#include "CONFIG.h"
#include "HAL.h"
#include "temp_beacon.h"
#include "usb_cdc.h"

__attribute__((aligned(4))) uint32_t MEM_BUF[BLE_MEMHEAP_SIZE / 4];

#if (defined(BLE_MAC)) && (BLE_MAC == TRUE)
const uint8_t MacAddr[6] = {0x84, 0xC2, 0xE4, 0x03, 0x02, 0x20};
#endif

__HIGH_CODE
__attribute__((noinline))
void Main_Circulation(void)
{
    while (1) {
        TMOS_SystemProcess();
        TempBeacon_PollConsole();
    }
}

int main(void)
{
#if (defined(DCDC_ENABLE)) && (DCDC_ENABLE == TRUE)
    PWR_DCDCCfg(ENABLE);
#endif
    SetSysClock(CLK_SOURCE_PLL_60MHz);

    /* Park all GPIOs low-leakage immediately at boot */
    GPIOA_ModeCfg(GPIO_Pin_All, GPIO_ModeIN_PD);
    GPIOB_ModeCfg(GPIO_Pin_All & ~(BEACON_INPUT_PINS), GPIO_ModeIN_PD);
    GPIOB_ModeCfg(BEACON_INPUT_PINS, GPIO_ModeIN_PU);

    CH59x_BLEInit();
    HAL_Init();
    /* Initialize USB CDC ACM subsystem AFTER HAL_Init() so RTC_InitTime() has zeroed the RTC */
    usb_cdc_init();
    setvbuf(stdout, NULL, _IONBF, 0);
    PRINT("%s\n", VER_LIB);
    GAPRole_BroadcasterInit();
    TempBeacon_Init();
    Main_Circulation();
}
