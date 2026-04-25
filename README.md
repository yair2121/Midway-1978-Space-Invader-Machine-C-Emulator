# Midway 1978 Space Invaders Arcade Machine Emulator (C)

## About

This project emulates the original Space Invaders hardware, including the Intel 8080 processor and arcade-specific hardware components. The emulator is designed with a modular architecture that separates the core emulation from platform-specific implementations.

## Features

- Intel 8080 CPU emulation
- Platform-independent core that can be integrated into any project
- SDL3 reference implementation with audio and video scaling support
- Accurate reproduction of original arcade behavior

## Architecture
The emulator consists of two main components:
- **Core Emulator**: Platform-agnostic emulation of the 8080 CPU and Space Invaders hardware
- **SDL Implementation**: Complete implementation using SDL3 for graphics, input, and audio

Custom platform integrations (e.g., Android, iOS) can be implemented by providing display, input, audio, and timing functions to the core emulator.

## Requirements

- **C Compiler**: C11-compliant
- **C++ Compiler**: C++14-compliant (required for the test suite)
- **SDL3**
  - *macOS*: `brew install sdl3`
- **Space Invaders ROM**: 8KB ROM file
- **Audio Files**: WAV files for sound effects (Optional)

---
## Running the Emulator

> [!IMPORTANT]
> **This project DOES NOT include any ROM or audio files due to copyright restrictions. You must provide your own legally obtained files.**

To run the emulator, you need to provide the ROM path and a directory containing the sound effects.

### 1. Game ROM
The emulator requires a single merged binary of the Space Invaders ROM (8KB).

### 2. Sound Effects
Place the following `.wav` files in a directory (e.g., `sounds/`):

| Filename | Sound Effect |
| :--- | :--- |
| `ufo.wav` | Persistent UFO sound |
| `shot.wav` | Player shot |
| `player_die.wav` | Player explosion |
| `invader_die.wav` | Alien explosion |
| `fleet_1.wav` - `fleet_4.wav` | Alien movement (Pitche 1-4) |
| `ufo_hit.wav` | UFO explosion |

**Note:** The emulator will run even if some sounds are missing (it will simply be silent for those effects).

### 3. Execution Command

```bash
# Usage: <ROM_PATH> <SOUND_DIRECTORY>
./space-invaders path/to/invaders.rom path/to/sounds/
```
