# M0ZJO Turnout Controller

An Arduino sketch that controls model railway turnouts (points) via serial commands. Each turnout is thrown by sending a short pulse to one of two output pins — one for **Normal** and one for **Reverse** — making it compatible with any twin-coil solenoid point motor (e.g. SEEP, Peco PL-10, Hornby, etc.).

---

## Table of Contents

1. [Features](#features)
2. [Hardware Requirements](#hardware-requirements)
3. [Wiring](#wiring)
4. [Installation](#installation)
5. [Configuration](#configuration)
6. [Serial Command Protocol](#serial-command-protocol)
7. [Response Messages](#response-messages)
8. [Error Handling](#error-handling)
9. [Adding More Turnouts](#adding-more-turnouts)
10. [Troubleshooting](#troubleshooting)
11. [Technical Reference](#technical-reference)

---

## Features

- Controls any number of turnouts (limited only by available Arduino output pins)
- Active-LOW output — pins idle HIGH and are pulsed LOW to fire the solenoid
- Configurable 40 ms pulse duration (safe default for most twin-coil motors)
- Framed serial protocol (`< >`) is robust against partial or garbled input
- Full input validation with descriptive error messages
- Serial confirmation on every successful command
- Flash-memory string storage (`F()` macro) to minimise RAM usage

---

## Hardware Requirements

| Component | Notes |
|---|---|
| Arduino (Uno, Nano, Mega, etc.) | Any board with sufficient digital output pins |
| Twin-coil solenoid point motor(s) | e.g. Tested with PECO PL-11 |
| Relay driver board | One relay/channel per coil (2 per turnout) |
| 12–16 V DC supply | For the solenoid coils; check your motor's datasheet |
| USB cable | For serial communication and programming |

> **Important:** Arduino output pins cannot drive solenoid coils directly. You must use a relay module, ULN2003 Darlington array, or similar driver between the Arduino pin and the coil.

---

## Wiring

Each turnout requires **two** Arduino output pins, one for each coil direction.

### Default Pin Assignment (Turnout 00)

```
Arduino Pin 8  →  Driver IN  →  Solenoid Coil A (Normal)
Arduino Pin 9  →  Driver IN  →  Solenoid Coil B (Reverse)
```

### Pin Behaviour

| State | Pin voltage |
|---|---|
| Idle (no command) | HIGH (5 V) |
| During pulse | LOW (0 V) for 40 ms |
| After pulse | HIGH (5 V) |

### Example Relay Wiring (per coil)

TBC...

---

## Installation

1. Download or clone this repository.
2. Open `turnoutControl.ino` in the [Arduino IDE](https://www.arduino.cc/en/software).
3. Select your board under **Tools → Board**.
4. Select the correct port under **Tools → Port**.
5. Click **Upload**.
6. Open **Tools → Serial Monitor**, set the baud rate to **9600**, and confirm you see:

```
### M0ZJO Turnout controller ready! ###
Send <##R> or <##N>  e.g. <00N> <00R>
```

---

## Configuration

All user-adjustable settings are near the top of the sketch.

### Pulse Duration

```cpp
const uint16_t PULSE_MS = 40;   // pulse duration in milliseconds
```

40 ms is the recommended safe minimum for most twin-coil motors. Increase if your motor fails to throw reliably; do **not** increase beyond ~100 ms to avoid overheating the coil.

### Turnout Pin Table

```cpp
const Turnout TURNOUTS[] = {
  {8, 9},    // turnout 00 — Normal pin, Reverse pin
};
```

Each row is one turnout. The index of the row (0, 1, 2 …) corresponds to the two-digit number used in serial commands (`00`, `01`, `02` …).

### Serial Baud Rate

```cpp
Serial.begin(9600);
```

Change to match your host software if needed. Common values: `9600`, `19200`, `57600`, `115200`.

---

## Serial Command Protocol

Commands are ASCII strings wrapped in angle brackets:

```
<NNX>
```

| Field | Length | Description |
|---|---|---|
| `<` | 1 char | Start-of-command marker |
| `NN` | 2 chars | Turnout number, zero-padded (`00`–`99`) |
| `X` | 1 char | Direction: `N` = Normal, `R` = Reverse |
| `>` | 1 char | End-of-command marker |

### Examples

| Command | Action |
|---|---|
| `<00N>` | Throw turnout 0 to **Normal** (pulse pin 8 LOW for 40 ms) |
| `<00R>` | Throw turnout 0 to **Reverse** (pulse pin 9 LOW for 40 ms) |
| `<01N>` | Throw turnout 1 to **Normal** (once added to the table) |
| `<01R>` | Throw turnout 1 to **Reverse** (once added to the table) |

Commands can be sent from any serial terminal (Arduino IDE Serial Monitor, PuTTY, etc.) or from automation software such as JMRI, Rocrail, or a custom script.

> **Tip:** The `<` `>` framing means stray characters or line endings in the serial stream are safely ignored. You can type commands with or without a newline.

---

## Response Messages

Every command produces a one-line response.

### Success

```
OK: turnout 0 -> Normal
OK: turnout 0 -> Reverse
```

### Errors

```
ERR: bad command length
ERR: command too long
ERR: turnout number must be two digits
ERR: unknown turnout 5
ERR: direction must be N or R
```

---

## Error Handling

| Error message | Cause | Fix |
|---|---|---|
| `ERR: bad command length` | Fewer than 3 characters between `<` and `>` | Ensure command is exactly `<NNX>` |
| `ERR: command too long` | More than 3 characters before `>` | Check for extra characters in the command string |
| `ERR: turnout number must be two digits` | Non-numeric characters in the `NN` field | Use digits only, e.g. `00`, `01` |
| `ERR: unknown turnout N` | Turnout number not present in the `TURNOUTS[]` table | Add the turnout to the table and re-upload |
| `ERR: direction must be N or R` | Third character is not `N` or `R` | Use uppercase `N` or `R` only |

---

## Adding More Turnouts

1. Decide which Arduino pins to use (two per turnout).
2. Wire the new motor driver to those pins.
3. Add a row to the `TURNOUTS[]` array in the sketch:

```cpp
const Turnout TURNOUTS[] = {
  { 8,  9},   // turnout 00
  {10, 11},   // turnout 01  ← new
  {12, 13},   // turnout 02  ← new
};
```

4. Re-upload the sketch. No other changes are required — `NUM_TURNOUTS` is calculated automatically and the new turnouts are immediately addressable as `<01N>`, `<01R>`, `<02N>`, `<02R>`, etc.

> **Arduino Uno/Nano pin budget:** Pins 0 and 1 are used by Serial (TX/RX) and should be avoided. Pins 2–13 are available, giving a maximum of **6 turnouts** (12 pins / 2) on a Uno or Nano. An Arduino Mega expands this to 54 digital pins, supporting up to **26 turnouts**.

---

## Troubleshooting

**Motor doesn't throw / only throws sometimes**
- Increase `PULSE_MS` in small steps
- Check your 12 V supply can deliver enough current (twin-coil motors can draw 1–3 A briefly).
- Verify relay wiring — check the relay LED/indicator activates when a command is sent.

**`ERR: unknown turnout` even after editing the table**
- Make sure you re-uploaded the sketch after editing.
- Confirm the row index matches the command number (first row = `00`, second = `01`, etc.).

**No response in Serial Monitor**
- Confirm the baud rate in Serial Monitor matches `Serial.begin()` in the sketch (default: 9600).
- Check the correct COM/serial port is selected.
- Try pressing the Arduino reset button, then re-open the Serial Monitor.

---

## Technical Reference

### Struct Definition

```cpp
struct Turnout {
  uint8_t normalPin;    // Arduino pin pulsed for Normal  (N)
  uint8_t reversePin;   // Arduino pin pulsed for Reverse (R)
};
```

### Key Constants

| Constant | Type | Default | Description |
|---|---|---|---|
| `PULSE_MS` | `uint16_t` | `40` | Solenoid pulse duration (ms) |
| `CMD_LEN` | `uint8_t` | `3` | Expected character count inside `< >` |
| `NUM_TURNOUTS` | `uint8_t` | auto | Calculated from `TURNOUTS[]` array size |

### Serial Parser State Machine

```
Waiting for '<'
      │
      ▼  c == '<'
   In Command ──── accumulate chars until cmdIndex == CMD_LEN
      │
      ▼  c == '>'
  Validate length
      │
      ├─ cmdIndex == CMD_LEN  →  processCommand()
      └─ cmdIndex != CMD_LEN  →  ERR: bad command length
```

### `processCommand()` Validation Order

1. Characters 0 and 1 are digits.
2. Turnout ID is within the bounds of `TURNOUTS[]`.
3. Character 2 is `N` or `R`.
4. If all checks pass — pulse the pin, print `OK`.
