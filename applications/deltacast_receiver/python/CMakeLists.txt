# SPDX-FileCopyrightText: Copyright (c) 2022-2026, DELTACAST.TV. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

# Ensure test data installs
add_custom_target(deltacast_receiver_python DEPENDS endoscopy_data)

if(HOLOSCAN_DELTACAST_BUILD_TESTING)
  add_test(NAME deltacast_receiver_python_test
           COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/deltacast_receiver.py
                   --config ${CMAKE_CURRENT_SOURCE_DIR}/deltacast_receiver.yaml
           WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  set_tests_properties(deltacast_receiver_python_test PROPERTIES
    PASS_REGULAR_EXPRESSION "Running the graph;"
    FAIL_REGULAR_EXPRESSION "[^a-z]Error;ERROR;Failed")

  set_property(TEST deltacast_receiver_python_test PROPERTY ENVIRONMENT
    "PYTHONPATH=${DELTACAST_PYTHON_ROOT}")
endif()

install(FILES deltacast_receiver.py deltacast_receiver.yaml
        DESTINATION bin/deltacast_receiver/python)
