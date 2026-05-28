# Holoscan Deltacast

Holoscan components for using the Deltacast Videomaster capture card.

This project is a **Holoscan Module** — a self-contained, redistributable library that extends
[Holoscan SDK](https://developer.nvidia.com/holoscan-sdk) with reusable operators under the
`holoscan.deltacast` namespace.

The operators in this module target the **DELTACAST.TV** Videomaster capture card family. Contact [DELTACAST.TV](https://www.deltacast.tv/) for SDK access and environment setup; a bundled mock SDK lets you build and exercise the sample apps without a card (see below).

---

## Quick Start

```bash
# Run the C++ receiver pipeline
./holohub run deltacast_receiver

# Run the Python receiver pipeline
./holohub run deltacast_receiver --language python
```

This project uses the [NVIDIA HoloHub CLI](https://github.com/nvidia-holoscan/holohub) to streamline developer build and run commands.
See `./holohub list` for more details.

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

### With HoloHub CLI (Recommended)

Run the following command to build operators and run examples in the project container:

```bash
./holohub run deltacast_[transmitter/receiver]
```

### Without HoloHub CLI

| Requirement | Version |
|---|---|
| Holoscan SDK | ≥ 4.2.0 |
| CUDA Toolkit | 13.x (matches the Holoscan SDK CUDA pin; the dev `Dockerfile` uses `cuda13-dgpu`) |
| CMake | ≥ 3.24 |
| C++ compiler | C++17 (GCC 11+) |
| pybind11 | ≥ 2.11 |
| Python | 3.10–3.13 |

The Deltacast VideoMaster SDK is also required to target real hardware — contact DELTACAST.TV for access. The build links the bundled mock SDK by default, so the cmake command below succeeds without it.

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

---

## DELTACAST.TV VideoMaster SDK 

The Deltacast VideoMaster SDK is required to build and run on DELTACAST.TV capture card hardware.
This project bundles a *mock library* to satisfy minimum build and testing
features (animated RGBA gradient for RX, silent accept for TX) in environments without capture card hardware.

Use the real SDK by passing `-DVideoMaster_SDK_DIR=<path> -DVIDEOMASTER_USE_MOCK=OFF`.

---

## Testing

```bash
./holohub test
```

Or, without the HoloHub CLI:

```bash
# C++ (GTest via CTest)
ctest --test-dir build --output-on-failure -L unit
```

```bash
# Python (pytest)
PYTHONPATH=build/python:$PYTHONPATH HOLOSCAN_DELTACAST_BUILD_DIR=build pytest tests/python/ -v
```

`PYTHONPATH` is **prepended** so that an ambient holoscan SDK install stays
visible. A bare `PYTHONPATH=build/python` would replace the variable and hide
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
