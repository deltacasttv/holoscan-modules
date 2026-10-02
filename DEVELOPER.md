# Developer Guide — Holoscan Deltacast

This guide covers the layout, build system, and day-to-day workflow for developing and
distributing this Holoscan Module.

---

## Module layout

```
holoscan-deltacast/
├── requirements-cli.txt            # Tested holoscan-cli version pin
├── Dockerfile                      # Dev container image
├── CMakeLists.txt                  # Root CMake — orchestrates operators/applications/tests
├── pyproject.toml                  # Python packaging metadata (scikit-build-core)
├── metadata.json                   # Module-level metadata (schema: urn:holohub:module:v2)
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

## Holoscan CLI commands

Install the pinned CLI into a Python 3.11+ environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install \
  --extra-index-url https://pypi.nvidia.com \
  -r requirements-cli.txt
```

The dev `Dockerfile` installs the same pin into the container image. After changing
`requirements-cli.txt` (keep `[dependency-groups].dev` in `pyproject.toml` in sync), reinstall
the requirements and rebuild the image.

| Command | What it does |
|---|---|
| `holoscan run-container` | Build and start the dev container |
| `holoscan build deltacast_receiver --language cpp` | CMake configure + build inside the container |
| `holoscan run deltacast_receiver --language cpp` | Run the C++ receiver example |
| `holoscan run deltacast_transmitter --language python` | Run the Python transmitter example |
| `holoscan test` | Run CTest (C++ unit tests) and pytest |
| `holoscan install --dev` | Install a `.pth` hook for live module imports in the current Python environment |

Both applications have C++ and Python variants; pass `--language` explicitly, since the CLI
defaults to Python. On a machine without an NVIDIA GPU / container toolkit, add
`--docker-opts="--runtime runc"` to build-only commands.

---

## Building without the CLI

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
PYTHONPATH=build/python/lib${PYTHONPATH:+:$PYTHONPATH} \
pytest tests/python/ -v
```

`PYTHONPATH` is **prepended**, not replaced — if the holoscan SDK is on `PYTHONPATH` in your shell, the form above keeps it visible. Replacing the variable (a bare `PYTHONPATH=build/python/lib`) silently drops the SDK, the module-level `importorskip("holoscan")` fires, and pytest exits with code 5 (no tests collected).

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

- [Holoscan CLI documentation](https://github.com/nvidia-holoscan/holoscan-cli)
- [HoloHub documentation](https://github.com/nvidia-holoscan/holohub)
- [Holoscan SDK documentation](https://docs.nvidia.com/holoscan/sdk-user-guide/)
- [Holoscan Module ecosystem](https://nvidia-holoscan.github.io/)
