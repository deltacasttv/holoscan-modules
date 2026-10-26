# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

if(TARGET videomaster)
    add_subdirectory(cpp)
endif()

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_test(
    NAME pytest
    COMMAND ${Python3_EXECUTABLE} -m pytest ${PROJECT_SOURCE_DIR}/tests/python/ -v --tb=short
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set_tests_properties(pytest PROPERTIES
    ENVIRONMENT "HOLOSCAN_DELTACAST_BUILD_DIR=${CMAKE_BINARY_DIR}"
    ENVIRONMENT_MODIFICATION "PYTHONPATH=path_list_prepend:${HOLOSCAN_DELTACAST_PYTHON_ROOT}"
    SKIP_RETURN_CODE 5
    LABELS "python")
