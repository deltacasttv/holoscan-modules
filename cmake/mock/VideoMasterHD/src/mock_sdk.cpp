// SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// Mock VideoMasterHD SDK implementation.
//
// Backs the `VideoMasterHD::Core` target exposed by cmake/FindVideoMasterHD.cmake
// when the user opts into the bundled mock (-DVIDEOMASTER_USE_MOCK=ON, the
// default). The goal is *functional* — enough state and behaviour that the
// VideoMasterSourceOp / VideoMasterTransmitterOp operators (and the sample
// deltacast_receiver / deltacast_transmitter apps) drive end-to-end without
// real hardware. We synthesize an animated RGBA gradient as the "captured"
// signal for RX, and silently accept frames for TX.

#include "VideoMasterHD_Core.h"
#include "VideoMasterHD_ApplicationBuffers.h"
#include "VideoMasterHD_Dv.h"
#include "VideoMasterHD_Keyer.h"
#include "VideoMasterHD_Sdi.h"
#include "VideoMasterHD_String.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <deque>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace {

struct MockBoard {
    std::unordered_map<uint32_t, uint32_t> props;
};

struct MockStream {
    VHD_STREAMTYPE type = VHD_ST_RX0;
    // Defaults match what both deltacast_{receiver,transmitter} YAMLs request
    // (1080p30). When the receiver reads back the stream's properties, the
    // api-helper synthesises a VideoFormat that the operator compares against
    // its YAML config — a mismatch throws "Input signal does not match
    // configuration", so this default has to line up.
    uint32_t width = 1920;
    uint32_t height = 1080;
    uint32_t framerate = 30;
    bool progressive = true;
    std::unordered_map<uint32_t, uint32_t> stream_props;
    std::deque<uintptr_t> rx_queue;   // slots waiting for VHD_WaitSlotFilled
    std::deque<uintptr_t> tx_queue;   // slots waiting for VHD_WaitSlotSent
    uint64_t frame_count = 0;
    std::chrono::steady_clock::time_point next_frame_at{};
};

struct MockSlot {
    void* buffer = nullptr;
    uint32_t size = 0;
    uintptr_t stream_id = 0;
};

std::mutex g_mu;
std::atomic<uintptr_t> g_next_handle{1};
std::unordered_map<uintptr_t, MockBoard>  g_boards;
std::unordered_map<uintptr_t, MockStream> g_streams;
std::unordered_map<uintptr_t, MockSlot>   g_slots;

inline uintptr_t  id(HANDLE h) { return reinterpret_cast<uintptr_t>(h); }
inline HANDLE     to_handle(uintptr_t i) { return reinterpret_cast<HANDLE>(i); }

// Fill an RGBA32 / RGBX32 buffer with an animated gradient that shifts each
// frame so callers can visually confirm distinct frames are flowing.
void fill_pattern_locked(MockStream& s, void* buf, uint32_t size) {
    if (!buf || size == 0) return;
    auto* bytes = static_cast<uint8_t*>(buf);
    const uint32_t bpp = 4;
    const uint32_t stride = s.width * bpp;
    const uint32_t rows = std::min<uint32_t>(s.height, size / stride);
    const uint8_t  shift = static_cast<uint8_t>(s.frame_count & 0xFF);
    for (uint32_t y = 0; y < rows; ++y) {
        uint8_t* row = bytes + y * stride;
        for (uint32_t x = 0; x < s.width; ++x) {
            uint8_t* px = row + x * bpp;
            px[0] = static_cast<uint8_t>((x + shift) & 0xFF);
            px[1] = static_cast<uint8_t>((y - shift) & 0xFF);
            px[2] = static_cast<uint8_t>(((x ^ y) + shift) & 0xFF);
            px[3] = 0xFF;
        }
    }
}

}  // namespace

