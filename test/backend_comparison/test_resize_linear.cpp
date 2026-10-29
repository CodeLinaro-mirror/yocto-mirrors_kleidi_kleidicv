// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <utility>

#include "framework/array.h"
#include "framework/generator.h"
#include "framework/utils.h"
#include "kleidicv/dispatch.h"
#include "kleidicv/resize/resize_linear.h"

namespace {

class ResizeLinearTestBackendComparison final {
 public:
  void test_regression_values(size_t scale) {
    // Cover multiply-add contraction and differences in the evaluation order
    // of interpolation between the first and last rows.
    const float kValues[][2][2] = {{{205.48390197753906F, 178.02554321289062F},
                                    {86.76388549804688F, 39.647274017333984F}},
                                   {{6.766672134399414F, 50.7036018371582F},
                                    {165.7205352783203F, 138.9600830078125F}}};
    for (const auto& values : kValues) {
      SCOPED_TRACE(values[0][0]);
      test(
          2,
          [&](size_t row, size_t column) {
            return values[row % 2][column % 2];
          },
          scale);
    }
  }

  void test_random_values(size_t scale) {
    test(19,
         test::PseudoRandomNumberGeneratorFloatRange<float>{-1000.0F, 1000.0F},
         scale);
  }

 private:
  template <typename InputValues>
  static void test(size_t height, const InputValues& input_values,
                   size_t scale) {
    const size_t kVectorLength = test::Options::vector_lanes<float>();
    // Cover scalar-only, full-vector and tail paths. Interpolation needs
    // adjacent source columns, so include one extra input column.
    for (size_t width :
         {size_t{1}, size_t{2}, size_t{5}, size_t{6}, kVectorLength + 1,
          kVectorLength + 2, 3 * kVectorLength + 1, 3 * kVectorLength + 2}) {
      SCOPED_TRACE(width);
      for (auto [input_padding, output_padding] :
           {std::pair<size_t, size_t>{0, 0}, {7, 5}}) {
        SCOPED_TRACE(input_padding);
        test::Array2D<float> input{width, height, input_padding};
        ASSERT_TRUE(input.valid());
        // Start each layout with the same values, including the random seed.
        auto values = input_values;
        input.fill(values);
        compare_backends(input, output_padding, scale);
      }
    }
  }

  static void compare_backends(const test::Array2D<float>& input,
                               size_t output_padding, size_t scale) {
    static const std::array kBackends = {
        std::pair{"SVE2", &kleidicv::sve2::kleidicv_resize_linear_stripe_f32},
        std::pair{"SME", &kleidicv::sme::kleidicv_resize_linear_stripe_f32},
        std::pair{"SME2", &kleidicv::sme2::kleidicv_resize_linear_stripe_f32},
    };

    // Use Neon as the reference; equality to it compares all four backends.
    test::Array2D<float> reference{input.width() * scale,
                                   input.height() * scale};
    ASSERT_TRUE(reference.valid());
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv::neon::kleidicv_resize_linear_stripe_f32(
                  input.data(), input.stride(), input.width(), input.height(),
                  0, input.height(), reference.data(), reference.stride(),
                  reference.width(), reference.height()));

    for (const auto& [name, function] : kBackends) {
      SCOPED_TRACE(name);
      test::Array2D<float> actual{reference.width(), reference.height(),
                                  output_padding};
      ASSERT_TRUE(actual.valid());
      actual.fill(42);
      ASSERT_EQ(KLEIDICV_OK,
                function(input.data(), input.stride(), input.width(),
                         input.height(), 0, input.height(), actual.data(),
                         actual.stride(), actual.width(), actual.height()));
      for (size_t row = 0; row < reference.height(); ++row) {
        EXPECT_EQ(0, std::memcmp(reference.at(row, 0), actual.at(row, 0),
                                 reference.width() * sizeof(float)))
            << "row " << row;
      }
    }
  }
};

class ResizeLinearBackendComparison : public testing::TestWithParam<size_t> {
 protected:
  void SetUp() override {
    if (!kleidicv::is_sve2_supported() || !kleidicv::is_sme_supported() ||
        !kleidicv::is_sme2_supported()) {
      GTEST_SKIP() << "Backend comparison requires SVE2, SME and SME2";
    }
  }
};

TEST_P(ResizeLinearBackendComparison, RegressionValues) {
  ResizeLinearTestBackendComparison{}.test_regression_values(GetParam());
}

TEST_P(ResizeLinearBackendComparison, RandomValues) {
  ResizeLinearTestBackendComparison{}.test_random_values(GetParam());
}

INSTANTIATE_TEST_SUITE_P(ResizeLinear, ResizeLinearBackendComparison,
                         testing::Values(2, 4, 8));

}  // namespace
