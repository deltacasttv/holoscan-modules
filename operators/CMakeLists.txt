# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

# Python3 + pybind11 are pulled in (find_package + FetchContent) by
# pybind11_add_holohub_module, which the operator's own python/CMakeLists.txt
# includes when cpp == 'cpp'. No find_package needed
# here at the aggregate operators/ level.

# Each operator is gated on its own OP_<name> option, defaulting to ${BUILD_ALL}.
# Add one add_holohub_operator() call per operator.
add_holohub_operator(deltacast_videomaster)

# Install the aggregate __init__.py alongside each operator's built module.
configure_file(
    ${PROJECT_SOURCE_DIR}/python/holoscan/deltacast/__init__.py
    ${HOLOSCAN_DELTACAST_PYTHON_PKG_DIR}/__init__.py COPYONLY)

# Relative DESTINATION — interpreted as <CMAKE_INSTALL_PREFIX>/holoscan/<mod>
# in plain `cmake --install` (set --prefix to ${Python3_SITEARCH}), and as
# <wheel-root>/holoscan/<mod> when scikit-build-core builds the wheel.
install(DIRECTORY ${HOLOSCAN_DELTACAST_PYTHON_PKG_DIR}
    DESTINATION holoscan)
