# *******************************************************************************
# Copyright (c) 2026 ZF Friedrichshafen AG
#
# See the NOTICE file(s) distributed with this work for additional
# information regarding copyright ownership.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# 
# Contributors:
#   Thomas Pfleiderer - initial function designer
# *******************************************************************************

cmake_minimum_required(VERSION 3.16)

project(function_designer
        VERSION 0.1.0
        DESCRIPTION "Editor for Eclipse autoapiframework function specifications"
        LANGUAGES CXX)

include(CTest)
option(AFD_BUILD_APP "Build the wxWidgets function designer application" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()


find_program(VSPEC_EXECUTABLE
    NAMES vspec vspec.exe
    HINTS
        "$ENV{HOME}/venvs/vss-tools/bin"
        "$ENV{USERPROFILE}/venvs/vss-tools/Scripts"
)

if(NOT VSPEC_EXECUTABLE)
    message(WARNING
        "vss-tools not installed - importing signals from vspec files will be disabled at runtime.")
    set(VSPEC_EXECUTABLE "")
endif()


if(AFD_BUILD_APP)
    option(ACD_FETCH_WXWIDGETS "Download and build wxWidgets when no installation is found" ON)

    # vcpkg and recent wxWidgets installations provide a package config file,
    # system installations are found through CMake's FindwxWidgets module.
    find_package(wxWidgets CONFIG QUIET COMPONENTS core base adv)
    if(NOT wxWidgets_FOUND)
        find_package(wxWidgets QUIET COMPONENTS core base adv)
    endif()

    if(wxWidgets_FOUND)
        if(wxWidgets_USE_FILE)
            include(${wxWidgets_USE_FILE})
        endif()
        set(ACD_WX_LIBRARIES ${wxWidgets_LIBRARIES})
    elseif(ACD_FETCH_WXWIDGETS)
        message(STATUS "wxWidgets not found - building it from source (this takes a while on the first run)")
        include(FetchContent)
        set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
        set(wxBUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(wxBUILD_SAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(wxWidgets
            GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
            GIT_TAG v3.2.6
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(wxWidgets)
        set(ACD_WX_LIBRARIES wx::core wx::base)
    else()
        message(FATAL_ERROR
            "wxWidgets not found. Install it (vcpkg: 'vcpkg install wxwidgets', "
            "Linux: 'libwxgtk3.2-dev') or configure with -DACD_FETCH_WXWIDGETS=ON.")
    endif()

    if(WIN32)
        enable_language(RC)
        configure_file(src/app_icon.rc.in app_icon.rc @ONLY)
        set(ACD_WINDOWS_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
    endif()

    # GUI executable for the Auto API Function Designer application
    add_executable(AutoAPIFunctionDesigner WIN32
        src/main.cpp
        src/main_frame.cpp
        src/json_signal_parser.cpp
        src/yaml_node.cpp
        src/yaml_parser.cpp
        src/yaml_writer.cpp
        src/meta_model.cpp
        src/validate_function.cpp    
        src/function_specification.cpp
        src/supervision_editor.cpp
        ${ACD_WINDOWS_RESOURCES}
    )

    target_include_directories(AutoAPIFunctionDesigner PRIVATE src)
    target_link_libraries(AutoAPIFunctionDesigner PRIVATE ${ACD_WX_LIBRARIES})
    target_compile_definitions(AutoAPIFunctionDesigner PRIVATE
        AFD_VSPEC_EXECUTABLE="${VSPEC_EXECUTABLE}")

    if(MSVC)
        target_compile_options(AutoAPIFunctionDesigner PRIVATE /W4 /WX /utf-8)
        target_compile_definitions(AutoAPIFunctionDesigner PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT AutoAPIFunctionDesigner)
    else()
        target_compile_options(AutoAPIFunctionDesigner PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    set_target_properties(AutoAPIFunctionDesigner PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:function_designer>")

    if(UNIX AND NOT APPLE)
        set(ACD_LINUX_ICON_COMMAND
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CMAKE_CURRENT_SOURCE_DIR}/autoapiframework_icon-128x128.png"
                    "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/autoapiframework_icon.png")
    endif()

    # The meta model and the sample specification are picked up next to the executable.
    add_custom_command(TARGET AutoAPIFunctionDesigner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/autoapiframework_metadata_V04.yaml"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/autoapiframework_meta_model.yaml"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/speed_hazard_detection.afs"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/speed_hazard_detection.afs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/vehicle_speed_fusion_multirate.afs"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/vehicle_speed_fusion_multirate.afs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/mandatory_property_is_missing_syntax_error.afs"            
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/mandatory_property_is_missing_syntax_error.afs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/examples/wheel_speed_plausibility_check.afs"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/wheel_speed_plausibility_check.afs"            
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/autoapiframework_logo.png"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/autoapiframework_logo.png"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/autoapiframework_logo_large.png"
                "$<TARGET_FILE_DIR:AutoAPIFunctionDesigner>/autoapiframework_logo_large.png"
        ${ACD_LINUX_ICON_COMMAND}
        VERBATIM)

    install(TARGETS AutoAPIFunctionDesigner RUNTIME DESTINATION bin)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/examples/autoapiframework_meta_model.yaml" DESTINATION bin)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/autoapiframework_logo.png" DESTINATION bin)
    if(UNIX AND NOT APPLE)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/autoapiframework_icon-128x128.png"
                DESTINATION bin
                RENAME autoapiframework_icon.png)
    endif()

    # Commandline executable for the Auto API Function Designer syntax check tool
    add_executable(AutoAPIFunctionDesigner_SyntaxCheck
        src/syntax_check/main.cpp
        src/yaml_node.cpp
        src/yaml_parser.cpp
        src/meta_model.cpp
        src/validate_function.cpp
    )

    target_include_directories(AutoAPIFunctionDesigner_SyntaxCheck PRIVATE src)
    target_compile_definitions(AutoAPIFunctionDesigner_SyntaxCheck PRIVATE
        AFD_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples")
    if(MSVC)
        target_compile_options(AutoAPIFunctionDesigner_SyntaxCheck PRIVATE /W4 /WX /utf-8)
    else()
        target_compile_options(AutoAPIFunctionDesigner_SyntaxCheck PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endif() #(AFD_BUILD_APP)

# Standalone AFS parser CLI; it does not depend on the wxWidgets application.
add_executable(AfsFileParser
    src/afs_file_parser/main.cpp
    src/afs_file_parser/afs_file_expectation.cpp
    src/afs_file_parser/afs_file_parser.cpp
    src/yaml_node.cpp
    src/yaml_parser.cpp
)
target_include_directories(AfsFileParser PRIVATE src/afs_file_parser)
if(MSVC)
    target_compile_options(AfsFileParser PRIVATE /W4 /WX /utf-8)
else()
    target_compile_options(AfsFileParser PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
install(TARGETS AfsFileParser RUNTIME DESTINATION bin)

if(BUILD_TESTING)
    add_executable(afs_file_parser_tests
        tests/afs_file_parser_tests.cpp
        src/afs_file_parser/afs_file_parser.cpp
        src/yaml_node.cpp
        src/yaml_parser.cpp
    )
    target_include_directories(afs_file_parser_tests PRIVATE src/afs_file_parser)
    target_compile_definitions(afs_file_parser_tests PRIVATE
        AFS_PARSER_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples")
    if(MSVC)
        target_compile_options(afs_file_parser_tests PRIVATE /W4 /WX /utf-8)
    else()
        target_compile_options(afs_file_parser_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME afs_file_parser_tests COMMAND afs_file_parser_tests)

    add_executable(function_designer_tests
        tests/yaml_tests.cpp
        src/yaml_node.cpp
        src/yaml_parser.cpp
        src/yaml_writer.cpp
    )
    target_include_directories(function_designer_tests PRIVATE src)
    target_compile_definitions(function_designer_tests PRIVATE
        AFD_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples")
    if(MSVC)
        target_compile_options(function_designer_tests PRIVATE /W4 /WX /utf-8)
    else()
        target_compile_options(function_designer_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME function_designer_tests COMMAND function_designer_tests)

    # Unit tests for ValidateFunction / MetaModel — no wxWidgets dependency.
    add_executable(syntax_check_tests
        tests/syntax_check_tests.cpp
        src/yaml_node.cpp
        src/yaml_parser.cpp
        src/meta_model.cpp
        src/validate_function.cpp
    )
    target_include_directories(syntax_check_tests PRIVATE src)
    target_compile_definitions(syntax_check_tests PRIVATE
        AFD_EXAMPLES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/examples")
    if(MSVC)
        target_compile_options(syntax_check_tests PRIVATE /W4 /WX /utf-8)
    else()
        target_compile_options(syntax_check_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
    add_test(NAME syntax_check_tests COMMAND syntax_check_tests)

    # Integration tests that invoke the CLI executable as a subprocess.
    # Each test uses PASS_REGULAR_EXPRESSION so the expected output is verified
    # regardless of whether the exit code is zero or non-zero.
    if(AFD_BUILD_APP)
        set(SC_EXE "$<TARGET_FILE:AutoAPIFunctionDesigner_SyntaxCheck>")
        set(EX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

        add_test(NAME syntax_check_cli_speed_hazard
                 COMMAND "${SC_EXE}" "${EX_DIR}/speed_hazard_detection.afs")
        set_tests_properties(syntax_check_cli_speed_hazard PROPERTIES
                             PASS_REGULAR_EXPRESSION "Syntax OK")

        add_test(NAME syntax_check_cli_vehicle_speed_fusion
                 COMMAND "${SC_EXE}" "${EX_DIR}/vehicle_speed_fusion_multirate.afs")
        set_tests_properties(syntax_check_cli_vehicle_speed_fusion PROPERTIES
                             PASS_REGULAR_EXPRESSION "Syntax OK")

        add_test(NAME syntax_check_cli_wheel_speed
                 COMMAND "${SC_EXE}" "${EX_DIR}/wheel_speed_plausibility_check.afs")
        set_tests_properties(syntax_check_cli_wheel_speed PROPERTIES
                             PASS_REGULAR_EXPRESSION "Syntax OK")

        add_test(NAME syntax_check_cli_mandatory_missing
                 COMMAND "${SC_EXE}"
                         "${EX_DIR}/mandatory_property_is_missing_syntax_error.afs")
        set_tests_properties(syntax_check_cli_mandatory_missing PROPERTIES
                             PASS_REGULAR_EXPRESSION "Syntax error")

        add_test(NAME syntax_check_cli_missing_file
                 COMMAND "${SC_EXE}" "nonexistent_file.afs")
        set_tests_properties(syntax_check_cli_missing_file PROPERTIES
                             PASS_REGULAR_EXPRESSION "Could not read")

        add_test(NAME syntax_check_cli_no_args COMMAND "${SC_EXE}")
        set_tests_properties(syntax_check_cli_no_args PROPERTIES
                             PASS_REGULAR_EXPRESSION "Usage:")
    endif() #(AFD_BUILD_APP)
endif() #(BUILD_TESTING)
