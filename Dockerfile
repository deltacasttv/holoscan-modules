# syntax=docker/dockerfile:1

# SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
# SPDX-License-Identifier: Apache-2.0
#
# Dev container for Holoscan Deltacast.
#
# Extends the Holoscan SDK image with the build tools needed to configure,
# compile, and test the module from a live source mount. No source is copied
# into the image; the project tree is bind-mounted by the Holoscan CLI at
# /workspace/deltacast when the container is launched.
#
# Build & run via the installed Holoscan CLI (recommended):
#   holoscan run-container
#
# Manual build (without the CLI):
#   docker build -t holoscan-deltacast .

ARG BASE_IMAGE=nvcr.io/nvidia/clara-holoscan/holoscan:v4.6.0-cuda13
FROM ${BASE_IMAGE}

ARG DEBIAN_FRONTEND=noninteractive

# Install the exact CLI pin. holoscan-cli has no runtime dependencies, so
# this adds nothing else to the image.
COPY requirements-cli.txt /tmp/requirements-cli.txt
RUN python3 -m pip install --no-cache-dir \
        --extra-index-url https://pypi.nvidia.com \
        -r /tmp/requirements-cli.txt \
    && holoscan version

# wget + unzip back the NGC CLI install below.
# ffmpeg is invoked by the SDK's download_ngc_data script (see
# cmake/holohub_download_data.cmake) to decode the .264 endoscopy sample into
# raw RGB frames piped into convert_video_to_gxf_entities.py.
RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        libgtest-dev \
        clang-format \
        pybind11-dev \
        wget \
        unzip \
        ffmpeg \
    && rm -rf /var/lib/apt/lists/*

# The SDK 4.6 image ships its helper scripts (download_ngc_data,
# convert_video_to_gxf_entities.py, ...) as root-only (0700). The CLI runs the
# container as the host user, so make them readable and executable.
RUN chmod -R a+rX /opt/nvidia/holoscan/bin

# NGC CLI for the endoscopy_data target in applications/CMakeLists.txt. The
# `ngcsdk` PyPI package is a Python SDK only — the `ngc` shell command ships in
# a separate standalone zip hosted on NGC itself.
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

# Test and wheel-build tooling. Must be in the same interpreter the CLI runs
# on: build and package pass it to CMake as Python3_EXECUTABLE.
RUN python3 -m pip install --no-cache-dir \
        'pytest>=8.2' pytest-timeout build scikit-build-core
