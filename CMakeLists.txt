cmake_minimum_required(VERSION 3.0)
project(game)
set(CMAKE_C_STANDARD 11)
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

#=== LIBRARY: sokol
# add headers to the the file list because they are useful to have in IDEs
set(SOKOL_HEADERS
    sokol/sokol_gfx.h
    sokol/sokol_app.h
    sokol/sokol_audio.h
    sokol/sokol_time.h
    sokol/sokol_glue.h)
add_library(sokol STATIC sokol/sokol.c ${SOKOL_HEADERS})
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    # compile sokol.c as Objective-C
    target_compile_options(sokol PRIVATE -x objective-c)
    target_link_libraries(sokol
        "-framework QuartzCore"
        "-framework Cocoa"
        "-framework MetalKit"
        "-framework Metal"
        "-framework AudioToolbox")
else()
    if (CMAKE_SYSTEM_NAME STREQUAL Linux)
        target_link_libraries(sokol INTERFACE X11 Xi Xcursor GL asound dl m)
        target_link_libraries(sokol PUBLIC Threads::Threads)
    endif()
endif()
target_include_directories(sokol INTERFACE sokol)

#=== EXECUTABLE: game

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(COMPILE_SHADER_CMD ../sokol-tools-bin/bin/win32/sokol-shdc.exe --input ../shaders.glsl --output ../shaders.glsl.h --slang hlsl5)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(COMPILE_SHADER_CMD ../sokol-tools-bin/bin/osx/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang metal_macos)
elseif(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(COMPILE_SHADER_CMD ../sokol-tools-bin/bin/linux/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang glsl100)
else()
    set(COMPILE_SHADER_CMD ../sokol-tools-bin/bin/linux/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang glsl330)
endif()
add_custom_command(OUTPUT ../shaders.glsl.h
    COMMAND ${COMPILE_SHADER_CMD}
    DEPENDS shaders.glsl
)
set_source_files_properties(../shaders.glsl.h PROPERTIES
    GENERATED TRUE
)

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_executable(game WIN32 main.c ../shaders.glsl.h)

    set_target_properties(game PROPERTIES
        LINK_FLAGS /SUBSYSTEM:CONSOLE
    )
else()
    add_executable(game main.c ../shaders.glsl.h)
endif()

target_link_libraries(game sokol)
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(game PUBLIC --shell-file ../sokol/shell.html)
    target_link_options(game PUBLIC -sNO_FILESYSTEM=1 
                                      -sASSERTIONS=1 -sMALLOC=emmalloc --closure=1)

    math(EXPR INITIAL_MEM "1024 * 64 * 750" OUTPUT_FORMAT DECIMAL)
    set(INITIAL_MEM -sINITIAL_MEMORY=${INITIAL_MEM})
    target_link_options(game PUBLIC ${INITIAL_MEM})
endif()

if(MSVC)
  target_compile_options(game PRIVATE /W4 /WX)
else()
  target_compile_options(game PRIVATE -Wall -Wextra -Werror
                                         -Wno-missing-braces
                                         -Wno-missing-field-initializers)
endif()

# explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(game PRIVATE LINKER:-dead_strip)
endif()
