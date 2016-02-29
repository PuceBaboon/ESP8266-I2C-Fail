# Changelog
# Changed the variables to include the header file directory
# Added global var for the XTENSA tool root
#
# This make file still needs some work.
#
#
# Output directors to store intermediate compiled files
# relative to the project directory
BUILD_BASE	= build
FW_BASE		= firmware
FLAVOR = release
#FLAVOR = debug

# Base directory for the compiler
##XTENSA_TOOLS_ROOT ?= c:/Espressif/xtensa-lx106-elf/bin
XTENSA_TOOLS_ROOT ?= /opt/Espressif/crosstool-NG/builds/xtensa-lx106-elf/bin


# base directory of the ESP8266 SDK package, absolute
##SDK_BASE	?= c:/Espressif/ESP8266_SDK
SDK_BASE	?= /opt/Espressif/esp_iot_sdk_v1.0.0

#Esptool.py path and port
ESPTOOL		?= /opt/Espressif/esptool-py/esptool.py
ESPPORT		?= /dev/ttyUSB0

MQTT_LUA	= ./esp_nodemcu_mqtt_release
BLANK_512K	= ./Archive/blank512k.bin

# name for the target project
TARGET		= app

# which modules (subdirectories) of the project to include in compiling
MODULES		= driver mqtt user modules
EXTRA_INCDIR    = include $(SDK_BASE)/../extra_include

# libraries used in this project, mainly provided by the SDK
LIBS		= c gcc hal phy pp net80211 lwip wpa upgrade main ssl

# compiler flags using during compilation of source files
CFLAGS		= -Os -Wpointer-arith -Wundef -Werror -Wl,-EL -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals  -D__ets__ -DICACHE_FLASH -DI2C_MASTER_SDA_GPIO=2 -DI2C_MASTER_SCL_GPIO=14

# linker flags used to generate the main object file
LDFLAGS		= -nostdlib -Wl,--no-check-sections -u call_user_start -Wl,-static

ifeq ($(FLAVOR),debug)
    CFLAGS += -g -O2
    LDFLAGS += -g -O2
endif

ifeq ($(FLAVOR),release)
##    CFLAGS += -g -O0
##    LDFLAGS += -g -O0
	CFLAGS += -O2		## Drop the profiling, build for size.
	LDFLAGS += -O2		## Drop the profiling, build for size.
endif

# linker script used for the above linkier step
LD_SCRIPT	= eagle.app.v6.ld

# various paths from the SDK used in this project
SDK_LIBDIR	= lib
SDK_LDDIR	= ld
SDK_INCDIR	= include include/json

# we create two different files for uploading into the flash
# these are the names and options to generate them
FW_FILE_1	= 0x00000
FW_FILE_1_ARGS	= -bo $@ -bs .text -bs .data -bs .rodata -bc -ec
##% FW_FILE_2	= 0x40000
FW_FILE_2	= 0x10000
FW_FILE_2_ARGS	= -es .irom0.text $@ -ec

# select which tools to use as compiler, librarian and linker
CC		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc
AR		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-ar
LD		:= $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-gcc

####
#### no user configurable options below here
####
##FW_TOOL		?= $(XTENSA_TOOLS_ROOT)/esptool
FW_TOOL		?= /usr/bin/esptool
SRC_DIR		:= $(MODULES)
BUILD_DIR	:= $(addprefix $(BUILD_BASE)/,$(MODULES))

SDK_LIBDIR	:= $(addprefix $(SDK_BASE)/,$(SDK_LIBDIR)) -L /opt/Espressif/extra_lib
SDK_INCDIR	:= $(addprefix -I$(SDK_BASE)/,$(SDK_INCDIR))

SRC		:= $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ		:= $(patsubst %.c,$(BUILD_BASE)/%.o,$(SRC))
LIBS		:= $(addprefix -l,$(LIBS))
APP_AR		:= $(addprefix $(BUILD_BASE)/,$(TARGET)_app.a)
TARGET_OUT	:= $(addprefix $(BUILD_BASE)/,$(TARGET).out)

LD_SCRIPT	:= $(addprefix -T$(SDK_BASE)/$(SDK_LDDIR)/,$(LD_SCRIPT))

INCDIR	:= $(addprefix -I,$(SRC_DIR))
EXTRA_INCDIR	:= $(addprefix -I,$(EXTRA_INCDIR))
MODULE_INCDIR	:= $(addsuffix /include,$(INCDIR))

