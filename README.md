# LC Tech ESP-01 Relay v2.3 Arduino Example

Minimal Arduino sketch for controlling the LC Technology ESP-01 relay v2.3 board from custom ESP8266 firmware.

## Overview

This repository demonstrates a small but important workaround for LC Technology ESP-01 relay boards that use a separate onboard relay MCU.

The ESP-01 does not drive the relay coil directly. Instead, the onboard MCU expects UART output that looks like Espressif ESP-AT firmware receiving TCP data. This sketch sends those UART messages directly, so the relay can be controlled from custom Arduino firmware without the vendor Android apps and without soldering.

## What This Solves

Some LC Technology ESP-01 relay boards do not wire the relay directly to an ESP8266 GPIO. On these boards, the relay is controlled by a separate onboard MCU. The ESP-01 normally runs Espressif AT firmware and acts as a Wi-Fi modem.

That means sketches like this usually do nothing:

```cpp
digitalWrite(0, HIGH);
digitalWrite(0, LOW);
```

The relay MCU expects to see AT-firmware-style UART messages from the ESP-01. This project sends those messages directly from an Arduino sketch.

## Hardware

This was tested on an LC Technology ESP-01 single relay board v2.3 style board with:

- ESP-01 / ESP-01S module
- onboard relay controller MCU
- TX/RX jumper caps
- S1 / S2 buttons
- relay command bytes documented as:
  - ON/open: `A0 01 01 A2`
  - OFF/close: `A0 01 00 A1`

The tested relay was marked `SRD-05VDC-SL-C`, so the relay board needed 5V power on `IN+` / `IN-`.

Other visually similar boards may use different relay voltage, jumpers, firmware modes, or command bytes.

## How It Works

The sketch does not use Wi-Fi.

Instead, it writes UART data at `115200` baud that looks like output from Espressif AT firmware:

```text
ready
WIFI CONNECTED
WIFI GOT IP
0,CONNECT
+IPD,4:<relay bytes>
+IPD,0,4:<relay bytes>
0,CLOSED
```

The relay command bytes are embedded inside the fake `+IPD` TCP receive messages.

Both `+IPD,<length>:` and `+IPD,<connection id>,<length>:` are sent because different board modes appear to expect different ESP-AT connection formats.

The actual relay command bytes are:

```text
A0 01 01 A2  relay on/open
A0 01 00 A1  relay off/close
```

## Board Setup

1. Power the relay board from the correct relay voltage.
   - `SRD-05VDC-SL-C`: use 5V.
   - `SRD-12VDC...`: use 12V.

2. Install the TX/RX jumper caps in the normal ESP-01 position.

3. Flash the ESP-01.

4. After flashing, remove GPIO0 from GND.

5. Reset or power-cycle the board.

## Arduino Setup

Use the Arduino IDE or any ESP8266 Arduino build workflow.

Useful setup links:

- [Download Arduino IDE](https://www.arduino.cc/en/software)
- [Install the ESP8266 Arduino core](https://arduino-esp8266.readthedocs.io/en/latest/installing.html)
- [ESP8266 Arduino core documentation](https://arduino-esp8266.readthedocs.io/)

When using Arduino IDE, install the ESP8266 core through Boards Manager, then select:

```text
Tools -> Board -> ESP8266 Boards -> Generic ESP8266 Module
```

Open `arduino.ino` and upload it to the ESP-01.

## Flashing Notes

To flash an ESP-01, GPIO0 must be connected to GND during reset/power-up. After flashing, disconnect GPIO0 from GND and reset or power-cycle the module so it boots normally.

## Expected Behavior

After boot:

- the ESP onboard LED blinks 3 times
- the relay turns on
- after a short delay, the relay turns off
- the cycle repeats

The sketch currently keeps the relay on for 2 seconds and off for 5 seconds.

## Troubleshooting

- This is not GPIO relay control.
- The ESP-01 is talking to the relay board MCU over UART.
- If the ESP blinks but the relay does not click, check power voltage and the TX/RX jumper caps first.
- If the board is in a different mode, press `S1` and try again. The included sketch sends enough AT-style state messages to work in both tested modes.
- If the ESP does not boot after flashing, make sure GPIO0 is no longer connected to GND.
