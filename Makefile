#
#   FQBN        Fully Qualified Board Name; if not set in the environment
#               it will be assigned a value in this makefile
#
#   SERIAL_DEV  Serial device to upload the sketch; if not set in the
#               environment it will be assigned:
#               /dev/ttyUSB0   if it exists, or
#               /dev/ttyACM0   if it exists, or
#               unknown
#
#   V           verbose flag; can be 0 (quiet) or 1 (verbose); if not set
#               in the environment it will be assigned a default value
#               in this makefile


MAKE_DIR   := $(PWD)
FQBN        ?= arduino:megaavr:nona4809
V          ?= 1
VFLAG      =

ifeq "$(V)" "1"
VFLAG      =-v
endif

MONITOR_CMD = minicom -con -D

ifndef SERIAL_DEV
  ifneq (,$(wildcard /dev/ttyUSB0))
    SERIAL_DEV = /dev/ttyUSB0
  else ifneq (,$(wildcard /dev/ttyACM0))
    SERIAL_DEV = /dev/ttyACM0
  else
    SERIAL_DEV = unknown
  endif
endif

BUILD_DIR  := $(subst :,.,build/$(FQBN))

SRC        := $(wildcard ./*/*.ino)
HDRS       := $(wildcard ./*/*.h)
LIBS       := ./*/libs/
BIN        := $(BUILD_DIR)/$(SRC).bin
ELF        := $(BUILD_DIR)/$(SRC).elf


$(info FQBN       is [${FQBN}])
$(info V          is [${V}])
$(info VFLAG      is [${VFLAG}])
$(info MAKE_DIR   is [${MAKE_DIR}])
$(info BUILD_DIR  is [${BUILD_DIR}])
$(info SRC        is [${SRC}])
$(info LIBS       is [${LIBS}])
$(info HDRS       is [${HDRS}])
$(info BIN        is [${BIN}])
$(info SERIAL_DEV is [${SERIAL_DEV}])

all: $(ELF) compile
.PHONY: all

compile: $(ELF)
.PHONY: compile

$(ELF): $(SRC) $(HDRS)
	arduino-cli compile -b $(FQBN) --build-path $(BUILD_DIR) --libraries $(LIBS)  --log-level trace $(VFLAG) $(SRC)

upload:
	@if [ ! -c $(SERIAL_DEV) ] ; \
	then echo "---> ERROR: Serial Device not available, please set the SERIAL_DEV environment variable" ; \
	else echo "---> Uploading sketch\n"; \
	arduino-cli upload -b $(FQBN) -p $(SERIAL_DEV) $(VFLAG) --input-dir $(BUILD_DIR) $(SRC); \
	fi

clean:
	@echo "---> Cleaning the build directory"
	rm -rf build

monitor:
	$(MONITOR_CMD) $(SERIAL_DEV)

find:
	avahi-browse _arduino._tcp --resolve --parsable --terminate

reset:
	/usr/bin/ard-reset-arduino $(SERIAL_DEV)

requirements:
	@if [ -e requirements.txt ]; \
	then while read -r i ; do echo ; \
	  echo "---> Installing " '"'$$i'"' ; \
	  arduino-cli lib install "$$i" ; \
	done < requirements.txt ; \
	else echo "---> MISSING requirements.txt file"; \
	fi