extern "C" {

// ── API info ────────────────────────────────────────────────────────────────
ULONG VHD_GetApiInfo(HANDLE* pApiHandle, ULONG* pNbBoards) {
    if (pApiHandle) *pApiHandle = nullptr;
    if (pNbBoards)  *pNbBoards  = 1;
    return VHDERR_NOERROR;
}

const char* VHD_GetStringVersion(void) {
    return "6.25.0-mock";
}

// ── Board lifecycle ─────────────────────────────────────────────────────────
ULONG VHD_OpenBoardHandle(ULONG BoardIndex, HANDLE* pBoardHandle,
                          void* /*pUnused*/, ULONG /*Unused*/) {
    (void)BoardIndex;
    if (!pBoardHandle) return VHDERR_INVALIDPOINTER;
    std::lock_guard<std::mutex> lk(g_mu);
    const auto h = g_next_handle.fetch_add(1);
    g_boards[h] = MockBoard{};
    *pBoardHandle = to_handle(h);
    return VHDERR_NOERROR;
}

ULONG VHD_CloseBoardHandle(HANDLE BoardHandle) {
    std::lock_guard<std::mutex> lk(g_mu);
    g_boards.erase(id(BoardHandle));
    return VHDERR_NOERROR;
}

// ── Board properties ────────────────────────────────────────────────────────
ULONG VHD_GetBoardProperty(HANDLE BoardHandle, ULONG Property, ULONG* pValue) {
    if (!pValue) return VHDERR_INVALIDPOINTER;
    *pValue = 0;
    // Channel-type queries: pretend every RX/TX is a 3G-SDI port so the
    // operator picks the SdiVideoInformation branch.
    if (Property >= VHD_CORE_BP_RX0_TYPE && Property <= VHD_CORE_BP_TX11_TYPE) {
        *pValue = VHD_CHNTYPE_3GSDI;
        return VHDERR_NOERROR;
    }
    // Status queries: 0 means "signal locked" (the operator checks
    // `status & VHD_CORE_RXSTS_UNLOCKED`).
    if (Property >= VHD_CORE_BP_RX0_STATUS && Property <= VHD_CORE_BP_RX11_STATUS) {
        *pValue = 0;
        return VHDERR_NOERROR;
    }
    // SDI configure_sync queries — return a defensible default so the call
    // chain doesn't bail with an error.
    if (Property == VHD_SDI_BP_RX0_STANDARD || Property == VHD_SDI_BP_RX1_STANDARD ||
        Property == VHD_SDI_BP_RX2_STANDARD || Property == VHD_SDI_BP_RX3_STANDARD ||
        Property == VHD_SDI_BP_RX4_STANDARD || Property == VHD_SDI_BP_RX5_STANDARD ||
        Property == VHD_SDI_BP_RX6_STANDARD || Property == VHD_SDI_BP_RX7_STANDARD ||
        Property == VHD_SDI_BP_RX8_STANDARD || Property == VHD_SDI_BP_RX9_STANDARD ||
        Property == VHD_SDI_BP_RX10_STANDARD || Property == VHD_SDI_BP_RX11_STANDARD) {
        *pValue = VHD_VIDEOSTD_S274M_1080p_60Hz;
        return VHDERR_NOERROR;
    }
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_boards.find(id(BoardHandle));
    if (it != g_boards.end()) {
        auto pit = it->second.props.find(Property);
        if (pit != it->second.props.end()) *pValue = pit->second;
    }
    return VHDERR_NOERROR;
}

ULONG VHD_SetBoardProperty(HANDLE BoardHandle, ULONG Property, ULONG Value) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_boards.find(id(BoardHandle));
    if (it != g_boards.end()) it->second.props[Property] = Value;
    return VHDERR_NOERROR;
}

ULONG VHD_GetBoardCapability(HANDLE /*BoardHandle*/, ULONG /*Capability*/, ULONG* pValue) {
    // Report no loopback capability so the operator's set_loopback_state()
    // path early-returns without touching loopback properties.
    if (pValue) *pValue = 0;
    return VHDERR_NOERROR;
}

// ── Stream lifecycle ────────────────────────────────────────────────────────
ULONG VHD_OpenStreamHandle(HANDLE /*BoardHandle*/, VHD_STREAMTYPE StreamType,
                           ULONG /*StreamProc*/, void* /*pUnused*/,
                           HANDLE* pStreamHandle, void* /*pUnused2*/) {
    if (!pStreamHandle) return VHDERR_INVALIDPOINTER;
    std::lock_guard<std::mutex> lk(g_mu);
    const auto h = g_next_handle.fetch_add(1);
    MockStream s;
    s.type = StreamType;
    g_streams.emplace(h, std::move(s));
    *pStreamHandle = to_handle(h);
    return VHDERR_NOERROR;
}

ULONG VHD_CloseStreamHandle(HANDLE StreamHandle) {
    std::lock_guard<std::mutex> lk(g_mu);
    const auto sid = id(StreamHandle);
    for (auto it = g_slots.begin(); it != g_slots.end(); ) {
        if (it->second.stream_id == sid) it = g_slots.erase(it);
        else ++it;
    }
    g_streams.erase(sid);
    return VHDERR_NOERROR;
}

