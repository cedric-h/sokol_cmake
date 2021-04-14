cmake_minimum_required(VERSION 2.8.11)
project(SERVER)

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

#=== LIBRARY: bq_websocket
add_library(bq_websocket STATIC ../bq_websocket/bq_websocket_platform.c
                                ../bq_websocket/bq_websocket_platform.h
                                ../bq_websocket/bq_websocket.c
                                ../bq_websocket/bq_websocket.h)

add_executable(server ./main.c ../../formpack/build/formpack.h)
target_link_libraries(server bq_websocket)
