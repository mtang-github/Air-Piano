# Builds:
#	air_piano_emu : emulator stubs
#	air_piano_bbb : Beaglebone Black build
#
# Usage:
#	make emu
#	make bbb
#	make clean

HOST_CC  := gcc
CROSS_CC := arm-linux-gnueabihf-gcc
CFLAGS   := -Wall -Wextra -O2 -static -std=c11 \
            -D_POSIX_C_SOURCE=200809L

# shared source files
APP_SRC := src/main.c \
       	   src/beam_thread.c \
       	   src/distance_thread.c \
	   	   src/display_thread.c \
       	   src/stop.c \
       	   src/timing.c \
       	   src/gpio_common.c \
	   	   src/distance_state.c \
	   	   src/failsafe_state.c \
	   	   src/timestamp_queue.c \
	   	   src/priority.c \
	   	   src/mutex_init.c \
	   	   src/logging_state.c \
	   	   src/logging_thread.c \
	   	   src/console_thread.c

# platform-dependent source files
EMU_SRC := src/pin_io_emu.c \
	       src/pwm_emu.c

BBB_SRC := src/pin_io_bbb.c \
		   src/pwm_sysfs.c \
		   src/gpio_bbb_sysfs.c

# Outputs
HOST_BIN := build/air_piano_host
EMU_BIN  := build/air_piano_emu
BBB_BIN  := build/air_piano_bbb

.PHONY: all host emu bbb clean help

all: help

help:
	@echo "Targets:"
	@echo "  make host  - build $(HOST_BIN) (native gcc, run on BBB directly)"
	@echo "  make emu   - build $(EMU_BIN)  (emulator stub, cross compiler)"
	@echo "  make bbb   - build $(BBB_BIN)  (cross-compiled for BBB)"
	@echo "  make clean - remove build artifacts"

host: $(HOST_BIN)

emu: $(EMU_BIN)

bbb: $(BBB_BIN)

$(HOST_BIN): $(APP_SRC) $(BBB_SRC)
	mkdir -p build
	$(HOST_CC) $(CFLAGS) -o $@ $^ -lpthread -lrt

$(EMU_BIN): $(APP_SRC) $(EMU_SRC)
	mkdir -p build
	$(CROSS_CC) $(CFLAGS) -DEMU -o $@ $^ -lpthread -lrt

$(BBB_BIN): ${APP_SRC} $(BBB_SRC)
	mkdir -p build
	$(CROSS_CC) $(CFLAGS) -o $@ $^ -lpthread -lrt

clean:
	rm -rf build
