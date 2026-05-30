# Air-Piano
The Air Piano is an embedded musical device which users play by breaking a break-beam sensor at specified distances away from an ultrasonic distance sensor.

## Hardware Requirements

The hardware components needed to physically realize
the Air Piano are as follows:
- Beaglebone Black SBC
- HC-SR04 Ultrasonic Distance Sensor
- 8 LEDs
- IR Breakbeam Sensor
- Passive Buzzer

## Installation

### Installation Requirements

To build the Air Piano for the BBB, users
must use either the cross compiler
`arm-linux-gnueabihf-gcc` or the native BBB compiler.
To emulate the Air Piano on Linux, users require QEMU.

The BBB requires Linux with the PREEMPT_RT patch.

### Installation Steps
First, download the repository and navigate to the
its root directory:

```
git clone https://github.com/mtang-github/Air-Piano
cd Air-Piano
```

Then, build the desired version.

For QEMU (emulated, host compiler cross-compiled for ARM):
```
make emu
```
Output: build/air_piano_emu


For BeagleBone Black:
```
make bbb
```
Output: build/air_piano_bbb

For local host testing (emulated, host compiler):
```
make host
```
Output: build/air_piano_host

To clean build artifacts:
```
make clean
```

## Usage

Users may choose to run the code either through an
emulator or on the BBB.

### Emulated

On Linux, use the QEMU emulator to run the
cross-compiled executable:

```
qemu-arm -L /usr/arm-linux-gnueabihf build/air_piano_emu
```

Users may also likely be able to invoke the executable
directly, assuming Linux automatically calls QEMU:

```
./build/air_piano_emu
```

The program will prompt for pin assignments; users
may enter any valid, distinct set of GPIO numbers, as
QEMU does not use real hardware.

### On the Beaglebone Black

#### Hardware Wiring

We offer an example hardware wiring on the BBB.

HC-SR04 Ultrasonic Distance Sensor:
- VCC  -> P9_5  (5V)
- GND  -> P9_1  (GND)
- TRIG -> P9_23 (GPIO 49,  trig_out)
- ECHO -> P9_25 (GPIO 117, echo_in)

Note LEDs (anode through current-limiting resistor; all cathodes -> GND):
- C4 LED -> P8_7  (GPIO 66)
- D4 LED -> P8_8  (GPIO 67)
- E4 LED -> P8_9  (GPIO 69)
- F4 LED -> P8_10 (GPIO 68)
- G4 LED -> P8_11 (GPIO 45)
- A4 LED -> P8_12 (GPIO 44)
- B4 LED -> P8_13 (GPIO 23)
- C5 LED -> P8_14 (GPIO 26)

Beambreak Sensor:
- VCC -> P9_3  (3.3V)
- GND -> P9_2  (GND)
- OUT -> P9_12 (GPIO 60,  beam_in)

#### Physical Setup

Users must properly align the two ends of the breakbeam
sensor such that a gap of >60cm is created between
the two ends. The distance sensor should be pointed
in parallel to the beam of the break beam.

#### Running

Assuming the Air Piano executable is already on the
BBB, begin executing the program:

```
./air_piano_bbb
```

Respond to the startup prompts according to the way
the device was wired. For example:

```
Enter GPIO pins (trig_out C4_led_out D4_led_out E4_led_out F4_led_out G4_led_out A4_led_out B4_led_out C5_led_out beam_in echo_in):
49 66 67 69 68 45 44 23 26 60 117

Enter note duration (s):
0.1
```

The note duration may be adjusted to taste.

#### Operation

Normal operation:
- Move your hand downwards between the beambreak sensors to trigger a note.
- The buzzer will play the note and the LEDs will light up indicating the pitch of the note that was played.
- Lift hand up and down again to play a new note.
- If "WARNING: beam not aligned" appears, make sure the beambreak sensor is properly alligned and the warning will go away.

Safe-state (LEDs turn off and sound is not output):
- If more than 10 beam breaks are detected within one second, the safe state will be entered, as this indicates a possible sensor or logical malfunction.
- If the distance sensor value is clearly outside of the sensor's own range, the safe state will be entered.
- To exit the safe-state: type `c` and press Enter at the terminal.

Shutdown:
- Type `q` and press Enter to stop the program cleanly.

## Demo Links

https://youtu.be/6kKO3OEG_N4

https://youtube.com/shorts/gGBMoctYcf4?feature=share

## Collaborators

Credit to N. Bacon and A. Taylor.
