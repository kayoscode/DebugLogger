cmake_minimum_required(VERSION 3.0)

SET(PROJ_NAME DebugLogger)
project(${PROJ_NAME})

file(GLOB SRC 
    "src/*.cpp"
    "include/*.h"
)

add_library(${PROJ_NAME}
    "${SRC}"
)

target_include_directories("${PROJ_NAME}" PRIVATE ${PROJECT_SOURCE_DIR}/./include)