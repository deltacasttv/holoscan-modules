# syntax=docker/dockerfile:1

# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Dev container for Holoscan Deltacast.
#
# Extends the Holoscan SDK image with the build tools needed to configure,
# compile, and test the module from a live source mount. No source is copied
# into the image; the project tree is bind-mounted by the HoloHub CLI at
# /workspace/deltacast when the container is launched.
#
# Build & run via the HoloHub CLI (recommended):
#   ./holohub run-container
#
# Manual build (without the CLI):
#   docker build -t holoscan-deltacast .

ARG BASE_IMAGE=nvcr.io/nvidia/clara-holoscan/holoscan:v4.2.0-cuda13-dgpu
FROM ${BASE_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

# wget + unzip back the NGC CLI install below (matching HoloHub CLI's tool choice).
# ffmpeg is invoked by cmake/holohub_download_data.cmake to decode the .264
# endoscopy sample into raw RGB frames piped into convert_video_to_gxf_entities.py.
RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        libgtest-dev \
        clang-format \
        pybind11-dev \
        wget \
        unzip \
        ffmpeg \
    && rm -rf /var/lib/apt/lists/*

# NGC CLI for the endoscopy_data target in CMakeLists.txt. The `ngcsdk` PyPI
# package is a Python SDK only — the `ngc` shell command ships in a separate
# standalone zip hosted on NGC itself. URL/version/wget recipe matches HoloHub
# CLI's setup_ngc_cli() (utilities/cli/util.py); HoloHub itself installs this
# on-demand at runtime, we just pre-bake it into the image.
ARG NGC_CLI_VERSION=4.18.0
RUN ARCH_SUFFIX=$([ "$(uname -m)" = "aarch64" ] && echo arm64 || echo linux) \
    && wget --quiet --content-disposition \
        "https://api.ngc.nvidia.com/v2/resources/nvidia/ngc-apps/ngc_cli/versions/${NGC_CLI_VERSION}/files/ngccli_${ARCH_SUFFIX}.zip" \
        -O /tmp/ngc.zip \
    && unzip -q /tmp/ngc.zip -d /opt \
    && chmod u+x /opt/ngc-cli/ngc \
    && ln -s /opt/ngc-cli/ngc /usr/local/bin/ngc \
    && rm /tmp/ngc.zip \
    && ngc --version

# Python tooling:
#   pytest-timeout            — test harness
#   build + scikit-build-core — wheel packaging via `./holohub package --pkg-generator WHEEL`
RUN pip install --no-cache-dir pytest-timeout build scikit-build-core
