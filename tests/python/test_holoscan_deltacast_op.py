# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Functional tests for VideoMasterSourceOp and VideoMasterTransmitterOp.

import importlib
import inspect

import pytest

pytest.importorskip("holoscan", reason="holoscan SDK not installed", exc_type=ImportError)


EXPECTED_OPERATORS = (
    "VideoMasterSourceOp",
    "VideoMasterTransmitterOp",
)


@pytest.mark.parametrize("operator_name", EXPECTED_OPERATORS)
def test_operator_is_importable(operator_name):
    # Skip per-test rather than at module level: a module-level importorskip
    # collects zero items, which makes pytest exit with code 5 and CTest mark
    # the whole run as Skipped — hiding the deltacast import failures we want
    # to surface.
    deltacast = importlib.import_module("holoscan.deltacast")

    assert hasattr(deltacast, operator_name), (
        f"holoscan.deltacast does not expose {operator_name}; "
        f"available names: {sorted(n for n in dir(deltacast) if not n.startswith('_'))}"
    )
    operator_cls = getattr(deltacast, operator_name)
    assert inspect.isclass(operator_cls), (
        f"holoscan.deltacast.{operator_name} is not a class (got {type(operator_cls)!r})"
    )


def test_placeholder():
    # TODO: build a minimal Application, call app.run(), assert on results.
    pass
