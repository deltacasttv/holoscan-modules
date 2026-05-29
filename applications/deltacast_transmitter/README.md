# Deltacast Videomaster Transmitter

This application transmits a video stream through a Deltacast Videomaster card.
Frames come from a `VideoStreamReplayerOp` reading the `surgical_video` GXF entity
pair, fetched at build time.

## Requirements

Install the DELTACAST VideoMaster SDK and driver and contact [DELTACAST.TV](https://www.deltacast.tv/) (or `support@deltacast.tv`) for SDK access. Then build with the real SDK linked in — see Build Instructions below.

A mock library is provided for testing purposes in the event that a capture card is not available.

This application additionally needs:

- `ngc` (the NGC CLI) on `PATH` to fetch the endoscopy sample. Get the standalone zip from <https://ngc.nvidia.com/setup/installers/cli>; the `ngcsdk` PyPI package is a Python SDK only and does **not** provide the `ngc` shell command. Preinstalled in this module's dev `Dockerfile`.
- `NGC_CLI_API_KEY` in the environment **only** for gated resources; the Holoscan endoscopy sample is public.

## Data

`cmake --build` runs the `endoscopy_data` target defined at
`applications/CMakeLists.txt` via `holoscan_download_data()` (see
[`cmake/holohub_download_data.cmake`](../../cmake/holohub_download_data.cmake)).
The CMake helper invokes the HoloHub `download_ngc_data` bash script — vendored
into `.holohub/utilities/` by the wrapper bootstrap — which:

1. Fetches [holoscan_endoscopy_sample_data:20230222](https://catalog.ngc.nvidia.com/orgs/nvidia/teams/clara-holoscan/resources/holoscan_endoscopy_sample_data)
   from NGC into `${HOLOHUB_DATA_DIR}/endoscopy/`.
2. Pipes `ffmpeg`-decoded raw RGB frames into
   `convert_video_to_gxf_entities.py` (854×480 @ 30 fps) to produce
   `surgical_video.gxf_entities` + `surgical_video.gxf_index`, which is what
   `VideoStreamReplayerOp` consumes.

Output lands at `<HOLOHUB_DATA_DIR>/endoscopy/` (default `<module>/data/endoscopy/` when
run via `./holohub`, or `<build>/data/endoscopy/` for a plain CMake build). Disable with
`-DHOLOSCAN_DELTACAST_DOWNLOAD_DATA=OFF` and supply your own:

```bash
./holohub run deltacast_transmitter -- --data /path/to/endoscopy            # C++
./holohub run deltacast_transmitter --language python -- --data_path /path  # Python
```

## Build

See [Building from Source](../../README.md#building-from-source) in the top-level README. To link against the real VideoMaster SDK (in place of the bundled mock):

```bash
./holohub build --local deltacast_transmitter \
   --configure-args="-DVIDEOMASTER_USE_MOCK=OFF -DVIDEOMASTER_SDK_DIR=<path to VideoMaster SDK>"
```

`VIDEOMASTER_SDK_DIR` is optional. Use it only when the SDK is installed in a non-default location. For the full SDK detection order and examples, see the top-level [DELTACAST.TV VideoMaster SDK](../../README.md#deltacasttv-videomaster-sdk) section.

## Run

```bash
# C++
./applications/deltacast_transmitter/cpp/deltacast_transmitter --data <build>/data/endoscopy

# Python (from the build dir, `data/endoscopy` is the default)
python3 applications/deltacast_transmitter/python/deltacast_transmitter.py
```
