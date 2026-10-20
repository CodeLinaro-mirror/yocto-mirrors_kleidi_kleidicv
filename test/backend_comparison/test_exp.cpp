// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include "framework/array.h"
#include "framework/generator.h"
#include "framework/utils.h"
#include "kleidicv/arithmetics/exp.h"
#include "kleidicv/dispatch.h"

namespace {

template <typename ElementType>
class ExpTestBackendComparison final {
 public:
  void test_regression_values() {
    constexpr size_t kHeight = 3;
    // Check a rounding regression, subnormal output, overflow, and underflow
    // across all vector backends and the Neon tail path.
    for (ElementType value : {-55.1533203125F, -100.07F, 88.947F, -105.31F}) {
      SCOPED_TRACE(value);
      test(kHeight, value);
    }
  }

  void test_mixed_values() {
    const std::vector<ElementType> kInputValues = {
        -55.1533203125F,
        -100.07F,
        88.947F,
        -105.31F,
        -81.012F,
        -47.66F,
        -3.1088F,
        -0.21F,
        0.7F,
        6.2F,
        39.7201F,
        86.11F,
        88.7F,
        0.0F,
        -0.0F,
        std::numeric_limits<ElementType>::denorm_min(),
        -std::numeric_limits<ElementType>::denorm_min(),
        std::numeric_limits<ElementType>::infinity(),
        -std::numeric_limits<ElementType>::infinity(),
        std::numeric_limits<ElementType>::quiet_NaN()};

    test(kInputValues.size(), [&](size_t row, size_t column) {
      return kInputValues[(row + column) % kInputValues.size()];
    });
  }

  void test_random_values() {
    constexpr size_t kHeight = 19;
    test(kHeight, test::PseudoRandomNumberGenerator<ElementType>{});
    test(kHeight, test::PseudoRandomNumberGeneratorFloatRange<ElementType>{
                      -104.0F, 89.0F});
  }

 private:
  template <typename InputValues>
  static void test(size_t height, const InputValues& input_values) {
    // Vector length in elements, as used for array widths.
    const size_t kVectorLength = test::Options::vector_lanes<ElementType>();
    for (size_t width : {size_t{1}, kVectorLength, 3 * kVectorLength - 1}) {
      SCOPED_TRACE(width);
      for (auto [input_padding, output_padding] :
           {std::pair<size_t, size_t>{0, 0}, {7, 5}}) {
        SCOPED_TRACE(input_padding);
        test::Array2D<ElementType> input{width, height, input_padding};
        // Start each layout with the same values, including the random seed.
        auto values = input_values;
        input.fill(values);
        compare_backends(input, output_padding);
      }
    }
  }

  static void compare_backends(const test::Array2D<ElementType>& input,
                               size_t output_padding) {
    static const std::array kBackends = {
        std::pair{"SVE2", &kleidicv::sve2::exp<ElementType>},
        std::pair{"SME", &kleidicv::sme::exp<ElementType>},
        std::pair{"SME2", &kleidicv::sme2::exp<ElementType>},
    };

    // Use Neon as the reference; equality to it compares all four backends.
    test::Array2D<ElementType> reference{input.width(), input.height()};
    ASSERT_TRUE(reference.valid());
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv::neon::exp<ElementType>(
                  input.data(), input.stride(), reference.data(),
                  reference.stride(), input.width(), input.height()));

    for (const auto& [name, function] : kBackends) {
      SCOPED_TRACE(name);
      test::Array2D<ElementType> actual{input.width(), input.height(),
                                        output_padding};
      ASSERT_TRUE(actual.valid());
      actual.fill(42);
      ASSERT_EQ(KLEIDICV_OK,
                function(input.data(), input.stride(), actual.data(),
                         actual.stride(), input.width(), input.height()));
      for (size_t row = 0; row < input.height(); ++row) {
        // Compare representations to detect signed-zero and NaN differences.
        EXPECT_EQ(0, std::memcmp(reference.at(row, 0), actual.at(row, 0),
                                 input.width() * sizeof(ElementType)))
            << "row " << row;
      }
    }
  }
};  // end of class ExpTestBackendComparison<ElementType>

template <typename ElementType>
class ExpBackendComparison : public testing::Test {
 protected:
  void SetUp() override {
    if (!kleidicv::is_sve2_supported() || !kleidicv::is_sme_supported() ||
        !kleidicv::is_sme2_supported()) {
      GTEST_SKIP() << "Backend comparison requires SVE2, SME and SME2";
    }
  }
};  // end of class ExpBackendComparison<ElementType>

using ElementTypes = ::testing::Types<float>;
TYPED_TEST_SUITE(ExpBackendComparison, ElementTypes);

TYPED_TEST(ExpBackendComparison, RegressionValues) {
  ExpTestBackendComparison<TypeParam>{}.test_regression_values();
}

TYPED_TEST(ExpBackendComparison, MixedValues) {
  ExpTestBackendComparison<TypeParam>{}.test_mixed_values();
}

TYPED_TEST(ExpBackendComparison, RandomValues) {
  ExpTestBackendComparison<TypeParam>{}.test_random_values();
}

}  // namespace
