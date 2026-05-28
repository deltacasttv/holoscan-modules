# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

# Each application is gated on its own APP_<name> option, defaulting to ${BUILD_ALL}.
# Add one add_holohub_application() call per application; declare operator
# dependencies via DEPENDS OPERATORS so they auto-enable when the app is built.
add_holohub_application(deltacast_receiver   DEPENDS OPERATORS deltacast_videomaster)
add_holohub_application(deltacast_transmitter DEPENDS OPERATORS deltacast_videomaster)

# ─────────────────────────────────────────────────────────────────────────────
# Sample data fetch — holoscan_endoscopy_sample_data
# ─────────────────────────────────────────────────────────────────────────────
option(HOLOSCAN_DELTACAST_DOWNLOAD_DATA
       "Download holoscan_endoscopy_sample_data for the deltacast_transmitter demo" ON)

if(HOLOSCAN_DELTACAST_DOWNLOAD_DATA)
    if(NOT DEFINED HOLOHUB_DATA_DIR)
        set(HOLOHUB_DATA_DIR "${CMAKE_SOURCE_DIR}/data"
            CACHE PATH "Directory for downloaded sample data")
    endif()

    # holoscan_download_data() wraps the HoloHub `download_ngc_data` bash script
    # (resolved via find_program; ships in .holohub/utilities/ after
    # ./holohub bootstrap, or at /opt/nvidia/holoscan/bin in an SDK install).
    # The script handles the NGC fetch and the ffmpeg → convert_video_to_gxf_entities
    # transcode internally, so no inline shell-pipe gymnastics here.
    include(holohub_download_data)
    holoscan_download_data(endoscopy
        URL nvidia/clara-holoscan/holoscan_endoscopy_sample_data:20230222
        DOWNLOAD_NAME surgical_video
        DOWNLOAD_DIR ${HOLOHUB_DATA_DIR}
        GENERATE_GXF_ENTITIES
        GXF_ENTITIES_WIDTH 854
        GXF_ENTITIES_HEIGHT 480
        GXF_ENTITIES_CHANNELS 3
        GXF_ENTITIES_FRAMERATE 30
        ALL
    )
endif()
