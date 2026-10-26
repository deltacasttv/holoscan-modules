# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0

import importlib
import inspect

import pytest

# Skip only when the Holoscan SDK itself is unavailable. Do not importorskip
# holoscan.deltacast: that would hide a broken module build as "skipped".
pytest.importorskip("holoscan.core", reason="holoscan SDK not installed", exc_type=ImportError)

EXPECTED_OPERATORS = ("VideoMasterSourceOp", "VideoMasterTransmitterOp")


@pytest.mark.parametrize("operator_name", EXPECTED_OPERATORS)
def test_operator_is_importable(operator_name):
    module = importlib.import_module("holoscan.deltacast")
    assert hasattr(module, operator_name), (
        f"holoscan.deltacast does not expose {operator_name}; "
        f"available names: {sorted(n for n in dir(module) if not n.startswith('_'))}"
    )
    assert inspect.isclass(getattr(module, operator_name))
