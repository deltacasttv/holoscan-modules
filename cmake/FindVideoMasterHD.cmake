# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# FindVideoMasterHD.cmake
#
# Locates the Deltacast VideoMaster HD SDK and exposes the VideoMasterHD::Core
# imported target. Falls back to a bundled mock when VIDEOMASTER_SDK_DIR is
# not set (or when VIDEOMASTER_USE_MOCK is explicitly set to ON), which is
# enough to build *and run* the operator and sample apps without hardware.
#
# Result variables
#   VideoMasterHD_FOUND       - TRUE when the target is available
#   VideoMasterHD_VERSION     - SDK version string
#   VideoMasterHD_MOCK        - TRUE when the mock is active
#
# Cache variables
#   VIDEOMASTER_SDK_DIR       - Root of the real SDK installation
#   VIDEOMASTER_USE_MOCK      - Force the mock regardless of SDK presence

include(FindPackageHandleStandardArgs)

# Avoid re-running SDK discovery when another subproject already resolved
# VideoMasterHD::Core in the same configure pass.
if(TARGET VideoMasterHD::Core)
    if(NOT DEFINED VideoMasterHD_MOCK)
        get_target_property(_vmhd_core_imported VideoMasterHD::Core IMPORTED)
        if(_vmhd_core_imported)
            set(VideoMasterHD_MOCK FALSE)
        else()
            set(VideoMasterHD_MOCK TRUE)
        endif()
    endif()

    if(NOT VideoMasterHD_VERSION)
        if(VideoMasterHD_MOCK)
             set(VideoMasterHD_VERSION "6.25.0")
        else()
            if(DEFINED VideoMasterHD_DIR
            AND EXISTS "${VideoMasterHD_DIR}/VideoMasterHDConfigVersion.cmake")
                unset(PACKAGE_VERSION)
                include("${VideoMasterHD_DIR}/VideoMasterHDConfigVersion.cmake")
                if(PACKAGE_VERSION)
                    set(VideoMasterHD_VERSION "${PACKAGE_VERSION}")
                endif()
            endif()

            if(NOT VideoMasterHD_VERSION)
                set(VideoMasterHD_VERSION "6.25.0")
            endif()
        endif()
    endif()

    set(VideoMasterHD_FOUND TRUE)
    find_package_handle_standard_args(VideoMasterHD
        REQUIRED_VARS VideoMasterHD_FOUND
        VERSION_VAR   VideoMasterHD_VERSION)
    return()
endif()

option(VIDEOMASTER_USE_MOCK
    "Use the bundled VideoMasterHD mock instead of the real SDK (enables builds and demo runs without hardware)"
    ON)

set(_use_mock TRUE)
set(_found_via_config FALSE)
set(_sdk_report "")

set(_sdk_root "")
if(VIDEOMASTER_SDK_DIR)
    set(_sdk_root "${VIDEOMASTER_SDK_DIR}")
endif()

