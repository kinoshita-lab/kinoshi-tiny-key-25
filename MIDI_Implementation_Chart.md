# Kinoshi-Tiny Key 25 MIDI Implementation Chart

Model: Kinoshi-Tiny Key 25 / Version 1.1.0 (Oct. 29 2026)

For details, see [MIDI_Implementation.md](./MIDI_Implementation.md).

| Function | | Transmitted | Recognized | Remarks |
|---|---|---|---|---|
| **Basic Channel** | Default | 1 | 1 | *1, *2 |
| | Changed | 1-16 | X | |
| **Mode** | Default | Mode 3 | X | |
| | Messages | X | X | |
| | Altered | ************** | | |
| **Note Number** | | 0-127 | X | *3 |
| | True Voice | ************** | X | |
| **Velocity** | Note On | O 9n v=1-127 | X | *4 |
| | Note Off | X 8n v=0 | X | |
| **After Touch** | Key's | X | X | |
| | Ch's | X | X | |
| **Pitch Bend** | | O | X | *5 |
| **Control Change** | 0-127 | O | X | Mod / Pitch+ / Pitch- / Sustain buttons *6 |
| | 1 | O | X | Modulation (Mod button default) |
| | 64 | O | X | Hold 1 (Sustain button default) |
| | 6 | O | O | Data Entry MSB *2 |
| | 98, 99 | O | O | NRPN LSB, MSB *2 |
| | 100, 101 | O | O | RPN LSB, MSB (Null only) *2 |
| **Program Change** | | X | X | |
| | True # | ************** | | |
| **System Exclusive** | | X | X | |
| **System Common** | Song Pos | X | X | |
| | Song Sel | X | X | |
| | Tune | X | X | |
| **System Real Time** | Clock | X | X | |
| | Commands | X | X | |
| **Aux Messages** | All Sound Off | X | X | |
| | Reset All Controllers | O | X | *7 |
| | Local On/Off | X | X | |
| | All Notes Off | O | X | *7 |
| | Active Sensing | X | X | |
| | System Reset | X | X | |

## Notes

- **\*1**: Channel messages are transmitted on both USB MIDI and TRS MIDI. Each has its own channel (default 1, changeable to 1-16 with the settings editor).
- **\*2**: Setting data (NRPN) only. It is transmitted and recognized on USB MIDI, channel 1 only, regardless of the channel settings. TRS MIDI has no receive function. Performance messages are not recognized.
- **\*3**: Note number = (Octave + 1) * 12 + Key. Octave is -1 to 9 (4 at power on), so the keyboard transmits 60-84 at power on. Keys above 127 transmit nothing.
- **\*4**: Velocity is fixed to the setting value (1-127, default 100).
- **\*5**: Pitch+ / Pitch- buttons with Function = Pitch Bend. The value moves from center to the end in about 250 ms and returns to center on release.
- **\*6**: CC number, On value, Off value and mode (Momentary / Toggle) can be set for each button. Pitch+ / Pitch- buttons transmit CC when Function = CC.
- **\*7**: Transmitted on USB MIDI and TRS MIDI when the device receives TRANSFER START from the settings editor.

---

| | |
|---|---|
| Mode 1: OMNI ON, POLY | Mode 2: OMNI ON, MONO |
| Mode 3: OMNI OFF, POLY | Mode 4: OMNI OFF, MONO |

O: Yes, X: No
