// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <limits>

#include "kleidicv/utils.h"

using kleidicv::neon::check_image_size;
using kleidicv::neon::saturating_sub;

TEST(Utils, SaturatingSub1) { EXPECT_EQ(5UL, saturating_sub(10UL, 5UL)); }

TEST(Utils, SaturatingSub2) { EXPECT_EQ(0UL, saturating_sub(5UL, 10UL)); }

TEST(Utils, CheckImageSizeValid) {
  EXPECT_EQ(KLEIDICV_OK, check_image_size(1, 1));
  EXPECT_EQ(KLEIDICV_OK, check_image_size(640, 480));
  EXPECT_EQ(KLEIDICV_OK, check_image_size(KLEIDICV_MAX_IMAGE_PIXELS, 1));
  EXPECT_EQ(KLEIDICV_OK, check_image_size(1, KLEIDICV_MAX_IMAGE_PIXELS));
}

TEST(Utils, CheckImageSizeZero) {
  EXPECT_EQ(KLEIDICV_OK, check_image_size(0, 0));
  EXPECT_EQ(KLEIDICV_OK,
            check_image_size(0, std::numeric_limits<size_t>::max()));
  EXPECT_EQ(KLEIDICV_OK,
            check_image_size(std::numeric_limits<size_t>::max(), 0));
}

TEST(Utils, CheckImageSizeOversize) {
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(KLEIDICV_MAX_IMAGE_PIXELS + 1, 1));
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(1, KLEIDICV_MAX_IMAGE_PIXELS + 1));
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(KLEIDICV_MAX_IMAGE_PIXELS / 2 + 1, 2));
}

TEST(Utils, CheckImageSizeOverflow) {
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(std::numeric_limits<size_t>::max(), 2));
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(std::numeric_limits<size_t>::max() / 2 + 1, 2));
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            check_image_size(2, std::numeric_limits<size_t>::max() / 2 + 1));
}
