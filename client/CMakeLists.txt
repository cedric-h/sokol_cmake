cmake_minimum_required(VERSION 3.0)
project(client)
set(CMAKE_C_STANDARD 11)
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

# Linux -pthread shenanigans
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

#=== LIBRARY: microui
add_library(microui STATIC ../mui/microui.c)

#=== LIBRARY: bq_websocket
add_library(bq_websocket STATIC ../bq_websocket/bq_websocket_platform.c
                                ../bq_websocket/bq_websocket_platform.h
                                ../bq_websocket/bq_websocket.c
                                ../bq_websocket/bq_websocket.h)

#== LIBRARY: formpack
if (CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(RUN_FORMPACK_GEN_CMD "\"./../../formpack/build/Debug/gen.exe\"")
else()
    set(RUN_FORMPACK_GEN_CMD "../../formpack/build/gen")
endif()
add_custom_command(OUTPUT ../../formpack/build/formpack.h
    COMMAND cmake --build ../../formpack/build/ && ${RUN_FORMPACK_GEN_CMD}
    DEPENDS ../formpack/formpack.dd
    DEPENDS ../formpack/gen.c
)
set_source_files_properties(../../formpack/build/formpack.h PROPERTIES GENERATED TRUE)

#== OPTION: colored output
if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
    option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
    if (${FORCE_COLORED_OUTPUT})
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
           add_compile_options (-fdiagnostics-color=always)
        elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
           add_compile_options (-fcolor-diagnostics)
        endif ()
    endif ()
endif()


#=== LIBRARY: sokol
# add headers to the the file list because they are useful to have in IDEs
set(SOKOL_HEADERS
    ../sokol/sokol_gfx.h
    ../sokol/sokol_app.h
    ../sokol/sokol_audio.h
    ../sokol/sokol_time.h
    ../sokol/sokol_glue.h)
add_library(sokol STATIC ../sokol/sokol.c ${SOKOL_HEADERS})
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
target_include_directories(sokol INTERFACE ../sokol)


#== GENERATION: shaders

if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    set(COMPILE_SHADER_CMD ../../sokol-tools-bin/bin/win32/sokol-shdc.exe --input ../shaders.glsl --output ../shaders.glsl.h --slang hlsl5)
elseif(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(COMPILE_SHADER_CMD ../../sokol-tools-bin/bin/osx/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang metal_macos)
elseif(CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    set(COMPILE_SHADER_CMD ../../sokol-tools-bin/bin/linux/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang glsl100)
else()
    set(COMPILE_SHADER_CMD ../../sokol-tools-bin/bin/linux/sokol-shdc --input ../shaders.glsl --output ../shaders.glsl.h --slang glsl330)
endif()
add_custom_command(OUTPUT ../shaders.glsl.h
    COMMAND ${COMPILE_SHADER_CMD}
    DEPENDS shaders.glsl
)
set_source_files_properties(../shaders.glsl.h PROPERTIES
    GENERATED TRUE
)

#=== EXECUTABLE: client
if(CMAKE_SYSTEM_NAME STREQUAL Windows)
    add_executable(client WIN32 main.c ../shaders.glsl.h  ../../formpack/build/formpack.h)

    set_target_properties(client PROPERTIES
        LINK_FLAGS /SUBSYSTEM:CONSOLE
    )
else()
    add_executable(client main.c ../shaders.glsl.h  ../../formpack/build/formpack.h)
endif()

target_link_libraries(client sokol)
target_link_libraries(client microui)
target_link_libraries(client bq_websocket)
if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(client PUBLIC --shell-file ../../sokol/shell.html)
    target_link_options(client PUBLIC -sNO_FILESYSTEM=1 
                                      -sASSERTIONS=1 -sMALLOC=emmalloc --closure=1)

    math(EXPR INITIAL_MEM "1024 * 64 * 750" OUTPUT_FORMAT DECIMAL)
    set(INITIAL_MEM -sINITIAL_MEMORY=${INITIAL_MEM})
    target_link_options(client PUBLIC ${INITIAL_MEM})
endif()

if(MSVC)
  target_compile_options(client PRIVATE /W4 /WX)
else()
  target_compile_options(client PRIVATE -Wall -Wextra -Werror
                                         # -g -O0
                                         -Wno-missing-braces
                                         -Wno-missing-field-initializers)
endif()

# explicitly strip dead code
if (CMAKE_C_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    target_link_options(client PRIVATE LINKER:-dead_strip)
endif()
