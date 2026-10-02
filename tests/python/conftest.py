# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Makes the build-tree Python package visible to pytest when PYTHONPATH does
# not already include <build>/python/lib (e.g. a bare `pytest` invocation).

import os

build_dir = os.environ.get(
    "HOLOSCAN_DELTACAST_BUILD_DIR",
    os.path.join(os.path.dirname(__file__), "..", "..", "build"),
)

try:
    import holoscan

    build_holoscan_path = os.path.abspath(os.path.join(build_dir, "python", "lib", "holoscan"))
    if build_holoscan_path not in holoscan.__path__:
        holoscan.__path__.insert(0, build_holoscan_path)
except ImportError:
    # holoscan not importable (e.g. outside the dev container); tests skip.
    pass