ULONG VHD_StartStream(HANDLE StreamHandle) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(id(StreamHandle));
    if (it == g_streams.end()) return VHDERR_INVALIDHANDLE;
    it->second.next_frame_at = std::chrono::steady_clock::now();
    return VHDERR_NOERROR;
}

ULONG VHD_StopStream(HANDLE /*StreamHandle*/) {
    return VHDERR_NOERROR;
}

// ── Stream properties ──────────────────────────────────────────────────────
ULONG VHD_GetStreamProperty(HANDLE StreamHandle, ULONG Property, ULONG* pValue) {
    if (!pValue) return VHDERR_INVALIDPOINTER;
    *pValue = 0;
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(id(StreamHandle));
    if (it == g_streams.end()) return VHDERR_NOERROR;
    // Pre-populated defaults the api-helper reads to assemble VideoFormat.
    // 1080p30 matches what both sample YAML configs request.
    switch (Property) {
        case VHD_SDI_SP_VIDEO_STANDARD: *pValue = VHD_VIDEOSTD_S274M_1080p_30Hz; return VHDERR_NOERROR;
        case VHD_SDI_SP_INTERFACE:      *pValue = VHD_INTERFACE_3G_A_425_1;      return VHDERR_NOERROR;
        case VHD_SDI_SP_CLOCK_SYSTEM:   *pValue = VHD_CLOCKDIV_1;                return VHDERR_NOERROR;
        case VHD_DV_SP_ACTIVE_WIDTH:    *pValue = it->second.width;              return VHDERR_NOERROR;
        case VHD_DV_SP_ACTIVE_HEIGHT:   *pValue = it->second.height;             return VHDERR_NOERROR;
        case VHD_DV_SP_INTERLACED:      *pValue = it->second.progressive ? 0 : 1; return VHDERR_NOERROR;
        case VHD_DV_SP_REFRESH_RATE:    *pValue = it->second.framerate;          return VHDERR_NOERROR;
        default: break;
    }
    auto pit = it->second.stream_props.find(Property);
    if (pit != it->second.stream_props.end()) *pValue = pit->second;
    return VHDERR_NOERROR;
}

ULONG VHD_SetStreamProperty(HANDLE StreamHandle, ULONG Property, ULONG Value) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(id(StreamHandle));
    if (it == g_streams.end()) return VHDERR_NOERROR;
    it->second.stream_props[Property] = Value;
    // DV path sets width/height/refresh/interlaced via SetStreamProperty too.
    switch (Property) {
        case VHD_DV_SP_ACTIVE_WIDTH:  it->second.width = Value; break;
        case VHD_DV_SP_ACTIVE_HEIGHT: it->second.height = Value; break;
        case VHD_DV_SP_REFRESH_RATE:  it->second.framerate = Value; break;
        case VHD_DV_SP_INTERLACED:    it->second.progressive = !Value; break;
        default: break;
    }
    return VHDERR_NOERROR;
}

// ── Video standard lookup ──────────────────────────────────────────────────
// SdiVideoInformation::set_video_format iterates every standard and asks for
// its (w, h, interlaced, fr) to find a match. We answer just enough of those
// queries that 1080p60 maps cleanly; everything else returns 0×0 so the
// iteration moves on.
ULONG VHD_GetVideoCharacteristics(ULONG VideoStandard,
                                  ULONG* pWidth, ULONG* pHeight,
                                  BOOL32* pInterlaced, ULONG* pFramerate) {
    if (!pWidth || !pHeight || !pInterlaced || !pFramerate) return VHDERR_INVALIDPOINTER;
    *pWidth = 0; *pHeight = 0; *pInterlaced = 0; *pFramerate = 0;
    switch (VideoStandard) {
        case VHD_VIDEOSTD_S274M_1080p_60Hz: *pWidth=1920; *pHeight=1080; *pFramerate=60; break;
        case VHD_VIDEOSTD_S274M_1080p_50Hz: *pWidth=1920; *pHeight=1080; *pFramerate=50; break;
        case VHD_VIDEOSTD_S274M_1080p_30Hz: *pWidth=1920; *pHeight=1080; *pFramerate=30; break;
        case VHD_VIDEOSTD_S274M_1080p_25Hz: *pWidth=1920; *pHeight=1080; *pFramerate=25; break;
        case VHD_VIDEOSTD_S274M_1080p_24Hz: *pWidth=1920; *pHeight=1080; *pFramerate=24; break;
        case VHD_VIDEOSTD_S274M_1080i_60Hz: *pWidth=1920; *pHeight=1080; *pFramerate=60; *pInterlaced=1; break;
        case VHD_VIDEOSTD_S274M_1080i_50Hz: *pWidth=1920; *pHeight=1080; *pFramerate=50; *pInterlaced=1; break;
        case VHD_VIDEOSTD_S296M_720p_60Hz:  *pWidth=1280; *pHeight=720;  *pFramerate=60; break;
        case VHD_VIDEOSTD_S296M_720p_50Hz:  *pWidth=1280; *pHeight=720;  *pFramerate=50; break;
        default: break;
    }
    return VHDERR_NOERROR;
}

