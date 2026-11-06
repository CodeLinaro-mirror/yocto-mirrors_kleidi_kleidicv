// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
#include <vector>

#include "framework/utils.h"
#include "kleidicv/analysis/standalone_lucas_kanade_alg.h"
#include "kleidicv/dispatch.h"

namespace {

class StandaloneLucasKanadeAlgBackendComparison : public testing::Test {
 protected:
  void SetUp() override {
    if (!kleidicv::is_sve2_supported() || !kleidicv::is_sme_supported()) {
      GTEST_SKIP() << "Backend comparison requires SVE2 and SME";
    }
  }

  static void compare(int window_width, int window_height, int channels,
                      size_t row_padding = 0) {
    SCOPED_TRACE(window_width);
    SCOPED_TRACE(window_height);
    SCOPED_TRACE(channels);
    SCOPED_TRACE(row_padding);
    constexpr int kWidth = 96;
    constexpr int kHeight = 80;
    const size_t pad =
        static_cast<size_t>(std::max(window_width, window_height));
    const size_t row_bytes = (kWidth + 2 * pad) * channels;
    const size_t stride = row_bytes + row_padding;
    const size_t rows = kHeight + 2 * pad;
    std::vector<uint8_t> prev(stride * rows), next(stride * rows);
    std::vector<int16_t> gradients(2 * stride * rows);
    std::mt19937 generator{
        static_cast<std::mt19937::result_type>(test::Options::seed())};
    std::generate(prev.begin(), prev.end(), generator);

    // Subpixel shift; +32 rounds to the nearest integer before >>6.
    for (size_t y = 1; y < rows; ++y) {
      for (size_t x = channels; x < row_bytes; ++x) {
        const size_t i = y * stride + x;
        next[i] = static_cast<uint8_t>((35 * prev[i] + 21 * prev[i - channels] +
                                        5 * prev[i - stride] +
                                        3 * prev[i - stride - channels] + 32) >>
                                       6);
      }
    }

    // Compute Scharr derivatives once, including the padded image region.
    for (size_t y = 1; y + 1 < rows; ++y) {
      for (size_t x = channels; x + channels < row_bytes; ++x) {
        const size_t i = y * stride + x;
        gradients[2 * i] = static_cast<int16_t>(
            3 * (prev[i - stride + channels] + prev[i + stride + channels] -
                 prev[i - stride - channels] - prev[i + stride - channels]) +
            10 * (prev[i + channels] - prev[i - channels]));
        gradients[2 * i + 1] = static_cast<int16_t>(
            3 * (prev[i + stride - channels] + prev[i + stride + channels] -
                 prev[i - stride - channels] - prev[i - stride + channels]) +
            10 * (prev[i + stride] - prev[i - stride]));
      }
    }

    const size_t origin = pad * stride + pad * channels;
    const std::array<float, 6> kPoints = {40.25F, 36.5F, 48.125F,
                                          40.75F, 50.5F, 32.25F};
    const std::array kBackends = {
        std::pair{"SVE2", &kleidicv::sve2::standalone_lucas_kanade_alg_u8},
        std::pair{"SME", &kleidicv::sme::standalone_lucas_kanade_alg_u8}};
    struct Result {
      std::array<float, 6> points;
      std::array<uint8_t, 3> status;
      std::array<float, 3> error;
    };
    for (bool get_min_eigen_vals : {false, true}) {
      SCOPED_TRACE(get_min_eigen_vals);
      const auto run = [&](auto function, Result &result) {
        result.points = kPoints;
        result.status.fill(1);
        result.error.fill(-1.0F);
        return function(prev.data() + origin, stride,
                        gradients.data() + 2 * origin,
                        2 * stride * sizeof(int16_t), next.data() + origin,
                        stride, kWidth, kHeight, channels, kPoints.data(),
                        result.points.data(), result.status.size(),
                        result.status.data(), result.error.data(), window_width,
                        window_height, 30, 0.0001, get_min_eigen_vals, 0.0001F);
      };

      Result reference{};
      ASSERT_EQ(
          KLEIDICV_OK,
          run(&kleidicv::neon::standalone_lucas_kanade_alg_u8, reference));
      ASSERT_GT(std::count(reference.status.begin(), reference.status.end(),
                           uint8_t{1}),
                0);
      for (const auto &[name, function] : kBackends) {
        SCOPED_TRACE(name);
        Result actual{};
        ASSERT_EQ(KLEIDICV_OK, run(function, actual));
        ASSERT_EQ(reference.status, actual.status);
        for (size_t point = 0; point < reference.status.size(); ++point) {
          SCOPED_TRACE(point);
          if (!reference.status[point]) {
            continue;
          }
          ASSERT_TRUE(std::isfinite(reference.error[point]));
          EXPECT_EQ(test::float_to_bits(reference.points[2 * point]),
                    test::float_to_bits(actual.points[2 * point]));
          EXPECT_EQ(test::float_to_bits(reference.points[2 * point + 1]),
                    test::float_to_bits(actual.points[2 * point + 1]));
          EXPECT_EQ(test::float_to_bits(reference.error[point]),
                    test::float_to_bits(actual.error[point]));
        }
      }
    }
  }
};

TEST_F(StandaloneLucasKanadeAlgBackendComparison, SuccessfulTracks) {
  for (int channels : {1, 2, 3, 4}) {
    for (int window : {15, 21, 31}) {
      compare(window, window, channels);
    }
  }
}

TEST_F(StandaloneLucasKanadeAlgBackendComparison, NarrowWindows) {
  for (int channels : {1, 2, 3, 4}) {
    for (int width : {1, 2, 3, 4, 5, 7, 8, 9}) {
      compare(width, 31, channels, 7);
    }
  }
}

TEST_F(StandaloneLucasKanadeAlgBackendComparison, LargeWindows) {
  for (int channels : {1, 4}) {
    compare(127, 127, channels, 5);
  }
}

}  // namespace
