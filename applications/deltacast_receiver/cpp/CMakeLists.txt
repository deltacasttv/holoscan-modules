# SPDX-FileCopyrightText: Copyright (c) 2022-2026, DELTACAST.TV. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

add_executable(deltacast_receiver main.cpp)

target_link_libraries(deltacast_receiver
  PRIVATE
    holoscan::core
    holoscan::ops::format_converter
    holoscan::ops::holoviz
    holoscan::videomaster
)

add_custom_target(deltacast_receiver_yaml
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_SOURCE_DIR}/deltacast_receiver.yaml"
    "${CMAKE_CURRENT_BINARY_DIR}"
  DEPENDS "deltacast_receiver.yaml"
  BYPRODUCTS "deltacast_receiver.yaml"
)
add_dependencies(deltacast_receiver
  deltacast_receiver_yaml
  endoscopy_data
)

install(TARGETS deltacast_receiver DESTINATION bin/deltacast_receiver/cpp)
install(FILES deltacast_receiver.yaml DESTINATION bin/deltacast_receiver/cpp)
