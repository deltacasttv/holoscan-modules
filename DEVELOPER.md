# Developer Guide — Holoscan Deltacast

This guide covers the layout, build system, and day-to-day workflow for developing and
distributing this Holoscan Module.

---

## Module layout

```
holoscan-deltacast/
├── holohub                         # CLI wrapper (delegates to HoloHub CLI)
├── Dockerfile                      # Dev container image
├── CMakeLists.txt                  # Root CMake — orchestrates operators/applications/tests
├── pyproject.toml                  # Python packaging metadata (scikit-build-core)
├── metadata.json                   # Module-level metadata (schema: holoscan/module/v2)
├── operators/
│   └── deltacast_videomaster/
│       ├── *.cpp / *.hpp           # Operator implementation
│       └── metadata.json           # Operator-level metadata
├── applications/
│   ├── deltacast_receiver/         # Example receiver pipeline (C++ + Python)
│   └── deltacast_transmitter/      # Example transmitter pipeline (C++ + Python)
├── python/holoscan/deltacast/
│   └── __init__.py                 # Re-exports operators for `from holoscan.deltacast import ...`
└── tests/
    ├── cpp/                        # GTest suite
    └── python/                     # pytest suite
```

---

## `holohub` wrapper commands

The `holohub` script at the module root wraps the HoloHub CLI with
module-specific defaults. On first run it downloads the CLI tools via sparse-checkout
into `.holohub/` (internet required; subsequent runs use the cached copy). Set
`CLI_FORCE_UPDATE=1` to re-download (e.g. after bumping `CLI_PINNED_COMMIT`).

| Command | What it does |
|---|---|
| `./holohub run-container` | Build and start the dev container |
| `./holohub build deltacast_receiver` | CMake configure + build inside the container |
| `./holohub run deltacast_receiver` | Run the receiver example |
| `./holohub run deltacast_transmitter` | Run the transmitter example |

---

## Building without the wrapper

```bash
cmake -S . -B build -DBUILD_ALL=ON -DHOLOSCAN_DELTACAST_BUILD_TESTING=ON
cmake --build build -j"$(nproc)"
```

Run C++ tests:

```bash
ctest --test-dir build --output-on-failure -L unit
```

Run Python tests:

```bash
HOLOSCAN_DELTACAST_BUILD_DIR=build \
PYTHONPATH=build/python:$PYTHONPATH \
pytest tests/python/ -v
```

`PYTHONPATH` is **prepended**, not replaced — if the holoscan SDK is on `PYTHONPATH` in your shell, the form above keeps it visible. Replacing the variable (a bare `PYTHONPATH=build/python`) silently drops the SDK, the module-level `importorskip("holoscan")` fires, and pytest exits with code 5 (no tests collected).

---

## Mock VideoMaster SDK

This repo ships a bundled mock SDK at `cmake/mock/VideoMasterHD/` so the
operator and sample apps build and run without a Deltacast card.

| Where | What |
|---|---|
| `cmake/FindVideoMasterHD.cmake` | `find_package(VideoMasterHD)` shim. Default `option(VIDEOMASTER_USE_MOCK ON)`; flip with `-DVIDEOMASTER_USE_MOCK=OFF` and `-DVIDEOMASTER_SDK_DIR=<path>` to link the real SDK. |
| `cmake/mock/VideoMasterHD/include/VideoMasterHD_*.h` | Header declarations covering Core, ApplicationBuffers, Sdi, Dv, Keyer, String. |
| `cmake/mock/VideoMasterHD/src/mock_sdk.cpp` | Stateful implementation. RX (`VHD_WaitSlotFilled`) paints an animated RGBA gradient into the slot buffer and paces to the configured framerate; TX (`VHD_WaitSlotSent`) accepts queued slots silently. |

`VIDEOMASTER_SDK_DIR` is optional. If set, it is tried first; otherwise CMake falls back to global config locations and then classic system paths.

Detection order:

1. `${VIDEOMASTER_SDK_DIR}` direct layout (`lib` + `headers`), then `${VIDEOMASTER_SDK_DIR}/VideoMasterHDConfig.cmake` or `${VIDEOMASTER_SDK_DIR}/cmake/VideoMasterHDConfig.cmake`
2. `/usr/local/cmake/VideoMasterHDConfig.cmake`
3. `/usr/share/deltacast/videomaster/cmake/VideoMasterHDConfig.cmake`
4. `/usr/lib/libvideomasterhd.so` + `/usr/include/videomaster`

The mock makes `find_package(VideoMasterHD)` succeed for both this module and
the `VideoMasterAPIHelper` FetchContent (which calls
`find_package(VideoMasterHD 6.25 REQUIRED)` from its own CMakeLists). The api-
helper compiles against the mock headers and links the mock static lib via
the `VideoMasterHD::Core` ALIAS.

To verify against real hardware later:

```bash
cmake -S . -B build-real \
    -DBUILD_ALL=ON \
    -DVIDEOMASTER_SDK_DIR=/path/to/videomaster-sdk \
    -DVIDEOMASTER_USE_MOCK=OFF
```

---

## `pyproject.toml`

`pyproject.toml` configures [scikit-build-core](https://scikit-build-core.readthedocs.io/) for
wheel packaging. Key fields to update before publishing:

| Field | Purpose |
|---|---|
| `[project].name` | PyPI package name — should match `metadata.json:module.binary_packages.pypi` |
| `[project].version` | Sync with `metadata.json:module.version` |
| `[project].description` | Short description shown on PyPI |
| `[project].authors` | Your name / organisation |
| `[tool.scikit-build].cmake.args` | Extra CMake flags passed during `pip install` |

Build a wheel:

```bash
pip install build
python -m build --wheel
```

---

## Naming conventions

| Context | Convention | Example |
|---|---|---|
| Python import / C++ namespace | `snake_case` | `holoscan.deltacast` |
| Repository folder | `holoscan-<slug>` (kebab) | `holoscan-deltacast` |
| Debian package | `holoscan-<slug>` (kebab) | `holoscan-deltacast` |
| Python wheel | `holoscan-<slug>` (kebab) | `holoscan-deltacast` |
| CMake option prefix | `UPPER_SNAKE` | `HOLOSCAN_DELTACAST_BUILD_TESTING` |

---

## Further reading

- [HoloHub documentation](https://github.com/nvidia-holoscan/holohub)
- [Holoscan SDK documentation](https://docs.nvidia.com/holoscan/sdk-user-guide/)
- [Holoscan Module ecosystem](https://nvidia-holoscan.github.io/)
