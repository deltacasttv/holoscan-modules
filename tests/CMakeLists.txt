# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(cpp)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(
    NAME pytest
    COMMAND ${Python3_EXECUTABLE} -m pytest ${PROJECT_SOURCE_DIR}/tests/python/ -vv --tb=short -s
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties(pytest PROPERTIES
    ENVIRONMENT "PYTHONPATH=${HOLOSCAN_DELTACAST_PYTHON_ROOT}:$ENV{PYTHONPATH};HOLOSCAN_DELTACAST_BUILD_DIR=${CMAKE_BINARY_DIR}"
    SKIP_RETURN_CODE 5
    LABELS "python")
