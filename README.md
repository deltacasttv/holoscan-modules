# Holoscan Deltacast

Holoscan components for using the Deltacast Videomaster capture card.

This project is a **Holoscan Module** — a self-contained, redistributable library that extends
[Holoscan SDK](https://developer.nvidia.com/holoscan-sdk) with reusable operators under the
`holoscan.deltacast` namespace.

The operators in this module target the **DELTACAST.TV** Videomaster capture card family. Contact [DELTACAST.TV](https://www.deltacast.tv/) for SDK access and environment setup; a bundled mock SDK lets you build and exercise the sample apps without a card (see below).

---

## Quick Start

Create and activate a Python (3.11+) virtual environment, then install the exact
[Holoscan CLI](https://github.com/nvidia-holoscan/holoscan-cli) version pinned by this module:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install \
  --extra-index-url https://pypi.nvidia.com \
  -r requirements-cli.txt

# Run the C++ receiver pipeline
holoscan run deltacast_receiver --language cpp

# Run the Python receiver pipeline
holoscan run deltacast_receiver --language python
```

Both applications ship C++ and Python variants; without `--language`, the CLI defaults to Python.
If you use [uv](https://docs.astral.sh/uv/), prefix the same commands with `uv run` instead
(for example `uv run holoscan list`). See `holoscan list` for all applications and operators.

---

## Operators

| Operator | Implementation | Direction |
|---|---|---|
| `VideoMasterSourceOp` | C++ + pybind11 | Capture from a Deltacast Videomaster card |
| `VideoMasterTransmitterOp` | C++ + pybind11 | Transmit to a Deltacast Videomaster card |

**Namespace**

- C++: `holoscan::ops`
- Python: `holoscan.deltacast`

See [`operators/deltacast_videomaster/README.md`](operators/deltacast_videomaster/README.md) for the full parameter list of each operator.

---

## Usage

Refer the [demo applications](applications/) for examples of how to use Holoscan Deltacast operators in your project.

### Python

```python
from holoscan.core import Application
from holoscan.deltacast import VideoMasterSourceOp


class MyApp(Application):
    def compose(self):
        source = VideoMasterSourceOp(self, name="source")
        # connect with add_flow(source, downstream_op, ...)


MyApp().run()
```

### C++

```cpp
#include <holoscan/holoscan.hpp>
#include <deltacast_videomaster/videomaster_source.hpp>

class MyApp : public holoscan::Application {
 public:
  void compose() override {
    auto source = make_operator<holoscan::ops::VideoMasterSourceOp>("source");
    // connect with add_flow(source, downstream_op, ...);
  }
};

int main() { holoscan::make_application<MyApp>()->run(); }
```

---

## Building from Source

### With Holoscan CLI (Recommended)

Build the dev container image, then build operators and run examples inside it:

```bash
holoscan build deltacast_receiver --language cpp
holoscan run deltacast_[transmitter/receiver] --language [cpp/python]
```

On a host without an NVIDIA GPU or NVIDIA Container Toolkit, add `--docker-opts="--runtime runc"`
to build and test commands. Running the applications still requires a GPU.

### Without Holoscan CLI

| Requirement | Version |
|---|---|
| Holoscan SDK | ≥ 4.6.0 |
| CUDA Toolkit | 13.x (matches the Holoscan SDK CUDA pin; the dev `Dockerfile` uses `cuda13`) |
| CMake | ≥ 3.24 |
| C++ compiler | C++17 (GCC 11+) |
| pybind11 | ≥ 2.11 |
| Python | ≥ 3.11 |

The Deltacast VideoMaster SDK is also required to target real hardware — contact DELTACAST.TV for access. The build links the bundled mock SDK by default, so the cmake command below succeeds without it.

```bash
cmake -S . -B build -DBUILD_ALL=ON -DHOLOSCAN_DELTACAST_BUILD_TESTING=ON
cmake --build build -j$(nproc)
```

Python bindings are staged in `build/python/lib/holoscan/deltacast/`. See [DEVELOPER.md](DEVELOPER.md#cmake-options)
for CMake options and their defaults.

---

## DELTACAST.TV VideoMaster SDK 

The Deltacast VideoMaster SDK is required to build and run on DELTACAST.TV capture card hardware.
This project bundles a *mock library* to satisfy minimum build and testing
features (animated RGBA gradient for RX, silent accept for TX) in environments without capture card hardware.

Use the real SDK by passing `-DVIDEOMASTER_SDK_DIR=<path> -DVIDEOMASTER_USE_MOCK=OFF`.

`VIDEOMASTER_SDK_DIR` is optional.

When it is set, it has top priority. The build system tries, in order:

1. `${VIDEOMASTER_SDK_DIR}/lib` + `${VIDEOMASTER_SDK_DIR}/headers`
2. `${VIDEOMASTER_SDK_DIR}/VideoMasterHDConfig.cmake` or `${VIDEOMASTER_SDK_DIR}/cmake/VideoMasterHDConfig.cmake`

If `VIDEOMASTER_SDK_DIR` is not set (or if detection from it fails), it falls back to:

1. `/usr/local/cmake/VideoMasterHDConfig.cmake`
2. `/usr/share/deltacast/videomaster/cmake/VideoMasterHDConfig.cmake`
3. `/usr/lib/libvideomasterhd.so` + headers in `/usr/include/videomaster`

Example custom SDK layout:

```
path/to/VideoMasterSDK/
|- headers/
|  |- VideoMasterHD_Core.h
|- lib/
   |- libvideomasterhd.so
```

In that case, set `VIDEOMASTER_SDK_DIR=path/to/VideoMasterSDK`.

---

## Testing

```bash
holoscan test
```

`holoscan test` configures a fresh build in `build-<project>/` and downloads data to
`data-<project>-<tag>/` in the repository root (both are git-ignored).

Or, without the Holoscan CLI:

```bash
# C++ (GTest via CTest); use `-L python` for the pytest entry, or omit -L for both
ctest --test-dir build --output-on-failure -L unit
```

```bash
# Python (pytest)
PYTHONPATH=build/python/lib${PYTHONPATH:+:$PYTHONPATH} HOLOSCAN_DELTACAST_BUILD_DIR=build pytest tests/python/ -v
```

`PYTHONPATH` is **prepended** so that an ambient holoscan SDK install stays
visible. A bare `PYTHONPATH=build/python/lib` would replace the variable and hide
the SDK from pytest.

Both suites currently cover importability and build-smoke only; full live-pipeline coverage is a TODO and requires real hardware. CTest is configured with `SKIP_RETURN_CODE 5` on the pytest entry — if a CTest run reports "Skipped" unexpectedly, the most common cause is that the holoscan SDK is not importable in the test environment (pytest collects zero items and exits 5).

---

## Support

This module is maintained by the DELTACAST.TV team. For help, please visit:

- **VideoMaster SDK, drivers, or DELTACAST hardware** — contact [DELTACAST.TV](https://www.deltacast.tv/) or email `support@deltacast.tv`.
- **Holoscan SDK, framework, or pipeline questions** — see the [Holoscan SDK documentation](https://docs.nvidia.com/holoscan/sdk-user-guide/).

---

## License

Apache-2.0 — see [LICENSE](LICENSE).
