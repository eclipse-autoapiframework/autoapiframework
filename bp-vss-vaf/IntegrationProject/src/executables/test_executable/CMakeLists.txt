set(TARGET vaf_TestExecutable_user_controller)

add_library(${TARGET} INTERFACE)

target_include_directories(${TARGET}
INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_sources(${TARGET}
INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/user_controller.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/user_controller.cpp
)
