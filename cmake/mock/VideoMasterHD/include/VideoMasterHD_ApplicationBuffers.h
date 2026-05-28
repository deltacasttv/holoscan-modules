/* SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Mock VideoMasterHD Application Buffers header.
 * Declarations only — see mock_sdk.cpp for implementations.
 */
#pragma once

#include "VideoMasterHD_Core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Application buffer descriptor ──────────────────────────────────────── */
typedef struct VHD_APPLICATION_BUFFER_DESCRIPTOR {
    ULONG  Size;        /* sizeof this struct */
    BYTE*  pBuffer;     /* caller-allocated buffer */
    BOOL   RDMAEnabled; /* TRUE for GPUDirect path */
} VHD_APPLICATION_BUFFER_DESCRIPTOR;

/* ── Function declarations ───────────────────────────────────────────────── */
ULONG VHD_InitApplicationBuffers(HANDLE StreamHandle);

ULONG VHD_GetApplicationBuffersSize(HANDLE StreamHandle, int BufferType,
                                    ULONG* pSize);

ULONG VHD_CreateSlotEx(HANDLE StreamHandle,
                       VHD_APPLICATION_BUFFER_DESCRIPTOR* pDescriptors,
                       HANDLE* pSlotHandle);

ULONG VHD_GetSlotBuffer(HANDLE SlotHandle, ULONG BufferType,
                        BYTE** ppBuffer, ULONG* pSize);

ULONG VHD_QueueInSlot(HANDLE SlotHandle);
ULONG VHD_QueueOutSlot(HANDLE SlotHandle);

ULONG VHD_WaitSlotFilled(HANDLE StreamHandle, HANDLE* pSlotHandle,
                         ULONG TimeoutMs);
ULONG VHD_WaitSlotSent(HANDLE StreamHandle, HANDLE* pSlotHandle,
                       ULONG TimeoutMs);

#ifdef __cplusplus
}
#endif
