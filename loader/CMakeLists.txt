add_library(loader INTERFACE)
target_include_directories(loader INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_sources(loader INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/swdloader.c
    ${CMAKE_CURRENT_LIST_DIR}/swdloader.h)
target_link_libraries(loader INTERFACE gpio)
