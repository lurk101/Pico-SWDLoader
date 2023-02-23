cmake_minimum_required(VERSION 3.5)

option(USE "build options" "")

# Definitions
set(PROJECT_NAME swdloader)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
else()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto")
endif()

if (USE STREQUAL "pi-pigpio")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSWCLK_PIN=25 -DSWDIO_PIN=24 -DRESET_PIN=23") # 0 for none
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_LIBPIGPIO")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lpigpio")
elseif (USE STREQUAL "pi-gpiod")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSWCLK_PIN=25 -DSWDIO_PIN=24 -DRESET_PIN=0") # 0 for none
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_LIBGPIOD")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgpiod")
elseif (USE STREQUAL "rock-5b-gpiod")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSWCLK_PIN=45 -DSWDIO_PIN=44 -DRESET_PIN=149")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_LIBGPIOD")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgpiod")
else()
message(FATAL_ERROR "-DUSE= must be pi-pigpio, pi-gpiod, rock-5b-gpiod")
endif()

project(${PROJECT_NAME})

# Include paths

# Build static/shared library
add_subdirectory(loader)
add_subdirectory(gpio)

add_executable(${PROJECT_NAME} main.c)

target_link_libraries(${PROJECT_NAME} PUBLIC loader)

message("-- Building for ${USE}")
