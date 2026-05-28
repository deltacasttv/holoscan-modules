# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.24)

project(holoscan_deltacast
    VERSION 0.1.0
    DESCRIPTION "Holoscan components for using the Deltacast Videomaster capture card"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------

find_package(holoscan REQUIRED COMPONENTS core)
find_package(CUDAToolkit REQUIRED)

# ---------------------------------------------------------------------------
# Build options
# ---------------------------------------------------------------------------
# BUILD_ALL toggles every operator/application/package on by default. It
# matches the HoloHub-monorepo convention: each subproject is a CMake option
# (e.g. APP_<name>, OP_<name>, PKG_<name>) defined via add_holohub_application
# / add_holohub_operator / add_holohub_package, defaulting to ${BUILD_ALL}.
# To build a single subproject, configure with -DBUILD_ALL=OFF -DAPP_<name>=ON.
#
# Default to ON only when this is the top-level CMake project (standalone
# build). When nested under another project (e.g. consumed as a HoloHub
# external dependency via add_subdirectory), defer to whatever the parent
# build has decided — otherwise this module's BUILD_ALL=ON would leak into
# the parent's variable scope and force-enable unrelated subprojects there.

if(NOT DEFINED BUILD_ALL)
    if(PROJECT_IS_TOP_LEVEL)
        set(BUILD_ALL ON CACHE BOOL "Build all operators, applications, and packages by default")
    else()
        set(BUILD_ALL OFF CACHE BOOL "Build all operators, applications, and packages by default")
    endif()
endif()

# Module tests live behind a module-namespaced flag rather than the standard
# CMake BUILD_TESTING variable. This lets a parent project (HoloHub, another
# module) keep its own BUILD_TESTING setting independent of whether *this*
# module's tests are built. Defaults ON when standalone, OFF when nested.
if(NOT DEFINED HOLOSCAN_DELTACAST_BUILD_TESTING)
    if(PROJECT_IS_TOP_LEVEL)
        set(HOLOSCAN_DELTACAST_BUILD_TESTING ON
            CACHE BOOL "Build the holoscan-deltacast module's CTest and pytest suites")
    else()
        set(HOLOSCAN_DELTACAST_BUILD_TESTING OFF
            CACHE BOOL "Build the holoscan-deltacast module's CTest and pytest suites")
    endif()
endif()

# ---------------------------------------------------------------------------
# Python package root in the build tree.
# PYTHONPATH=${HOLOSCAN_DELTACAST_PYTHON_ROOT} lets Python find
# holoscan.deltacast alongside the installed Holoscan SDK package.
# ---------------------------------------------------------------------------

set(HOLOSCAN_DELTACAST_PYTHON_ROOT
    ${CMAKE_BINARY_DIR}/python)
set(HOLOSCAN_DELTACAST_PYTHON_PKG_DIR
    ${HOLOSCAN_DELTACAST_PYTHON_ROOT}/holoscan/deltacast)

# Variables consumed by pybind11_add_holohub_module:
#   HOLOHUB_PYTHON_MODULE_OUT_DIR — where per-operator subpackages land
#   HOLOSCAN_INSTALL_LIB_DIR     — relative lib dir, used for rpath resolution
set(HOLOHUB_PYTHON_MODULE_OUT_DIR
    ${HOLOSCAN_DELTACAST_PYTHON_PKG_DIR})
file(MAKE_DIRECTORY ${HOLOHUB_PYTHON_MODULE_OUT_DIR})
if(NOT DEFINED HOLOSCAN_INSTALL_LIB_DIR)
    set(HOLOSCAN_INSTALL_LIB_DIR lib)
endif()

# ---------------------------------------------------------------------------
# CMake helpers
# ---------------------------------------------------------------------------
# HoloHubConfigHelpers.cmake provides add_holohub_application/operator/package
# which gate each subproject on its own option (APP_/OP_/PKG_<name>). The
# helpers ship with the HoloHub CLI and are copied into this module's cmake/
# directory by the cookiecutter post-gen hook so the project also builds
# without a HoloHub clone.
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(HoloHubConfigHelpers)

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------

add_subdirectory(applications)
add_subdirectory(operators)

if(HOLOSCAN_DELTACAST_BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# ---------------------------------------------------------------------------
# Stage dev-mode hook files in the build tree
# ---------------------------------------------------------------------------
# `./holohub install --dev` copies these into the user's
# Python user-site so `import holoscan.deltacast` works in
# any shell — without a wheel install. We only *stage* them here; the install
# itself is opt-in so a plain `./holohub build` never
# touches the host's Python environment.

set(_DEV_HELPER_FILE "${CMAKE_BINARY_DIR}/holoscan_deltacast_dev.py")
set(_DEV_PTH_FILE    "${CMAKE_BINARY_DIR}/holoscan-deltacast-dev.pth")
set(_DEV_TARGET_PATH "${HOLOSCAN_DELTACAST_PYTHON_ROOT}/holoscan")

file(WRITE ${_DEV_HELPER_FILE}
"# Auto-generated by ./holohub build — do not edit.
# Extends holoscan.__path__ to include this module's build tree, so that
# `import holoscan.deltacast` resolves to the live build
# output without a wheel install. Installed to user-site by
# `./holohub install --dev`.
import os

_BUILD_PATH = r\"${_DEV_TARGET_PATH}\"

try:
    import holoscan
    if os.path.isdir(_BUILD_PATH) and _BUILD_PATH not in holoscan.__path__:
        holoscan.__path__.insert(0, _BUILD_PATH)
except Exception:
    # Stay silent — a stale dev hook must never abort Python startup.
    pass
")

file(WRITE ${_DEV_PTH_FILE}
"import holoscan_deltacast_dev
")

# ---------------------------------------------------------------------------
# Packaging — Debian (.deb) via holohub_configure_deb
# ---------------------------------------------------------------------------
# Option name follows the HoloHub convention: PKG_<package-name-with-underscores>.
# `./holohub package` derives the option name from metadata.json:module.name,
# so this stays in sync with what the CLI emits.

option(PKG_holoscan_deltacast
       "Build the holoscan-deltacast Debian package" OFF)

if(PKG_holoscan_deltacast)
    include(holohub_configure_deb)
    holohub_configure_deb(
        NAME        "holoscan-deltacast"
        DESCRIPTION "${PROJECT_DESCRIPTION}"
        VERSION     "${PROJECT_VERSION}"
        VENDOR      "Deltacast"
        CONTACT     "support@deltacast.tv"
        DEPENDS     "holoscan (>= 4.2.0)"
    )
endif()
