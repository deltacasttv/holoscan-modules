# SPDX-FileCopyrightText: Copyright (c) 2022-2026, DELTACAST.TV. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

include(pybind11_add_holohub_module)

pybind11_add_holohub_module(
    CPP_CMAKE_TARGET videomaster
    CLASS_NAME "VideoMasterSourceOp, VideoMasterTransmitterOp"
    SOURCES videomaster.cpp
)
