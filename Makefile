CROSS_COMPILE ?= /home/igor/esp32/tools/xpack-riscv-none-elf-gcc-15.2.0-1/bin/riscv-none-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
SIZE = $(CROSS_COMPILE)size

TARGET = ch592_thermometer
BUILD_DIR = build

SRCS_C = \
	APP/temp_beacon_main.c \
	APP/temp_beacon.c \
	APP/bthome_temp.c \
	USB/usb_cdc.c \
	HAL/MCU.c \
	HAL/RTC.c \
	HAL/SLEEP.c \
	StdPeriphDriver/CH59x_sys.c \
	StdPeriphDriver/CH59x_clk.c \
	StdPeriphDriver/CH59x_flash.c \
	StdPeriphDriver/CH59x_gpio.c \
	StdPeriphDriver/CH59x_pwr.c \
	StdPeriphDriver/CH59x_uart1.c \
	StdPeriphDriver/CH59x_adc.c

SRCS_ASM = \
	Startup/startup_CH592.S \
	LIB/ble_task_scheduler.S

INCLUDES = \
	-IAPP/include \
	-IHAL/include \
	-ILIB \
	-IStdPeriphDriver/inc \
	-IRVMSIS \
	-IUSB/include

# Beacon ID (1..99 -> "TMP-01".."TMP-99"): make BEACON_ID=2
BEACON_ID ?= 1
# Periodic RTC broadcast interval in seconds (default 60s): make REPORT_INTERVAL_SEC=60
REPORT_INTERVAL_SEC ?= 60
# Optional 1-point temperature trim in 0.01 deg C (e.g. -310 = -3.10 C)
TEMP_OFFSET_001C ?= -230
# Default TX power: 4 dBm for CR2032 operation (use LL_TX_POWEER_9_DBM for max range)
BEACON_TXPOWER ?= LL_TX_POWEER_4_DBM

CFLAGS = -march=rv32imac_zicsr_zifencei -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore \
         -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -MMD -MP \
         -std=gnu99 -DDEBUG=Debug_USB -DHAL_SLEEP=TRUE -DCLK_OSC32K=1 -DDCDC_ENABLE=FALSE -DINT_SOFT \
         -DBEACON_ID=$(BEACON_ID) -DREPORT_INTERVAL_SEC=$(REPORT_INTERVAL_SEC) \
         -DTEMP_OFFSET_001C=$(TEMP_OFFSET_001C) -DBLE_TX_POWER=$(BEACON_TXPOWER) $(INCLUDES)

ASFLAGS = -march=rv32imac_zicsr_zifencei -mabi=ilp32 $(INCLUDES) -x assembler-with-cpp

LDFLAGS = -march=rv32imac_zicsr_zifencei -mabi=ilp32 -mcmodel=medany \
          -T Ld/Link.ld -nostartfiles -Wl,--gc-sections \
          -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
          --specs=nano.specs --specs=nosys.specs \
          -LStdPeriphDriver -LLIB -lISP592 -lCH59xBLE

OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS_C:.c=.o)) $(addprefix $(BUILD_DIR)/, $(SRCS_ASM:.S=.o))

CFLAGS_STAMP = $(BUILD_DIR)/.cflags

all: $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

$(CFLAGS_STAMP): FORCE
	@mkdir -p $(BUILD_DIR)
	@echo "$(CFLAGS)" | cmp -s - $@ || echo "$(CFLAGS)" > $@

$(BUILD_DIR)/%.o: %.c $(CFLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.S $(CFLAGS_STAMP)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(OBJS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary -S $< $@

flash: $(BUILD_DIR)/$(TARGET).bin
	./flash.sh

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)

FORCE:

.PHONY: all clean flash FORCE