// ── Application buffers ─────────────────────────────────────────────────────
ULONG VHD_InitApplicationBuffers(HANDLE /*StreamHandle*/) {
    return VHDERR_NOERROR;
}

ULONG VHD_GetApplicationBuffersSize(HANDLE StreamHandle, int /*BufferType*/,
                                    ULONG* pSize) {
    if (!pSize) return VHDERR_INVALIDPOINTER;
    *pSize = 0;
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(id(StreamHandle));
    if (it == g_streams.end()) return VHDERR_INVALIDHANDLE;
    *pSize = it->second.width * it->second.height * 4u;  // RGBA/RGBX32
    return VHDERR_NOERROR;
}

ULONG VHD_CreateSlotEx(HANDLE StreamHandle,
                       VHD_APPLICATION_BUFFER_DESCRIPTOR* pDescriptors,
                       HANDLE* pSlotHandle) {
    if (!pSlotHandle || !pDescriptors) return VHDERR_INVALIDPOINTER;
    std::lock_guard<std::mutex> lk(g_mu);
    auto sit = g_streams.find(id(StreamHandle));
    if (sit == g_streams.end()) return VHDERR_INVALIDHANDLE;
    const auto h = g_next_handle.fetch_add(1);
    MockSlot slot;
    slot.stream_id = id(StreamHandle);
    slot.buffer = pDescriptors[0].pBuffer;
    slot.size   = sit->second.width * sit->second.height * 4u;
    g_slots[h] = slot;
    *pSlotHandle = to_handle(h);
    return VHDERR_NOERROR;
}

ULONG VHD_GetSlotBuffer(HANDLE SlotHandle, ULONG /*BufferType*/,
                        BYTE** ppBuffer, ULONG* pSize) {
    if (!ppBuffer || !pSize) return VHDERR_INVALIDPOINTER;
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_slots.find(id(SlotHandle));
    if (it == g_slots.end()) return VHDERR_INVALIDHANDLE;
    *ppBuffer = static_cast<BYTE*>(it->second.buffer);
    *pSize    = it->second.size;
    return VHDERR_NOERROR;
}

ULONG VHD_QueueInSlot(HANDLE SlotHandle) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_slots.find(id(SlotHandle));
    if (it == g_slots.end()) return VHDERR_INVALIDHANDLE;
    auto sit = g_streams.find(it->second.stream_id);
    if (sit == g_streams.end()) return VHDERR_INVALIDHANDLE;
    sit->second.rx_queue.push_back(id(SlotHandle));
    return VHDERR_NOERROR;
}

ULONG VHD_QueueOutSlot(HANDLE SlotHandle) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_slots.find(id(SlotHandle));
    if (it == g_slots.end()) return VHDERR_INVALIDHANDLE;
    auto sit = g_streams.find(it->second.stream_id);
    if (sit == g_streams.end()) return VHDERR_INVALIDHANDLE;
    sit->second.tx_queue.push_back(id(SlotHandle));
    return VHDERR_NOERROR;
}

