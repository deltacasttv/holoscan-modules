/* SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Mock VideoMasterHD Core header.
 *
 * Declarations only — implementations live in mock_sdk.cpp and are linked
 * into the VideoMasterHD::Core target by the bundled FindVideoMasterHD.cmake.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Basic types ─────────────────────────────────────────────────────────── */
typedef void*          HANDLE;
typedef unsigned long  ULONG;
typedef unsigned char  BYTE;
typedef int            BOOL;
typedef unsigned int   BOOL32;
#ifndef TRUE
#  define TRUE  1
#endif
#ifndef FALSE
#  define FALSE 0
#endif
#ifndef NULL
#  define NULL ((void*)0)
#endif

/* ── Error codes ─────────────────────────────────────────────────────────── */
typedef enum VHD_ERRORCODE {
    VHDERR_NOERROR                                       = 0,
    VHDERR_FATALERROR                                    = 1,
    VHDERR_OPERATIONFAILED                               = 2,
    VHDERR_NOTENOUGHRESOURCE                             = 3,
    VHDERR_NOTIMPLEMENTED                                = 4,
    VHDERR_NOTFOUND                                      = 5,
    VHDERR_BADARG                                        = 6,
    VHDERR_INVALIDPOINTER                                = 7,
    VHDERR_INVALIDHANDLE                                 = 8,
    VHDERR_INVALIDPROPERTY                               = 9,
    VHDERR_INVALIDSTREAM                                 = 10,
    VHDERR_RESOURCELOCKED                                = 11,
    VHDERR_BOARDNOTPRESENT                               = 12,
    VHDERR_INCOHERENTBOARDSTATE                          = 13,
    VHDERR_INCOHERENTDRIVERSTATE                         = 14,
    VHDERR_INCOHERENTLIBSTATE                            = 15,
    VHDERR_SETUPLOCKED                                   = 16,
    VHDERR_CHANNELUSED                                   = 17,
    VHDERR_STREAMUSED                                    = 18,
    VHDERR_READONLYPROPERTY                              = 19,
    VHDERR_OFFLINEPROPERTY                               = 20,
    VHDERR_TXPROPERTY                                    = 21,
    VHDERR_TIMEOUT                                       = 22,
    VHDERR_STREAMNOTRUNNING                              = 23,
    VHDERR_BADINPUTSIGNAL                                = 24,
    VHDERR_BADREFERENCESIGNAL                            = 25,
    VHDERR_FRAMELOCKED                                   = 26,
    VHDERR_FRAMEUNLOCKED                                 = 27,
    VHDERR_INCOMPATIBLESYSTEM                            = 28,
    VHDERR_ANCLINEISEMPTY                                = 29,
    VHDERR_ANCLINEISFULL                                 = 30,
    VHDERR_BUFFERTOOSMALL                                = 31,
    VHDERR_BADANC                                        = 32,
    VHDERR_BADCONFIG                                     = 33,
    VHDERR_FIRMWAREMISMATCH                              = 34,
    VHDERR_LIBRARYMISMATCH                               = 35,
    VHDERR_FAILSAFE                                      = 36,
    VHDERR_RXPROPERTY                                    = 37,
    VHDERR_ALREADYINITIALIZED                            = 38,
    VHDERR_NOTINITIALIZED                                = 39,
    VHDERR_CROSSTHREAD                                   = 40,
    VHDERR_INCOHERENTDATA                                = 41,
    VHDERR_BADSIZE                                       = 42,
    VHDERR_WAKEUP                                        = 43,
    VHDERR_DEVICE_REMOVED                                = 44,
    VHDERR_DATANOTREADY                                  = 45,
    VHDERR_NOSFPMODULE_DEPRECATED                        = 46,
    VHDERR_SFPMODULELOCKED_DEPRECATED                    = 47,
    VHDERR_INVALIDTABLE_DEPRECATED                       = 48,
    VHDERR_TEMPERATURETOOHIGH                            = 49,
    VHDERR_LTCSOURCEUNLOCKED                             = 50,
    VHDERR_INVALIDACCESSRIGHT                            = 51,
    VHDERR_LICENSERESTRICTION_DEPRECATED                 = 52,
    VHDERR_SOFTWAREPROTECTION_FAILURE_DEPRECATED         = 53,
    VHDERR_SOFTWAREPROTECTION_IDNOTFOUND_DEPRECATED      = 54,
    VHDERR_SOFTWAREPROTECTION_BADLICENSEINFO_DEPRECATED  = 55,
    VHDERR_SOFTWAREPROTECTION_UNAUTHORIZEDHOST_DEPRECATED = 56,
    VHDERR_SOFTWAREPROTECTION_STREAMSTARTED_DEPRECATED   = 57,
    VHDERR_BAD_PARITY                                    = 58,
    VHDERR_INVALIDCAPABILITY                             = 59,
    VHDERR_DEPRECATED                                    = 60,
    VHDERR_DEVICENEEDSRESTART                            = 61,
    VHDERR_FWUPDATEINPROGRESS                            = 62,
    VHDERR_NOMOREDATATOPROCESS                           = 63,
    VHDERR_NOTSUPPORTED                                  = 64,
    NB_VHD_ERRORCODE
} VHD_ERRORCODE;

