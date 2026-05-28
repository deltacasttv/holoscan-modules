# SPDX-FileCopyrightText: Copyright (c) 2022-2026, DELTACAST.TV. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

if(HOLOSCAN_DELTACAST_BUILD_TESTING)
  # Configure the yaml file for testing (override count: 0 → count: 10)
  file(READ "${CMAKE_CURRENT_SOURCE_DIR}/deltacast_transmitter.yaml" CONFIG_FILE)
  string(REPLACE "count: 0" "count: 10" CONFIG_FILE ${CONFIG_FILE})
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/deltacast_transmitter.yaml" ${CONFIG_FILE})

  add_test(NAME deltacast_transmitter_python_test
           COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/deltacast_transmitter.py
                   --config ${CMAKE_CURRENT_BINARY_DIR}/deltacast_transmitter.yaml
           WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  set_tests_properties(deltacast_transmitter_python_test PROPERTIES
    PASS_REGULAR_EXPRESSION "Reach end of file or playback count reaches to the limit. Stop ticking.;"
    FAIL_REGULAR_EXPRESSION "[^a-z]Error;ERROR;Failed")

  set_property(TEST deltacast_transmitter_python_test PROPERTY ENVIRONMENT
    "PYTHONPATH=${DELTACAST_PYTHON_ROOT}")
endif()

install(FILES deltacast_transmitter.py deltacast_transmitter.yaml
        DESTINATION bin/deltacast_transmitter/python)