// RX side: pop a queued slot, paint a gradient into it, pace at the stream
// framerate, return the slot handle. The operator's compute() then wraps that
// buffer into a GXF entity and emits it downstream.
ULONG VHD_WaitSlotFilled(HANDLE StreamHandle, HANDLE* pSlotHandle,
                         ULONG /*TimeoutMs*/) {
    uintptr_t slot_id = 0;
    void* buf = nullptr;
    uint32_t size = 0;
    uint32_t framerate = 60;
    std::chrono::steady_clock::time_point wake_at{};
    {
        std::lock_guard<std::mutex> lk(g_mu);
        auto sit = g_streams.find(id(StreamHandle));
        if (sit == g_streams.end()) return VHDERR_INVALIDHANDLE;
        auto& s = sit->second;
        if (s.rx_queue.empty()) return VHDERR_TIMEOUT;

        slot_id = s.rx_queue.front();
        s.rx_queue.pop_front();
        auto slot_it = g_slots.find(slot_id);
        if (slot_it == g_slots.end()) return VHDERR_INVALIDHANDLE;
        buf  = slot_it->second.buffer;
        size = slot_it->second.size;
        framerate = s.framerate ? s.framerate : 60u;

        fill_pattern_locked(s, buf, size);
        s.frame_count++;

        const auto period = std::chrono::milliseconds(1000 / framerate);
        const auto now = std::chrono::steady_clock::now();
        if (s.next_frame_at == std::chrono::steady_clock::time_point{} || s.next_frame_at < now) {
            s.next_frame_at = now + period;
        } else {
            s.next_frame_at += period;
        }
        wake_at = s.next_frame_at;
    }
    std::this_thread::sleep_until(wake_at);
    if (pSlotHandle) *pSlotHandle = to_handle(slot_id);
    return VHDERR_NOERROR;
}

// TX side: pop a queued slot and report success immediately. The operator's
// VHD_QueueOutSlot already deposited the frame contents.
ULONG VHD_WaitSlotSent(HANDLE StreamHandle, HANDLE* pSlotHandle,
                       ULONG /*TimeoutMs*/) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto sit = g_streams.find(id(StreamHandle));
    if (sit == g_streams.end()) return VHDERR_INVALIDHANDLE;
    auto& s = sit->second;
    if (s.tx_queue.empty()) return VHDERR_TIMEOUT;
    const auto slot_id = s.tx_queue.front();
    s.tx_queue.pop_front();
    if (pSlotHandle) *pSlotHandle = to_handle(slot_id);
    return VHDERR_NOERROR;
}

// DV path uses this instead of per-property SetStreamProperty for the format.
ULONG VHD_PresetTimingStreamProperties(HANDLE StreamHandle, VHD_DV_STD /*Standard*/,
                                       ULONG Width, ULONG Height,
                                       ULONG Framerate, BOOL Interlaced) {
    std::lock_guard<std::mutex> lk(g_mu);
    auto it = g_streams.find(id(StreamHandle));
    if (it == g_streams.end()) return VHDERR_INVALIDHANDLE;
    it->second.width = Width;
    it->second.height = Height;
    it->second.framerate = Framerate;
    it->second.progressive = !Interlaced;
    return VHDERR_NOERROR;
}

const char* VHD_STREAMTYPE_ToString(VHD_STREAMTYPE StreamType) {
    switch (StreamType) {
        case VHD_ST_RX0: return "VHD_ST_RX0"; case VHD_ST_RX1: return "VHD_ST_RX1";
        case VHD_ST_RX2: return "VHD_ST_RX2"; case VHD_ST_RX3: return "VHD_ST_RX3";
        case VHD_ST_RX4: return "VHD_ST_RX4"; case VHD_ST_RX5: return "VHD_ST_RX5";
        case VHD_ST_RX6: return "VHD_ST_RX6"; case VHD_ST_RX7: return "VHD_ST_RX7";
        case VHD_ST_RX8: return "VHD_ST_RX8"; case VHD_ST_RX9: return "VHD_ST_RX9";
        case VHD_ST_RX10: return "VHD_ST_RX10"; case VHD_ST_RX11: return "VHD_ST_RX11";
        case VHD_ST_TX0: return "VHD_ST_TX0"; case VHD_ST_TX1: return "VHD_ST_TX1";
        case VHD_ST_TX2: return "VHD_ST_TX2"; case VHD_ST_TX3: return "VHD_ST_TX3";
        case VHD_ST_TX4: return "VHD_ST_TX4"; case VHD_ST_TX5: return "VHD_ST_TX5";
        case VHD_ST_TX6: return "VHD_ST_TX6"; case VHD_ST_TX7: return "VHD_ST_TX7";
        case VHD_ST_TX8: return "VHD_ST_TX8"; case VHD_ST_TX9: return "VHD_ST_TX9";
        case VHD_ST_TX10: return "VHD_ST_TX10"; case VHD_ST_TX11: return "VHD_ST_TX11";
        default: return "VHD_ST_MOCK";
    }
}

}  // extern "C"
