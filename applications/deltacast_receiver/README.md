# Deltacast Videomaster Receiver

This application demonstrates the use of `videomaster_source` to receive and display video streams from a Deltacast capture card using Holoviz for visualization.

## Requirements

Install the DELTACAST VideoMaster SDK and driver and contact [DELTACAST.TV](https://www.deltacast.tv/) (or `support@deltacast.tv`) for SDK access. Then build with the real SDK linked in — see Build Instructions below.

A mock library is provided for testing purposes in the event that a capture card is not available.

## Build and Run Instructions

See [Building from Source](../../README.md#building-from-source) in the top-level README. To link against the real VideoMaster SDK (in place of the bundled mock):

```bash
./holohub build --local deltacast_receiver \
    --configure-args="-DVIDEOMASTER_USE_MOCK=OFF -DVideoMaster_SDK_DIR=<path to VideoMaster SDK>"
```

Once built, run the binaries from the build directory:

```bash
# C++
./applications/deltacast_receiver/cpp/deltacast_receiver

# Python
python3 applications/deltacast_receiver/python/deltacast_receiver.py
```