/* ── Stream types ────────────────────────────────────────────────────────── */
typedef enum VHD_STREAMTYPE {
    VHD_ST_RX0  = 0,  VHD_ST_RX1,  VHD_ST_RX2,  VHD_ST_RX3,
    VHD_ST_RX4,       VHD_ST_RX5,  VHD_ST_RX6,  VHD_ST_RX7,
    VHD_ST_RX8,       VHD_ST_RX9,  VHD_ST_RX10, VHD_ST_RX11,
    VHD_ST_TX0  = 12, VHD_ST_TX1,  VHD_ST_TX2,  VHD_ST_TX3,
    VHD_ST_TX4,       VHD_ST_TX5,  VHD_ST_TX6,  VHD_ST_TX7,
    VHD_ST_TX8,       VHD_ST_TX9,  VHD_ST_TX10, VHD_ST_TX11,
    NB_VHD_STREAMTYPE
} VHD_STREAMTYPE;

/* ── Channel types ───────────────────────────────────────────────────────── */
typedef enum VHD_CHANNELTYPE {
    VHD_CHNTYPE_HDSDI       = 0,
    VHD_CHNTYPE_3GSDI       = 1,
    VHD_CHNTYPE_12GSDI      = 2,
    VHD_CHNTYPE_HDMI        = 3,
    VHD_CHNTYPE_DISPLAYPORT = 4,
    NB_VHD_CHANNELTYPE
} VHD_CHANNELTYPE;

/* ── Genlock sources ─────────────────────────────────────────────────────── */
typedef enum VHD_GENLOCKSOURCE {
    VHD_GENLOCK_RX0  = 0, VHD_GENLOCK_RX1,  VHD_GENLOCK_RX2,  VHD_GENLOCK_RX3,
    VHD_GENLOCK_RX4,      VHD_GENLOCK_RX5,  VHD_GENLOCK_RX6,  VHD_GENLOCK_RX7,
    VHD_GENLOCK_RX8,      VHD_GENLOCK_RX9,  VHD_GENLOCK_RX10, VHD_GENLOCK_RX11,
    VHD_GENLOCK_LOCAL
} VHD_GENLOCKSOURCE;

/* ── Buffer packing ──────────────────────────────────────────────────────── */
typedef enum VHD_BUFPACK {
    VHD_BUFPACK_VIDEO_RGB_32  = 0,
    VHD_BUFPACK_VIDEO_RGBA_32 = 1
} VHD_BUFPACK;

