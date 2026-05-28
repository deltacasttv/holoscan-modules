# SPDX-FileCopyrightText: Copyright (c) 2022-2026, DELTACAST.TV. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_executable(deltacast_transmitter main.cpp)

target_link_libraries(deltacast_transmitter
  PRIVATE
    holoscan::core
    holoscan::ops::video_stream_replayer
    holoscan::ops::format_converter
    holoscan::videomaster
)

add_custom_target(deltacast_transmitter_yaml
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/deltacast_transmitter.yaml"
    "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "deltacast_transmitter.yaml"
  BYPRODUCTS "deltacast_transmitter.yaml"
)
add_dependencies(deltacast_transmitter deltacast_transmitter_yaml)

install(TARGETS deltacast_transmitter DESTINATION bin/deltacast_transmitter/cpp)
install(FILES deltacast_transmitter.yaml DESTINATION bin/deltacast_transmitter/cpp)
