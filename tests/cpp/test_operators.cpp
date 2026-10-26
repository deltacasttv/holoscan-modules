/*
 * SPDX-FileCopyrightText: Copyright (c) 2026 Deltacast / NVIDIA. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>

#include <deltacast_videomaster/videomaster_source.hpp>
#include <deltacast_videomaster/videomaster_transmitter.hpp>
#include <holoscan/holoscan.hpp>

TEST(VideoMasterOperators, SourceOpCanBeCreated) {
  holoscan::Fragment fragment;
  auto op = fragment.make_operator<holoscan::ops::VideoMasterSourceOp>("source");
  ASSERT_NE(op, nullptr);
  EXPECT_EQ(op->name(), "source");
  EXPECT_NE(op->spec()->outputs().find("signal"), op->spec()->outputs().end());
}

TEST(VideoMasterOperators, TransmitterOpCanBeCreated) {
  holoscan::Fragment fragment;
  auto op = fragment.make_operator<holoscan::ops::VideoMasterTransmitterOp>("transmitter");
  ASSERT_NE(op, nullptr);
  EXPECT_EQ(op->name(), "transmitter");
  EXPECT_NE(op->spec()->inputs().find("source"), op->spec()->inputs().end());
}
