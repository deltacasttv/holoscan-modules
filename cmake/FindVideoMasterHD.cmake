# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# FindVideoMasterHD.cmake
#
# Locates the Deltacast VideoMaster HD SDK and exposes the VideoMasterHD::Core
# imported target. Falls back to a bundled mock when VideoMaster_SDK_DIR is
# not set (or when VIDEOMASTER_USE_MOCK is explicitly set to ON), which is
# enough to build *and run* the operator and sample apps without hardware.
#
# Result variables
#   VideoMasterHD_FOUND       - TRUE when the target is available
#   VideoMasterHD_VERSION     - SDK version string
#   VideoMasterHD_MOCK        - TRUE when the mock is active
#
# Cache variables
#   VideoMaster_SDK_DIR       - Root of the real SDK installation
#   VIDEOMASTER_USE_MOCK      - Force the mock regardless of SDK presence

include(FindPackageHandleStandardArgs)

option(VIDEOMASTER_USE_MOCK
    "Use the bundled VideoMasterHD mock instead of the real SDK (enables builds and demo runs without hardware)"
    ON)

# ── Real SDK path detection ────────────────────────────────────────────────
set(_sdk_root "")
if(VideoMaster_SDK_DIR)
    set(_sdk_root "${VideoMaster_SDK_DIR}")
elseif(EXISTS "/usr/share/deltacast/videomaster/cmake/VideoMasterHDConfig.cmake")
    set(_sdk_root "/usr")
elseif(EXISTS "/usr/local/cmake/VideoMasterHDConfig.cmake")
    set(_sdk_root "/usr/local")
elseif(EXISTS "/usr/local/deltacast")
    set(_sdk_root "/usr/local/deltacast")
endif()

set(_use_mock TRUE)
if(_sdk_root AND NOT VIDEOMASTER_USE_MOCK)
    # Prefer vendor CMake package config when available (e.g. /usr/share/deltacast/videomaster/cmake).
    set(_vmhd_config_hints
        "${_sdk_root}"
        "${_sdk_root}/share/deltacast/videomaster"
        "${_sdk_root}/share/deltacast/videomaster/cmake"
        "${_sdk_root}/cmake"
        "/usr/share/deltacast/videomaster"
        "/usr/share/deltacast/videomaster/cmake"
        "/usr/local/cmake")

    find_package(VideoMasterHD CONFIG QUIET
        HINTS ${_vmhd_config_hints}
        NO_MODULE)

    if(VideoMasterHD_FOUND AND TARGET VideoMasterHD::Core)
        set(_use_mock FALSE)
        set(_found_via_config TRUE)
    else()
        set(_found_via_config FALSE)
        set(VideoMasterHD_FOUND FALSE)
        find_path(_vmhd_inc_dir VideoMasterHD_Core.h
            PATH_SUFFIXES videomaster
            PATHS
                "${_sdk_root}"
                "${_sdk_root}/Include"
                "${_sdk_root}/include"
                "/usr/include"
                "/usr/local/include")
        find_library(_vmhd_lib videomasterhd
            PATHS
                "${_sdk_root}/lib"
                "${_sdk_root}/Lib"
                "/usr/lib"
                "/usr/local/lib"
                "/usr/lib/aarch64-linux-gnu")
        if(_vmhd_inc_dir AND _vmhd_lib)
            set(_use_mock FALSE)
        endif()
    endif()
endif()

# ── Target creation ────────────────────────────────────────────────────────
if(_use_mock)
    set(VideoMasterHD_VERSION "6.25.0")
    set(VideoMasterHD_MOCK TRUE)
    set(_mock_dir "${CMAKE_CURRENT_LIST_DIR}/mock/VideoMasterHD")

    if(NOT TARGET _videomasterhd_mock)
        add_library(_videomasterhd_mock STATIC
            "${_mock_dir}/src/mock_sdk.cpp")
        target_include_directories(_videomasterhd_mock PUBLIC
            "${_mock_dir}/include")
        target_compile_definitions(_videomasterhd_mock PUBLIC VIDEOMASTER_HD_MOCK=1)
        target_compile_features(_videomasterhd_mock PUBLIC cxx_std_17)
        set_target_properties(_videomasterhd_mock PROPERTIES
            POSITION_INDEPENDENT_CODE ON)
    endif()
    if(NOT TARGET VideoMasterHD::Core)
        add_library(VideoMasterHD::Core ALIAS _videomasterhd_mock)
    endif()
    if(VideoMasterHD_FIND_REQUIRED)
        message(STATUS
            "VideoMasterHD: using bundled mock (set VideoMaster_SDK_DIR and -DVIDEOMASTER_USE_MOCK=OFF to use the real SDK)")
    endif()
else()
    set(VideoMasterHD_MOCK FALSE)

    # VideoMasterAPIHelper requests find_package(VideoMasterHD 6.25 REQUIRED).
    # When using a real SDK, expose a numeric version (not a label) so CMake's
    # version comparison can evaluate correctly.
    if(NOT VideoMasterHD_VERSION)
        if(DEFINED VideoMasterHD_DIR
           AND EXISTS "${VideoMasterHD_DIR}/VideoMasterHDConfigVersion.cmake")
            unset(PACKAGE_VERSION)
            include("${VideoMasterHD_DIR}/VideoMasterHDConfigVersion.cmake")
            if(PACKAGE_VERSION)
                set(VideoMasterHD_VERSION "${PACKAGE_VERSION}")
            endif()
        endif()

        if(NOT VideoMasterHD_VERSION)
            # Conservative fallback: minimum version required by dependencies.
            set(VideoMasterHD_VERSION "6.25.0")
            message(WARNING "Could not determine VideoMasterHD version from config, falling back to ${VideoMasterHD_VERSION}")
        endif()
    endif()

    if(NOT TARGET VideoMasterHD::Core AND NOT _found_via_config)
        add_library(VideoMasterHD::Core UNKNOWN IMPORTED GLOBAL)
        set_target_properties(VideoMasterHD::Core PROPERTIES
            IMPORTED_LOCATION "${_vmhd_lib}"
            INTERFACE_INCLUDE_DIRECTORIES "${_vmhd_inc_dir}")
    endif()
    message(STATUS "VideoMasterHD: using real SDK at ${_sdk_root}")
endif()

set(VideoMasterHD_FOUND TRUE)

find_package_handle_standard_args(VideoMasterHD
    REQUIRED_VARS VideoMasterHD_FOUND
    VERSION_VAR   VideoMasterHD_VERSION)
