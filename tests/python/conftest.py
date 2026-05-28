# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# conftest.py — makes the build-tree Python package visible to pytest.
#
# Holoscan SDK is installed as a regular package (has __init__.py), so Python
# discards namespace-package directories alongside it. We extend holoscan.__path__
# directly after import to insert our build-tree holoscan/ directory, making
# holoscan.deltacast resolvable to our compiled modules.

import os

build_dir = os.environ.get(
    "HOLOSCAN_DELTACAST_BUILD_DIR",
    os.path.join(os.path.dirname(__file__), "..", "..", "build"),
)

try:
    import holoscan  # noqa: E402

    build_holoscan_path = os.path.join(build_dir, "python", "holoscan")
    if build_holoscan_path not in holoscan.__path__:
        holoscan.__path__.insert(0, build_holoscan_path)
except ImportError:
    # holoscan not importable (e.g. CUDA not available outside the dev container).
    # Individual tests use pytest.importorskip("holoscan") to skip gracefully.
    pass