/* ── Core board properties ───────────────────────────────────────────────── */
typedef enum VHD_CORE_BOARDPROPERTY {
    VHD_CORE_BP_RX0_TYPE  = 0x1000, VHD_CORE_BP_RX1_TYPE,  VHD_CORE_BP_RX2_TYPE,
    VHD_CORE_BP_RX3_TYPE,            VHD_CORE_BP_RX4_TYPE,  VHD_CORE_BP_RX5_TYPE,
    VHD_CORE_BP_RX6_TYPE,            VHD_CORE_BP_RX7_TYPE,  VHD_CORE_BP_RX8_TYPE,
    VHD_CORE_BP_RX9_TYPE,            VHD_CORE_BP_RX10_TYPE, VHD_CORE_BP_RX11_TYPE,
    VHD_CORE_BP_TX0_TYPE,  VHD_CORE_BP_TX1_TYPE,  VHD_CORE_BP_TX2_TYPE,
    VHD_CORE_BP_TX3_TYPE,  VHD_CORE_BP_TX4_TYPE,  VHD_CORE_BP_TX5_TYPE,
    VHD_CORE_BP_TX6_TYPE,  VHD_CORE_BP_TX7_TYPE,  VHD_CORE_BP_TX8_TYPE,
    VHD_CORE_BP_TX9_TYPE,  VHD_CORE_BP_TX10_TYPE, VHD_CORE_BP_TX11_TYPE,
    VHD_CORE_BP_RX0_STATUS,  VHD_CORE_BP_RX1_STATUS,  VHD_CORE_BP_RX2_STATUS,
    VHD_CORE_BP_RX3_STATUS,  VHD_CORE_BP_RX4_STATUS,  VHD_CORE_BP_RX5_STATUS,
    VHD_CORE_BP_RX6_STATUS,  VHD_CORE_BP_RX7_STATUS,  VHD_CORE_BP_RX8_STATUS,
    VHD_CORE_BP_RX9_STATUS,  VHD_CORE_BP_RX10_STATUS, VHD_CORE_BP_RX11_STATUS,
    VHD_CORE_BP_BYPASS_RELAY_0, VHD_CORE_BP_BYPASS_RELAY_1,
    VHD_CORE_BP_BYPASS_RELAY_2, VHD_CORE_BP_BYPASS_RELAY_3,
    VHD_CORE_BP_ACTIVE_LOOPBACK_0,
    VHD_CORE_BP_FIRMWARE_LOOPBACK_0, VHD_CORE_BP_FIRMWARE_LOOPBACK_1,
    VHD_CORE_BP_BOARD_TYPE
} VHD_CORE_BOARDPROPERTY;

/* ── Core stream properties ──────────────────────────────────────────────── */
typedef enum VHD_CORE_SP {
    VHD_CORE_SP_BUFFER_PACKING = 0x4000,
    VHD_CORE_SP_FIELD_MERGE,
    VHD_CORE_SP_TX_SYNC_ENABLE
} VHD_CORE_SP;

/* ── Board capability flags ──────────────────────────────────────────────── */
#define VHD_CORE_BOARD_CAP_PASSIVE_LOOPBACK  0x00000001UL
#define VHD_CORE_BOARD_CAP_ACTIVE_LOOPBACK   0x00000002UL
#define VHD_CORE_BOARD_CAP_FIRMWARE_LOOPBACK 0x00000004UL

/* ── RX status flags ─────────────────────────────────────────────────────── */
#define VHD_CORE_RXSTS_UNLOCKED 0x00000001UL

/* ── Function declarations ───────────────────────────────────────────────── */
ULONG       VHD_GetApiInfo(HANDLE* pApiHandle, ULONG* pNbBoards);
const char* VHD_GetStringVersion(void);

ULONG VHD_OpenBoardHandle(ULONG BoardIndex, HANDLE* pBoardHandle,
                          void* pUnused, ULONG Unused);
ULONG VHD_CloseBoardHandle(HANDLE BoardHandle);

ULONG VHD_OpenStreamHandle(HANDLE BoardHandle, VHD_STREAMTYPE StreamType,
                           ULONG StreamProc, void* pUnused,
                           HANDLE* pStreamHandle, void* pUnused2);
ULONG VHD_CloseStreamHandle(HANDLE StreamHandle);
ULONG VHD_StartStream(HANDLE StreamHandle);
ULONG VHD_StopStream(HANDLE StreamHandle);

ULONG VHD_GetBoardProperty(HANDLE BoardHandle, ULONG Property, ULONG* pValue);
ULONG VHD_SetBoardProperty(HANDLE BoardHandle, ULONG Property, ULONG Value);
ULONG VHD_GetBoardCapability(HANDLE BoardHandle, ULONG Capability, ULONG* pValue);

ULONG VHD_GetStreamProperty(HANDLE StreamHandle, ULONG Property, ULONG* pValue);
ULONG VHD_SetStreamProperty(HANDLE StreamHandle, ULONG Property, ULONG Value);

ULONG VHD_GetVideoCharacteristics(ULONG VideoStandard,
                                  ULONG* pWidth, ULONG* pHeight,
                                  BOOL32* pInterlaced, ULONG* pFramerate);

#ifdef __cplusplus
}
#endif
