add_library(gpio INTERFACE)
target_include_directories(gpio INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_sources(gpio INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/gpiopin.c
    ${CMAKE_CURRENT_LIST_DIR}/gpiopin.h)
