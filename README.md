# M5Stack Cardputer OpenClaw Client

An interactive prompt client running on the [M5Stack Cardputer](https://docs.m5stack.com/en/core/Cardputer) (ESP32-S3) that communicates with an [OpenClaw](https://openclaw.dev/) AI agent gateway over Wi-Fi.

## Features

- Japanese text input via on-screen candidate selector
- LCD display for dialogue UI (idle / input / response / error states)
- HTTP communication with the OpenClaw gateway
- Built with PlatformIO + Arduino framework

## Hardware

| Component | Detail |
|---|---|
| Board | M5Stack Cardputer |
| SoC | ESP32-S3FN8 (Xtensa LX7 dual-core, 240 MHz) |
| Display | 1.14" TFT ST7789V2 (240x135) |
| Input | 56-key keyboard |
| Connectivity | 2.4 GHz Wi-Fi, USB-C |

## Project Structure

```
software/cardputer-client/   PlatformIO project (firmware source)
  src/config.h                Wi-Fi / OpenClaw connection settings
  src/main.cpp                Entry point
  src/dialogue_manager.*      Session state machine and button handling
  src/prompt_input.*          Japanese character candidate input
  src/display_manager.*       LCD rendering
  src/network_client.*        Wi-Fi + HTTP client
docs/                         Requirements, setup guide, toolchain spec
tests/                        Python unit tests (state machine, communication)
```

## Quick Start

### Prerequisites

- Raspberry Pi (or any Linux host) with Python 3 and USB connection to Cardputer
- PlatformIO Core CLI

### Build & Flash

```bash
# Set up environment
python3 -m venv .venv
source .venv/bin/activate
pip install platformio
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio  # optional: use SSD storage

# Configure
# Edit software/cardputer-client/src/config.h with your Wi-Fi and OpenClaw settings

# Build
cd software/cardputer-client
pio run --environment m5stack-cardputer

# Flash (replace /dev/ttyACM1 with your actual port)
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
```

See [docs/toolchain-spec.md](docs/toolchain-spec.md) for detailed toolchain documentation and troubleshooting.

## Usage

1. **A button** — Start dialogue mode (shows candidate selector and input area)
2. **A button** (in dialogue) — Cycle through Japanese character candidates
3. **B button** — Commit selected character to input buffer
4. **A + B** — Backspace (delete last character)
5. **C button** — Send prompt to OpenClaw and display response

## License

See [LICENSE](LICENSE) for details.
