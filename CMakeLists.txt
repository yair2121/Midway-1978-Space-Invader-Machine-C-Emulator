cmake_minimum_required(VERSION 3.20)
project(Midway_1978_Space_Invader_Machine_C_Emulator)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 14)

if(APPLE) # Add Homebrew directories to the prefix path for Apple Silicon/Intel
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew" "/usr/local")
endif()

find_package(SDL3 REQUIRED)

include_directories(include)
include_directories(include/CPU8080)
include_directories(include/SDL3_plugin)
include_directories(include/SpaceInvaders_machine)

add_executable(Midway_1978_Space_Invader_Machine_C_Emulator
        include/CPU8080/Disassembler8080.h
        include/CPU8080/Emulate8080.h
        include/CPU8080/Opcodes8080.h
        include/SDL3_plugin/SDL_display.h
        include/SDL3_plugin/SDL_input.h
        include/SDL3_plugin/SDL_platform.h
        include/SDL3_plugin/SDL_sound.h
        include/SpaceInvaders_machine/EmulateMWSpaceInvaders.h
        include/SpaceInvaders_machine/MWDisplay.h
        include/SpaceInvaders_machine/MWInput.h
        include/SpaceInvaders_machine/MWSound.h
        include/SpaceInvaders_machine/SpaceInvaderMachine.h
        include/SpaceInvaders_machine/SpaceInvaderPlatformInterface.h
        include/resource.h
        src/CPU8080/Disassembler8080.c
        src/CPU8080/Emulate8080.c
        src/CPU8080/Opcodes8080.c
        src/SDL3_plugin/main_SDL.c
        src/SDL3_plugin/SDL_display.c
        src/SDL3_plugin/SDL_input.c
        src/SDL3_plugin/SDL_platform.c
        src/SDL3_plugin/SDL_sound.c
        src/SpaceInvaders_machine/EmulateMWSpaceInvaders.c
        src/SpaceInvaders_machine/MWDisplay.c
        src/SpaceInvaders_machine/MWInput.c
        src/SpaceInvaders_machine/MWSound.c
        src/SpaceInvaders_machine/SpaceInvaderMachine.c)

target_link_libraries(Midway_1978_Space_Invader_Machine_C_Emulator PRIVATE SDL3::SDL3)
