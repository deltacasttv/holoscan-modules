# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

find_package(GTest REQUIRED)

add_executable(test_operators test_operators.cpp)

target_link_libraries(test_operators PRIVATE
    videomaster
    holoscan::core
    GTest::gtest_main)

target_include_directories(test_operators PRIVATE
    ${PROJECT_SOURCE_DIR}/operators)

include(GoogleTest)
gtest_discover_tests(test_operators PROPERTIES LABELS "unit")
