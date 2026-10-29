# Kinoshi-Tiny Key 25 MIDI Implementation

Version 1.1.0 (Oct. 29 2026)

See also [MIDI_Implementation_Chart.md](./MIDI_Implementation_Chart.md).

- [1. Transmitted Data](#1-transmitted-data)
- [2. Recognized Receive Data](#2-recognized-receive-data)
- [3. Setting Data Format](#3-setting-data-format)
- [4. Setting Sequence](#4-setting-sequence)

[H]: Hex, [D]: Decimal

## 1. Transmitted Data

### 1-1 Channel Messages

| Status [H] | Second [H] ([D]) | Third [H] ([D]) | Description | Transmitted by | Note |
|---|---|---|---|---|---|
| 8n | kk (kk) | 00 (00) | Note Off | Keyboard | *1 |
| 9n | kk (kk) | vv (vv) | Note On vv=1~127 | Keyboard | *1, *2 |
| Bn | cc (cc) | vv (vv) | Control Change | Mod Button | *3 |
| Bn | cc (cc) | vv (vv) | Control Change | Pitch+/- Button | *3, *4 |
| Bn | cc (cc) | vv (vv) | Control Change | Sustain Button | *3 |
| Bn | 79 (121) | 00 (00) | Reset All Controllers | TRANSFER START | *5 |
| Bn | 7B (123) | 00 (00) | All Notes Off | TRANSFER START | *5 |
| En | bb (bb) | bb (bb) | Pitch Bend Change | Pitch+/- Button | *4 |

- **n**: MIDI Channel = 0 ~ F
  - USB MIDI and TRS MIDI each transmit on their own channel (TABLE 1, MSB 40/41).
  - Every channel message is transmitted on both USB MIDI and TRS MIDI.
- **vv**: Value

**\*1**: kk = 00~7F : 0~127

- kk = (Octave + 1) * 12 + Key
  - Octave = -1~9 (selected by OCT-/OCT+ buttons, 4 at power on)
  - Key = 0~24 (0 = leftmost key)
- At power on the keyboard transmits kk = 3C~54 (60~84).
- Keys whose kk would exceed 7F (127) transmit nothing.
- Note Off is transmitted with the same kk as the Note On, even if the octave has been changed while the key is held.

**\*2**: vv = Keyboard Velocity (TABLE 1, MSB 48 LSB 01).

**\*3**: cc = CC Number (TABLE 1, LSB 12).

- Mode = Momentary: vv = On Value when pressed, Off Value when released.
- Mode = Toggle: vv = On Value and Off Value alternately on each press.
  - Nothing is transmitted on release.
  - The first press after power on transmits On Value.

**\*4**:

- Function = Pitch Bend: Pitch Bend Change is transmitted.
  - While pressed, the value changes gradually and is transmitted repeatedly. It takes about 250 ms from center to the end.
  - Pitch+: toward bb,bb = 7F,7F (+8191), then holds.
  - Pitch-: toward bb,bb = 00,00 (-8192), then holds.
  - When released, bb,bb = 00,40 (0) is transmitted immediately.
  - If both buttons are pressed, the last pressed one is effective. Releasing either one returns the value to center.
- Function = CC: Control Change is transmitted (*3).

**\*5**: Transmitted when TRANSFER START (TABLE 2) is received, in the order All Notes Off, Reset All Controllers. They are transmitted on the channels in effect before the transfer.
At the same time the device forgets held keys (no Note Off is transmitted when they are released), resets toggle states and stops Pitch Bend at center. A held key or button takes effect again when pressed again.

Each time a parameter of TABLE 1 is received (2-2), even if its value is unchanged, the following happens:

- Toggle states of all buttons are reset without transmitting anything (the next press transmits On Value).
- If Pitch Bend is not at center or a button is bending, bb,bb = 00,40 (0) is transmitted. The bend stays at center until the button is pressed again.

### 1-2 Setting Data (NRPN)

| Status [H] | Second [H] ([D]) | Third [H] ([D]) | Description | Transmitted by | Note |
|---|---|---|---|---|---|
| B0 | 63 (99) | mm (mm) | NRPN MSB | Dump | *6 |
| B0 | 62 (98) | ll (ll) | NRPN LSB | Dump | *6 |
| B0 | 06 (06) | vv (vv) | Data Entry MSB | Dump | *6 |
| B0 | 65 (101) | 7F (127) | RPN MSB = Null | Dump | *6 |
| B0 | 64 (100) | 7F (127) | RPN LSB = Null | Dump | *6 |

- Setting data is transmitted on USB MIDI only (not on TRS MIDI).
- Setting data is always transmitted on MIDI Channel 1 (B0), regardless of the USB MIDI / TRS MIDI channel settings.

**\*6**:

- mm,ll = Parameter (TABLE 1, TABLE 2, TABLE 3)
- vv = Value (TABLE 1, TABLE 2, TABLE 3)
- Transmitted when DUMP REQUEST is received. (2-2, SEQUENCE 2)
- Firmware version (TABLE 3) is transmitted when VERSION REQUEST is received. (SEQUENCE 3)
- ACK is transmitted when SAVE has been completed. (SEQUENCE 1)

## 2. Recognized Receive Data

- Received on USB MIDI only. TRS MIDI has no receive function.
- Received messages are not echoed (no MIDI Thru).

### 2-1 Channel Messages

Not applicable (performance messages are not recognized).

### 2-2 Setting Data (NRPN)

| Status [H] | Second [H] ([D]) | Third [H] ([D]) | Description | Note |
|---|---|---|---|---|
| B0 | 63 (99) | mm (mm) | NRPN MSB | *7 |
| B0 | 62 (98) | ll (ll) | NRPN LSB | *7 |
| B0 | 06 (06) | vv (vv) | Data Entry MSB | *7 |
| B0 | 65 (101) | 7F (127) | RPN MSB = Null | *8 |
| B0 | 64 (100) | 7F (127) | RPN LSB = Null | *8 |

Recognized on MIDI Channel 1 (B0) only. NRPN on other channels is ignored.

**\*7**:

- mm,ll = Parameter (TABLE 1, TABLE 2)
- vv = Value (TABLE 1, TABLE 2)
- The value is applied when Data Entry MSB is received.
- Data Entry LSB (26H), Data Increment (60H) and Data Decrement (61H) are not recognized.
- Undefined parameters and out-of-range values are ignored. No error is transmitted.
- Applied values take effect immediately but are not saved until SAVE is received.

**\*8**: May be omitted.
When RPN MSB or RPN LSB is received (with any value), the NRPN selection is cleared and following Data Entry MSB is ignored until NRPN MSB and LSB are received again.

## 3. Setting Data Format

One parameter is transferred as the following sequence.

| Byte [H] | Description |
|---|---|
| B0,63,mm | NRPN MSB, mm = Target (TABLE 1) |
| B0,62,ll | NRPN LSB, ll = Item (TABLE 1) |
| B0,06,vv | Data Entry MSB, vv = Value (TABLE 1) |
| B0,65,7F | RPN MSB = Null |
| B0,64,7F | RPN LSB = Null |

Example: USB MIDI Channel = 10

```
B0,63,40, B0,62,00, B0,06,09, B0,65,7F, B0,64,7F
```

### TABLE 1: Setting Parameter

| MSB [H] | LSB [H] | Parameter | Value / Description | Default |
|---|---|---|---|---|
| 40 | 00 | USB MIDI MIDI Channel | 00~0F = 1~16 | 00 |
| 41 | 00 | TRS MIDI MIDI Channel | 00~0F = 1~16 | 00 |
| 48 | 01 | Keyboard Velocity | 01~7F = 1~127 | 64 |
| 50 | 11 | Mod Button Mode | 00,01 = Momentary,Toggle | 00 |
| 50 | 12 | Mod Button CC Number | 00~7F = 0~127 | 01 |
| 50 | 13 | Mod Button On Value | 00~7F = 0~127 | 7F |
| 50 | 14 | Mod Button Off Value | 00~7F = 0~127 | 00 |
| 51 | 10 | Pitch+ Button Function | 00,01 = Pitch Bend,CC | 00 |
| 51 | 11 | Pitch+ Button Mode | 00,01 = Momentary,Toggle | 00 |
| 51 | 12 | Pitch+ Button CC Number | 00~7F = 0~127 | 00 |
| 51 | 13 | Pitch+ Button On Value | 00~7F = 0~127 | 7F |
| 51 | 14 | Pitch+ Button Off Value | 00~7F = 0~127 | 00 |
| 52 | 10 | Pitch- Button Function | 00,01 = Pitch Bend,CC | 00 |
| 52 | 11 | Pitch- Button Mode | 00,01 = Momentary,Toggle | 00 |
| 52 | 12 | Pitch- Button CC Number | 00~7F = 0~127 | 00 |
| 52 | 13 | Pitch- Button On Value | 00~7F = 0~127 | 7F |
| 52 | 14 | Pitch- Button Off Value | 00~7F = 0~127 | 00 |
| 53 | 11 | Sustain Button Mode | 00,01 = Momentary,Toggle | 00 |
| 53 | 12 | Sustain Button CC Number | 00~7F = 0~127 | 40 |
| 53 | 13 | Sustain Button On Value | 00~7F = 0~127 | 7F |
| 53 | 14 | Sustain Button Off Value | 00~7F = 0~127 | 00 |

- Total 21 parameters. Dump is transmitted in the order of this table.
- Mod and Sustain buttons always work as CC (no Function parameter).
- While Function = Pitch Bend, LSB 11~14 are kept and restored when Function = CC.
- Settings are restored to DEFAULT and saved by holding OCT- and OCT+ at power on (factory reset), or when the saved settings are found to be invalid at power on.

### TABLE 2: System Command

| MSB [H] | LSB [H] | Command | R/T | Value / Description |
|---|---|---|---|---|
| 60 | 00 | DUMP REQUEST | R | 00: Transmits all parameters (TABLE 1) and DUMP COMPLETE |
| 60 | 01 | DUMP COMPLETE | T | 00: End of dump |
| 60 | 02 | SAVE | R | 00: Writes current settings to internal memory |
| 60 | 03 | ACK | T | 00: SAVE has been completed |
| 60 | 04 | TRANSFER START | R | 00: Editor starts transmitting settings (SEQUENCE 1). Transmits All Notes Off and Reset All Controllers (1-1) |
| 60 | 05 | VERSION REQUEST | R | 00: Transmits firmware version (TABLE 3) |

- R: Received, T: Transmitted
- Commands with a value other than 00 are ignored.
- Parameters of TABLE 1 are accepted at any time, with or without TRANSFER START.

### TABLE 3: Device Information

| MSB [H] | LSB [H] | Parameter | R/T | Value / Description |
|---|---|---|---|---|
| 61 | 00 | FIRMWARE VERSION MAJOR | T | 00~7F = 0~127 |
| 61 | 01 | FIRMWARE VERSION MINOR | T | 00~7F = 0~127 |
| 61 | 02 | FIRMWARE VERSION PATCH | T | 00~7F = 0~127 |

- The firmware version follows Semantic Versioning (MAJOR.MINOR.PATCH).
- Transmitted in the order of this table. Received values are ignored.

Example: Version 1.1.0

```
B0,63,61, B0,62,00, B0,06,01, B0,65,7F, B0,64,7F,
B0,63,61, B0,62,01, B0,06,01, B0,65,7F, B0,64,7F,
B0,63,61, B0,62,02, B0,06,00, B0,65,7F, B0,64,7F
```

## 4. Setting Sequence

### SEQUENCE 1: Send settings to the device (Editor -> Device)

1. Editor transmits TRANSFER START (60,04,00).
   The device transmits All Notes Off and Reset All Controllers on USB MIDI and TRS MIDI (1-1, *5).
2. Editor transmits all parameters of TABLE 1 (21 parameters).
   The device applies each value to the current settings (not saved yet).
3. Editor transmits SAVE (60,02,00).
   The device writes the current settings to internal memory.
4. After writing is completed, the device transmits ACK (60,03,00).
   If writing fails, ACK is not transmitted.
5. When ACK is received, the editor reports that the transfer succeeded.
   If ACK is not received (timeout), the editor shows an error.

### SEQUENCE 2: Receive settings from the device (Device -> Editor)

1. Editor transmits DUMP REQUEST (60,00,00).
2. The device transmits all parameters of TABLE 1 (21 parameters) in table order.
3. The device transmits DUMP COMPLETE (60,01,00).
4. When DUMP COMPLETE is received and all 21 parameters are received, the editor applies them.
   Otherwise (or on timeout) the editor shows an error and keeps the current settings.

### SEQUENCE 3: Get the firmware version (Device -> Editor)

1. Editor transmits VERSION REQUEST (60,05,00).
2. The device transmits FIRMWARE VERSION MAJOR, MINOR and PATCH (TABLE 3) in table order.
3. When all three values are received, the editor shows the version.
   Otherwise (on timeout) the editor shows an error.
