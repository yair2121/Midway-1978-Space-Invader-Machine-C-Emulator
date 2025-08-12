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

Custom implementations can be created by providing display, input, audio, and timing functions to the core emulator.

## Status

Playable - ongoing refinements and improvements.

## Requirements

- C compiler
- SDL3 (for included implementation)
- Space Invaders ROM files
- Audio files

---

*ROM and audio files not included due to copyright restrictions.*

