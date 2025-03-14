cmake_minimum_required(VERSION 3.16)

set(IDF_TARGET "esp32c6")

execute_process(
    COMMAND git describe --tags --always --dirty=-dev
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_VERSION MATCHES "^v[0-9]+\\.[0-9]+\\.[0-9]+$")
    set(PROJECT_VER ${GIT_VERSION})
else()
    set(PROJECT_VER "${GIT_VERSION}-dev")
endif()

set(PROJECT_VER ${GIT_VERSION})

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project("SmartClimate" C)
