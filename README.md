# Tower Defense Game

A cross-platform tower defense game developed in C++ using modern game development libraries and tools.

## Overview

This project is a feature-rich tower defense game built with a modern C++ stack, featuring entity-component-system architecture, cross-platform compatibility, and professional game development practices.

## Features

- **Cross-platform compatibility** - Runs on Windows, Linux, and macOS
- **Entity-Component-System (ECS)** architecture using EnTT
- **Modern graphics** with SDL3 and OpenGL
- **Audio system** with SDL3_mixer
- **Map design** using Tiled editor
- **Real-time debugging** with ImGui
- **Comprehensive logging** with spdlog
- **JSON configuration** with nlohmann-json
- **Mathematical operations** with GLM

## Technology Stack

### Core Libraries
- **EnTT** - Entity-Component-System framework
- **SDL3** - Cross-platform multimedia library (display, input, windowing)
- **GLM** - Mathematics library for 3D graphics
- **ImGui** - Immediate mode GUI for debugging
- **nlohmann-json** - JSON parsing and serialization

### SDL Extensions
- **SDL3_mixer** - Audio playback and mixing
- **SDL3_image** - Image loading and manipulation
- **SDL3_ttf** - TrueType font rendering

### Development Tools
- **spdlog** - Fast logging library
- **Tiled** - Map editor for level design

### Build System
- **CMake** - Cross-platform build configuration
- **Ninja** - Fast build system
- **clang** - C++ compiler
- **git** - Version control

## Project Structure

The project follows modern C++ game development practices with clean separation of concerns through the ECS architecture.

## Building the Project

### Prerequisites
- CMake 3.20+
- Ninja build system
- clang compiler
- SDL3 development libraries
- Other dependencies as specified in CMakeLists.txt

### Build Commands
```bash
cmake --preset build-clang-ninja
cmake --build --preset build-TDPreset
```

## Usage

After building, run the executable to start the tower defense game. Use the Tiled editor to create and modify game levels.
```bash
.build-clang-ninja/TD-Linux # if on Linux
.build-clang-ninja/TD-Windows.exe # if on Windows
.build-clang-ninja/TD-MacOS # if on macOS
```