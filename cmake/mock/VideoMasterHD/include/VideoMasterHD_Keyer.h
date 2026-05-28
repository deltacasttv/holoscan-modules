/* SPDX-FileCopyrightText: Copyright (c) 2026 Tom Birdsong / NVIDIA. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Mock VideoMasterHD Keyer header.
 */
#pragma once

#include "VideoMasterHD_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Keyer input selector ────────────────────────────────────────────────── */
typedef enum VHD_KEYERINPUT {
    VHD_KINPUT_RX0 = 0, VHD_KINPUT_RX1, VHD_KINPUT_RX2, VHD_KINPUT_RX3,
    VHD_KINPUT_TX0,     VHD_KINPUT_TX1, VHD_KINPUT_TX2, VHD_KINPUT_TX3,
    NB_VHD_KEYERINPUT
} VHD_KEYERINPUT;

/* ── Keyer output selector ───────────────────────────────────────────────── */
typedef enum VHD_KEYEROUTPUT {
    VHD_KOUTPUT_RX0 = 0, VHD_KOUTPUT_RX1, VHD_KOUTPUT_RX2, VHD_KOUTPUT_RX3,
    VHD_KOUTPUT_KEYER,
    NB_VHD_KEYEROUTPUT
} VHD_KEYEROUTPUT;

/* ── Keyer board properties ──────────────────────────────────────────────── */
typedef enum VHD_KEYER_BOARDPROPERTY {
    /* Per-keyer input selection (0=keyer 0, 1=keyer 1) */
    VHD_KEYER_BP_INPUT_A_0 = 0x3000, VHD_KEYER_BP_INPUT_A_1, VHD_KEYER_BP_INPUT_A,
    VHD_KEYER_BP_INPUT_B_0,           VHD_KEYER_BP_INPUT_B_1, VHD_KEYER_BP_INPUT_B,
    VHD_KEYER_BP_INPUT_K_0,           VHD_KEYER_BP_INPUT_K_1, VHD_KEYER_BP_INPUT_K,
    /* Alpha clip */
    VHD_KEYER_BP_ALPHACLIP_MIN_0, VHD_KEYER_BP_ALPHACLIP_MIN_1, VHD_KEYER_BP_ALPHACLIP_MIN,
    VHD_KEYER_BP_ALPHACLIP_MAX_0, VHD_KEYER_BP_ALPHACLIP_MAX_1, VHD_KEYER_BP_ALPHACLIP_MAX,
    /* Alpha blend factor — also doubles as a "value" constant for the initial factor */
    VHD_KEYER_BP_ALPHABLEND_FACTOR_0, VHD_KEYER_BP_ALPHABLEND_FACTOR_1,
    VHD_KEYER_BP_ALPHABLEND_FACTOR,
    /* Enable */
    VHD_KEYER_BP_ENABLE_0, VHD_KEYER_BP_ENABLE_1, VHD_KEYER_BP_ENABLE,
    /* Video/ANC output routing */
    VHD_KEYER_BP_VIDEOOUTPUT_TX_0, VHD_KEYER_BP_VIDEOOUTPUT_TX_1,
    VHD_KEYER_BP_VIDEOOUTPUT_TX_2, VHD_KEYER_BP_VIDEOOUTPUT_TX_3,
    VHD_KEYER_BP_ANCOUTPUT_TX_0,   VHD_KEYER_BP_ANCOUTPUT_TX_1,
    VHD_KEYER_BP_ANCOUTPUT_TX_2,   VHD_KEYER_BP_ANCOUTPUT_TX_3
} VHD_KEYER_BOARDPROPERTY;

#ifdef __cplusplus
}
#endif
