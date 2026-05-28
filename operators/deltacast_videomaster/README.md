# DELTACAST VideoMaster Operators

The DELTACAST VideoMaster operator provides functionality to capture and stream high-quality video streams from DELTACAST cards. It supports both SDI and HDMI input and output sources, enabling professional video capture in various formats and resolutions. DELTACAST VideoMaster operators are designed to work seamlessly with DELTACAST's hardware capabilities.

This library contains two operators:

- `VideoMasterSourceOp` (`videomaster_source`): Captures a signal from the DELTACAST capture card.
- `VideoMasterTransmitterOp` (`videomaster_transmitter`): Streams a signal through the DELTACAST capture card.

Hardware-free builds and tests run the real operators against the bundled mock VideoMaster SDK (`cmake/mock/VideoMasterHD/`) — see [`DEVELOPER.md`](../../DEVELOPER.md) for details.

## Requirements

To target real hardware:

- VideoMaster SDK from DELTACAST (≥ 6.25).
- A compatible DELTACAST capture card.
- The DELTACAST driver, so the card is discoverable at runtime.

The module also ships a bundled mock VideoMaster SDK (`cmake/mock/VideoMasterHD/`), enabled by default, so the operators build and the sample apps run end-to-end without a card. See the top-level [`README.md`](../../README.md) and [`DEVELOPER.md`](../../DEVELOPER.md) for the mock-vs-real toggle (`-DVIDEOMASTER_USE_MOCK=ON|OFF`).

## Parameters

### videomaster_source

The following parameters can be configured for this operator:

| Parameter     | Type     | Description                                                                                          | Default |
| ------------- | -------- | ---------------------------------------------------------------------------------------------------- | ------- |
| `board`       | uint32_t | Index of the DELTACAST.TV board to use as source                                                     | 0       |
| `rdma`        | bool     | Enable RDMA for video input (DELTACAST driver must be compiled with RDMA enabled to use this option) | false   |
| `input`       | uint32_t | Index of the RX channel to use on the selected board                                                 | 0       |
| `width`       | uint32_t | Width of the input stream                                                                            | 1920    |
| `height`      | uint32_t | Height of the input stream                                                                           | 1080    |
| `progressive` | bool     | Interleaved or progressive                                                                           | true    |
| `framerate`   | uint32_t | Framerate of the input stream                                                                        | 60      |

### videomaster_transmitter

The following parameters can be configured for this operator:

| Parameter        | Type     | Description                                                                                          | Default |
| ---------------- | -------- | ---------------------------------------------------------------------------------------------------- | ------- |
| `board`          | uint32_t | Index of the DELTACAST.TV board to use as source                                                     | 0       |
| `rdma`           | bool     | Enable RDMA for video input (DELTACAST driver must be compiled with RDMA enabled to use this option) | false   |
| `output`         | uint32_t | Index of the TX channel to use on the selected board                                                 | 0       |
| `width`          | uint32_t | The width of the output stream                                                                       | 1920    |
| `height`         | uint32_t | The height of the output stream                                                                      | 1080    |
| `progressive`    | bool     | interleaved or progressive                                                                           | true    |
| `framerate`      | uint32_t | The framerate of the output stream                                                                   | 60      |
| `enable_overlay` | bool     | Is overlay provided by the card or not                                                               | false   |

## Building the operator

See [Building from Source](../../README.md#building-from-source) in the top-level README. The mock VideoMaster SDK is linked by default; pass `-DVIDEOMASTER_USE_MOCK=OFF -DVideoMaster_SDK_DIR=<path>` to link against the real SDK.

## Tests

Hardware validation last performed with the DELTACAST VideoMaster SDK `6.32`. The Holoscan SDK column was updated to reflect the current minimum-required version (see [`metadata.json`](metadata.json)); these scenarios should be re-validated whenever the Holoscan SDK pin moves.

| Application             | Device                  | Configuration                       | Holoscan SDK 4.2 |
| ----------------------- | ----------------------- | ----------------------------------- | ---------------- |
| deltacast_transmitter   | DELTA-12G-elp-key 11    | TX0 (SDI) / ~~RDMA~~                | PASSED           |
| deltacast_transmitter   | DELTA-12G-elp-key 11    | TX0 (SDI) / RDMA                    | PASSED           |
| deltacast_receiver      | DELTA-12G-elp-key 11    | RX0 (SDI) / ~~RDMA~~                | PASSED           |
| deltacast_receiver      | DELTA-12G-elp-key 11    | RX0 (SDI) / RDMA                    | PASSED           |
| endoscopy_tool_tracking | DELTA-12G-elp-key 11    | RX0 (SDI) / ~~overlay~~ / ~~RDMA~~  | PASSED           |
| endoscopy_tool_tracking | DELTA-12G-elp-key 11    | RX0 (SDI) / ~~overlay~~ / RDMA      | PASSED           |
| endoscopy_tool_tracking | DELTA-12G-elp-key 11    | RX0/TX0 (SDI) / overlay / ~~RDMA~~  | PASSED           |
| endoscopy_tool_tracking | DELTA-12G-elp-key 11    | RX0/TX0 (SDI) / overlay / RDMA      | PASSED           |