if(NOT VIDEOMASTER_USE_MOCK)
    if(_sdk_root)
        message(STATUS "VideoMasterHD: looking for SDK at ${_sdk_root} (set VIDEOMASTER_SDK_DIR to override)")

        unset(_vmhd_inc_dir_sdk)
        unset(_vmhd_inc_dir_sdk CACHE)
        unset(_vmhd_lib_sdk)
        unset(_vmhd_lib_sdk CACHE)
        find_path(_vmhd_inc_dir_sdk VideoMasterHD_Core.h
            PATHS 
                "${_sdk_root}/headers"
                "${_sdk_root}/include"
                "${_sdk_root}"
            NO_DEFAULT_PATH)
        find_library(_vmhd_lib_sdk videomasterhd
            PATHS
                "${_sdk_root}/lib"
                "${_sdk_root}"
            NO_DEFAULT_PATH)

        if(_vmhd_inc_dir_sdk AND _vmhd_lib_sdk)
            message(STATUS "VideoMasterHD: found SDK at ${_sdk_root} (set VIDEOMASTER_SDK_DIR to override)")
            set(_vmhd_inc_dir "${_vmhd_inc_dir_sdk}")
            set(_vmhd_lib "${_vmhd_lib_sdk}")
            set(_use_mock FALSE)
            set(_sdk_report "${_sdk_root} (direct lib/headers)")
        else()
            message(WARNING "VideoMasterHD: no SDK found at ${_sdk_root}, trying config mode (set VIDEOMASTER_SDK_DIR to override)")
            set(VideoMasterHD_FOUND FALSE)
            find_package(VideoMasterHD CONFIG QUIET
                HINTS
                    "${_sdk_root}"
                    "${_sdk_root}/cmake"
                NO_MODULE
                NO_DEFAULT_PATH)
            if(VideoMasterHD_FOUND AND TARGET VideoMasterHD::Core)
                message(STATUS "VideoMasterHD: found SDK via config at ${VideoMasterHD_DIR}")
                set(_use_mock FALSE)
                set(_found_via_config TRUE)
                set(_sdk_report "${VideoMasterHD_DIR}")
            else()
                message(WARNING "VideoMasterHD: no SDK found at ${_sdk_root} via config, trying other methods (set VIDEOMASTER_SDK_DIR to override)")
            endif()
        endif()
    endif()

    if(_use_mock)
        message(STATUS "VideoMasterHD: looking for SDK via config (set VIDEOMASTER_SDK_DIR to override)")
        set(VideoMasterHD_FOUND FALSE)
        find_package(VideoMasterHD CONFIG QUIET
            HINTS "/usr/local/cmake"
            NO_MODULE
            NO_DEFAULT_PATH)
        if(VideoMasterHD_FOUND AND TARGET VideoMasterHD::Core)
            message(STATUS "VideoMasterHD: found SDK via config at ${VideoMasterHD_DIR}")
            set(_use_mock FALSE)
            set(_found_via_config TRUE)
            set(_sdk_report "${VideoMasterHD_DIR}")
        else()
            message(WARNING "VideoMasterHD: no SDK found via config, trying other methods (set VIDEOMASTER_SDK_DIR to override)")
        endif()
    endif()

    if(_use_mock)
        message(STATUS "VideoMasterHD: looking for SDK via deltacast share config (set VIDEOMASTER_SDK_DIR to override)")
        set(VideoMasterHD_FOUND FALSE)
        find_package(VideoMasterHD CONFIG QUIET
            HINTS "/usr/share/deltacast/videomaster/cmake"
            NO_MODULE
            NO_DEFAULT_PATH)
        if(VideoMasterHD_FOUND AND TARGET VideoMasterHD::Core)
            message(STATUS "VideoMasterHD: found SDK via config at ${VideoMasterHD_DIR}")
            set(_use_mock FALSE)
            set(_found_via_config TRUE)
            set(_sdk_report "${VideoMasterHD_DIR}")
        else()
            message(WARNING "VideoMasterHD: no SDK found via deltacast share config, trying other methods (set VIDEOMASTER_SDK_DIR to override)")
        endif()
    endif()

    if(_use_mock)
        message(STATUS "VideoMasterHD: looking for SDK via classic system layout fallback (set VIDEOMASTER_SDK_DIR to override)")
        unset(_vmhd_inc_dir_usr)
        unset(_vmhd_inc_dir_usr CACHE)
        unset(_vmhd_lib_usr)
        unset(_vmhd_lib_usr CACHE)
        find_path(_vmhd_inc_dir_usr VideoMasterHD_Core.h
            PATHS "/usr/include/videomaster"
            NO_DEFAULT_PATH)
        find_library(_vmhd_lib_usr videomasterhd
            PATHS "/usr/lib"
            NO_DEFAULT_PATH)

        if(_vmhd_inc_dir_usr AND _vmhd_lib_usr)
            message(STATUS "VideoMasterHD: found SDK via classic system layout at /usr")
            set(_vmhd_inc_dir "${_vmhd_inc_dir_usr}")
            set(_vmhd_lib "${_vmhd_lib_usr}")
            set(_use_mock FALSE)
            set(_sdk_report "/usr (fallback lib/include)")
        else()
            message(WARNING "VideoMasterHD: no SDK found via classic system layout fallback, using mock (set VIDEOMASTER_SDK_DIR and -DVIDEOMASTER_USE_MOCK=OFF to use the real SDK)")
        endif()
    endif()
endif()

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
            "VideoMasterHD: using bundled mock (set VIDEOMASTER_SDK_DIR and -DVIDEOMASTER_USE_MOCK=OFF to use the real SDK)")
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
    if(_found_via_config)
        message(STATUS "VideoMasterHD: using real SDK from config at ${_sdk_report}")
    else()
        message(STATUS "VideoMasterHD: using real SDK via fallback lookup (${_sdk_report})")
    endif()
endif()

set(VideoMasterHD_FOUND TRUE)

find_package_handle_standard_args(VideoMasterHD
    REQUIRED_VARS VideoMasterHD_FOUND
    VERSION_VAR   VideoMasterHD_VERSION)
