cmake_minimum_required (VERSION 3.5)

option (BUILD_FOR "build options" "pi-pigpio, pi-gpiod or rock-5b-gpiod")

set (PROJECT_NAME swdloader)

set (CMAKE_C_STANDARD 11)
set (CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_FOR_TYPE STREQUAL "Debug")
    add_compile_options ("-g")
else ()
    add_compile_options ("-O3")
    add_compile_options ("-flto")
endif ()

# Set SWRST_GPIO to 0 to disable optional reset pin
if (BUILD_FOR STREQUAL "pi-pigpio")
    add_compile_options ("-DSWCLK_GPIO=25")
    add_compile_options ("-DSWDIO_GPIO=24")
    add_compile_options ("-DSWRST_GPIO=23")
    add_compile_options ("-DUSE_LIBPIGPIO")
    add_link_options ("-lpigpio")
elseif (BUILD_FOR STREQUAL "pi-gpiod")
    add_compile_options ("-DSWCLK_GPIO=25")
    add_compile_options ("-DSWDIO_GPIO=24")
    add_compile_options ("-DSWRST_GPIO=23")
    add_compile_options ("-DUSE_LIBGPIOD")
    add_link_options ("-lgpiod")
elseif (BUILD_FOR STREQUAL "rock-5b-gpiod")
    add_compile_options ("-DSWCLK_GPIO=45")
    add_compile_options ("-DSWDIO_GPIO=44")
    add_compile_options ("-DSWRST_GPIO=149")
    add_compile_options ("-DUSE_LIBGPIOD")
    add_link_options ("-lgpiod")
else ()
    message (FATAL_ERROR "-DBUILD_FOR= must be pi-pigpio, pi-gpiod or rock-5b-gpiod")
endif ()

project (${PROJECT_NAME})

add_subdirectory (loader)
add_subdirectory (gpio)

add_executable (${PROJECT_NAME} main.c)

target_link_libraries (${PROJECT_NAME} PUBLIC loader)

message ("-- Building for ${BUILD_FOR}")
