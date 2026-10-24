cmake_minimum_required(VERSION 3.16)

project(sht30_portable_driver VERSION 0.1.0 LANGUAGES C)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(SHT30_TOP_LEVEL ON)
else()
    set(SHT30_TOP_LEVEL OFF)
endif()

option(SHT30_BUILD_TESTS "Build host-side unit tests" ${SHT30_TOP_LEVEL})
option(SHT30_BUILD_STM32_PORT "Build the STM32 HAL adapter" OFF)
option(SHT30_BUILD_STM32_EXAMPLE "Compile the STM32F103 integration example" OFF)
option(SHT30_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
set(STM32_HAL_INCLUDE_DIR "" CACHE STRING "Directory containing stm32f1xx_hal.h")

add_library(sht30 src/sht30.c)
add_library(sht30::sht30 ALIAS sht30)
target_include_directories(sht30 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(sht30 PUBLIC c_std_99)

function(sht30_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
        if(SHT30_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
        if(SHT30_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

sht30_enable_warnings(sht30)

if(SHT30_BUILD_TESTS)
    enable_testing()
    add_executable(sht30_tests
        tests/test_sht30.c
        tests/mock_i2c.c
    )
    target_include_directories(sht30_tests PRIVATE tests)
    target_link_libraries(sht30_tests PRIVATE sht30)
    sht30_enable_warnings(sht30_tests)
    add_test(NAME sht30_unit_tests COMMAND sht30_tests)
endif()

if(SHT30_BUILD_STM32_PORT)
    if(NOT STM32_HAL_INCLUDE_DIR)
        message(FATAL_ERROR "Set STM32_HAL_INCLUDE_DIR to the directory containing stm32f1xx_hal.h")
    endif()
    if(NOT IS_ABSOLUTE "${STM32_HAL_INCLUDE_DIR}")
        set(STM32_HAL_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${STM32_HAL_INCLUDE_DIR}")
    endif()
    add_library(sht30_stm32 port/stm32/sht30_stm32.c)
    target_include_directories(sht30_stm32 PUBLIC port/stm32 ${STM32_HAL_INCLUDE_DIR})
    target_link_libraries(sht30_stm32 PUBLIC sht30)
    sht30_enable_warnings(sht30_stm32)

    if(SHT30_BUILD_STM32_EXAMPLE)
        add_library(sht30_stm32_example OBJECT examples/stm32f103/sht30_example.c)
        target_link_libraries(sht30_stm32_example PRIVATE sht30_stm32)
        sht30_enable_warnings(sht30_stm32_example)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS sht30 EXPORT sht30Targets)
install(FILES include/sht30.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT sht30Targets NAMESPACE sht30:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sht30)
