cmake_minimum_required(VERSION 3.20.0)

set(DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/esp32.overlay")
set(DTS_BINDING_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dts/bindings")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
enable_language(CXX)

project(cnc-pendant)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB SOURCES "${SRC_DIR}/*.cpp")

target_sources(app PRIVATE ${SOURCES})