FW_FILE_1	:= $(addprefix $(FW_BASE)/,$(FW_FILE_1).bin)
FW_FILE_2	:= $(addprefix $(FW_BASE)/,$(FW_FILE_2).bin)
BLANKER	:= $(addprefix $(SDK_BASE)/,bin/blank.bin)

V ?= $(VERBOSE)
ifeq ("$(V)","1")
Q :=
vecho := @true
else
Q := @
vecho := @echo
endif

vpath %.c $(SRC_DIR)

define compile-objects
$1/%.o: %.c
	$(vecho) "CC $$<"
	$(Q) $(CC) $(INCDIR) $(MODULE_INCDIR) $(EXTRA_INCDIR) $(SDK_INCDIR) $(CFLAGS)  -c $$< -o $$@
##%
##% The following lines added 15th June 2015 in an effort to reclaim some RAM.  This
##% incantation is meant to move text and literals out of RAM and into flash (irom0).
##% [ *** See the TARGET_OUT section below for additional change *** ]
##%     Taken from:-   http://www.esp8266.com/viewtopic.php?f=6&t=3266
##%
# move all object files to irom
OC      := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objcopy
OD      := $(XTENSA_TOOLS_ROOT)/xtensa-lx106-elf-objdump
	$(Q) $(OC) --rename-section .text=.irom0.text --rename-section .literal=.irom0.literal $$@
##% End of first section (addition 15th June 2015).
endef

.PHONY: all checkdirs clean

all: checkdirs $(TARGET_OUT) $(FW_FILE_1) $(FW_FILE_2)

$(FW_FILE_1): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_1_ARGS)

$(FW_FILE_2): $(TARGET_OUT)
	$(vecho) "FW $@"
	$(Q) $(FW_TOOL) -eo $(TARGET_OUT) $(FW_FILE_2_ARGS)

##% *** The line "$(Q) $(OD) -h $@" was added to enable the move of literals and txt to irom (see above). ***
$(TARGET_OUT): $(APP_AR)
	$(vecho) "LD $@"
	$(Q) $(LD) -L$(SDK_LIBDIR) $(LD_SCRIPT) $(LDFLAGS) -Wl,--start-group $(LIBS) $(APP_AR) -Wl,--end-group -o $@
##% Start of second section (addition 15th June 2015).
	$(Q) $(OD) -h $@
##% End of second section (addition 15th June 2015).

$(APP_AR): $(OBJ)
	$(vecho) "AR $@"
	$(Q) $(AR) cru $@ $^

checkdirs: $(BUILD_DIR) $(FW_BASE)

$(BUILD_DIR):
	$(Q) mkdir -p $@

firmware:
	$(Q) mkdir -p $@

##% flash: firmware/0x00000.bin firmware/0x40000.bin
##% 	$(PYTHON) $(ESPTOOL) -p $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x3C000 $(BLANKER) 0x40000 firmware/0x40000.bin 
flash: firmware/0x00000.bin firmware/0x10000.bin
##-	$(PYTHON) $(ESPTOOL) -p $(ESPPORT) write_flash 0x00000 firmware/blank512k.bin 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin 
	$(PYTHON) $(ESPTOOL) -p $(ESPPORT) write_flash 0x00000 firmware/0x00000.bin 0x10000 firmware/0x10000.bin 

flash_lua:	$(MQTT_LUA)/0x00000.bin $(MQTT_LUA)/0x10000.bin
	$(ESPTOOL) -p $(ESPPORT) write_flash 0x00000 $(MQTT_LUA)/0x00000.bin 0x10000 $(MQTT_LUA)/0x10000.bin

flash_sclean:	$(BLANK_512K)
	$(ESPTOOL) -p $(ESPPORT) write_flash 0x00000 $(BLANK_512K)


test: flash
	screen $(ESPPORT) 115200

rebuild: clean all

clean:
	$(Q) rm -f $(APP_AR)
	$(Q) rm -f $(TARGET_OUT)
	$(Q) rm -rf $(BUILD_DIR)
	$(Q) rm -rf $(BUILD_BASE)
	$(Q) rm -f $(FW_FILE_1)
	$(Q) rm -f $(FW_FILE_2)
	$(Q) rm -rf $(FW_BASE)

$(foreach bdir,$(BUILD_DIR),$(eval $(call compile-objects,$(bdir))))
