# AGENTS.md — Kinoshi-Tiny Key 25 firmware (C++ / PlatformIO)

AI coding agent guidance for this directory. This directory is the active development target for the Kinoshi-Tiny Key 25 C++ firmware.

## Scope and priority
- This directory takes precedence over the parent repository-level AGENTS.md.
- The parent file treats this directory as a MicroPython-port reference. This file overrides that guidance for active C++/PlatformIO firmware work.
- This project is also the reference implementation for the MicroPython port; therefore, changes to protocol behavior, pin mapping, and MIDI semantics must be made carefully and with awareness of the downstream port.

## Development environment (PlatformIO)
- PlatformIO Core 6.1.19 is installed as part of the VS Code extension bundle.
- The `pio` CLI is not on PATH by default. Use the bundled executable explicitly when running from the terminal:

  ```powershell
  $pio = "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe"
  & $pio run
  ```

- The `~/.platformio` directory already contains platforms and packages, so a fresh install is not required.
- VS Code GUI operations (build/upload/monitor) are also available through the PlatformIO extension.

## Common commands
Run all commands from this directory (`software/kinoshi-tiny-key-25/`):

| Operation | Command |
|---|---|
| Build | `pio run` |
| Clean | `pio run -t clean` |
| Upload | `pio run -t upload` |
| Serial monitor | `pio device monitor` |
| Static analysis | `pio check` |

Notes:
- `monitor_speed = 11520` is set in `platformio.ini`; do not change it to 115200 without an explicit reason.
- `pio check` runs clang-tidy with `--fix`; it may automatically rewrite source files. Review the diff afterward.
- To exit the serial monitor, use `Ctrl+T` then `Ctrl+X` in PlatformIO monitor.

## Project structure
| Path | Role |
|---|---|
| `platformio.ini` | Build config for `waveshare_rp2040_zero` / Arduino / `-DUSE_TINYUSB=1` |
| `src/main.cpp` | Entry point: `setup()` / `loop()` and repeating timer |
| `src/application.{h,cpp}` | Switch-to-MIDI mapping logic |
| `src/switch.hpp` | 3-chain active-low switch scanner |
| `src/leds.{h,cpp}` | Octave display LED logic |
| `src/midi_process.{h,cpp}` | Dual MIDI output: USB MIDI + Serial2 |
| `src/device_config.h` | USB descriptors, MIDI config, octave range, LED colors |
| `src/pins.h` | Pin assignments (source of truth for GPIO mapping) |
| `MIDI_Implementation.txt` | MIDI implementation chart |
| `.clang-format` | Formatting rules |
| `test/` | Empty stub; no project tests are present yet |

## Dependency libraries
- `adafruit/Adafruit TinyUSB Library@^3.2.0`
- `fastled/FastLED@^3.7.6`

## Hardware constraints
- Board: Waveshare RP2040 Zero
- Pins (from `src/pins.h`):

| Signal | GPIO | Purpose |
|---|---:|---|
| NPL | 15 | Shift-register latch |
| CLOCK | 14 | Shift-register clock |
| OUT1 | 27 | Chain 1 read |
| OUT2 | 26 | Chain 2 read |
| OUT3 | 28 | Chain 3 read |
| NeoPixel (Octave) | 29 | Octave LED |
| NeoPixel (Zero) | 16 | Reserved |

- Switches are active LOW. OFF = HIGH, ON = LOW.
- The scanner reads 3 chains in parallel over 16 clock cycles.
- Debounce requires two consecutive matching reads.
- All MIDI messages are sent to both USB MIDI and UART MIDI outputs.
- On initialization, if Modulation + OctMinus are both ON, enter UF2 boot mode (`reset_usb_boot` equivalent).

## Coding conventions
- Namespace: `kinoshita_lab::kinoshi_tiny_key_25::<module>`
- Constants use the `k` prefix: `kPin…`, `kSwitchId…`, `kNum…`
- Include guard style: `#pragma once` + `<NAME>_H` (e.g. `LEDS_H`)
- Formatting follows `.clang-format` (`Google`-based, 4-space indentation, `ColumnLimit: 0`)
- Keep changes small and incremental.
- Commit messages should be brief and English-oriented when possible; use an area prefix such as `[firmware]` and close issues with `Fixes #N` when appropriate.

## Working rules
- Follow the repository workflow: plan first, then implement, then validate.
- Do not make broad, unrelated changes in a single patch.
- Run `pio run` after making code changes and confirm the build succeeds before reporting completion.
- If hardware behavior is affected, ask the user before changing wiring or protocol behavior.
- Since there are no project tests, build success and static analysis are the minimum validation bar.

## Notes
- This file is intentionally local to this directory so that work here follows PlatformIO-based firmware development rather than the parent MicroPython-port instructions.
- If the repo later needs a different default toolchain for this folder, update this file explicitly and keep the intent clear.
