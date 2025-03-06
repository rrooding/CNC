cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(CMAKE_CXX_STANDARD 20)
enable_language(CXX)

project(esp32-pendant)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

file(GLOB SOURCES "${SRC_DIR}/*.cpp")

target_sources(app PRIVATE ${SOURCES})

# GrblParser sources
set(GRBL_DIR ${LIB_DIR}/GrblParser)
file(GLOB GRBL_SOURCES "${GRBL_DIR}/src/*.c")
target_sources(app PRIVATE ${GRBL_SOURCES})

# Include directory for GrblParser headers
target_include_directories(app PRIVATE "${GRBL_DIR}/src/")
