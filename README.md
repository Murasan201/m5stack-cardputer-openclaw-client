# M5Stack Cardputer OpenClaw Client

An interactive prompt client running on the [M5Stack Cardputer](https://docs.m5stack.com/en/core/Cardputer) (ESP32-S3) that communicates with an [OpenClaw](https://openclaw.dev/) AI agent gateway over Wi-Fi.

## Features

- Romaji-to-hiragana conversion engine (full kana coverage including voiced, combo, double consonants)
- Japanese / ASCII input mode toggle
- LCD display for dialogue UI (idle / input / sending / response / error states)
- HTTP communication with OpenClaw via HTTP bridge server
- Wi-Fi connection status display on startup
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
  src/config.h.example        Wi-Fi / OpenClaw connection settings template
  src/main.cpp                Entry point
  src/dialogue_manager.*      Session state machine and keyboard handling
  src/prompt_input.*          Romaji-to-hiragana conversion engine
  src/display_manager.*       LCD rendering
  src/network_client.*        Wi-Fi + HTTP client
docs/
  system-spec.md              System specification
  feature-romaji-input.md     Romaji input feature spec
  feature-http-bridge.md      HTTP bridge feature spec
  toolchain-spec.md           Toolchain and build documentation
  requirements.md             Requirements specification
  troubleshooting.md          Troubleshooting guide
tests/                        Python unit tests (state machine, communication)
```

## Quick Start

### Prerequisites

- Raspberry Pi (or any Linux host) with Python 3 and USB connection to Cardputer
- PlatformIO Core CLI
- OpenClaw gateway running on the Raspberry Pi
- [HTTP bridge server](docs/feature-http-bridge.md) running on the Raspberry Pi

### Build & Flash

```bash
# Set up environment
python3 -m venv .venv
source .venv/bin/activate
pip install platformio
export PLATFORMIO_CORE_DIR=/mnt/ssd/.platformio  # optional: use SSD storage

# Configure
cp software/cardputer-client/src/config.h.example software/cardputer-client/src/config.h
# Edit config.h with your Wi-Fi credentials and Raspberry Pi IP

# Build
cd software/cardputer-client
pio run --environment m5stack-cardputer

# Flash (replace /dev/ttyACM1 with your actual port)
pio run --target upload --environment m5stack-cardputer --upload-port /dev/ttyACM1
```

See [docs/toolchain-spec.md](docs/toolchain-spec.md) for detailed toolchain documentation and troubleshooting.

## Usage

1. Press **any key** on the idle screen to start input mode
2. Type your prompt using the 56-key keyboard
   - **Tab** — Toggle between Japanese `[あ]` and ASCII `[A]` input mode
   - In Japanese mode, type romaji (e.g. `konnichiha`) to get hiragana (`こんにちは`)
   - **Del** — Delete last character
3. **Enter** — Send prompt to OpenClaw and display the AI response
4. **Esc** (Fn + `` ` ``) — Return to idle screen from any state
5. On the response/error screen, press any key to start a new prompt

## License

See [LICENSE](LICENSE) for details.
