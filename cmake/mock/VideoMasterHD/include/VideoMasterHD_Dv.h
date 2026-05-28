/* SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Mock VideoMasterHD DV/HDMI header.
 * Enum/constant declarations + one function declaration; implementation in
 * mock_sdk.cpp.
 */
#pragma once

#include "VideoMasterHD_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── DV standard ─────────────────────────────────────────────────────────── */
typedef enum VHD_DV_STD {
    VHD_DV_STD_SMPTE = 0,
    NB_VHD_DV_STD
} VHD_DV_STD;

/* ── DV stream properties ────────────────────────────────────────────────── */
typedef enum VHD_DV_SP {
    VHD_DV_SP_ACTIVE_WIDTH  = 0x6000,
    VHD_DV_SP_ACTIVE_HEIGHT,
    VHD_DV_SP_INTERLACED,
    VHD_DV_SP_REFRESH_RATE
} VHD_DV_SP;

/* ── DV buffer types ─────────────────────────────────────────────────────── */
typedef enum VHD_DV_BUFFERTYPE {
    VHD_DV_BT_VIDEO = 0,
    NB_VHD_DV_BUFFERTYPE
} VHD_DV_BUFFERTYPE;

/* DV stream processing mode constant */
#define VHD_DV_STPROC_DISJOINED_VIDEO 0

/* Board type — mixed interface (SDI + DV) board detection */
#define VHD_BOARDTYPE_MIXEDINTERFACE  0xFFFFUL

/* ── Function declaration ────────────────────────────────────────────────── */
ULONG VHD_PresetTimingStreamProperties(HANDLE StreamHandle, VHD_DV_STD Standard,
                                       ULONG Width, ULONG Height,
                                       ULONG Framerate, BOOL Interlaced);

#ifdef __cplusplus
}
#endif
