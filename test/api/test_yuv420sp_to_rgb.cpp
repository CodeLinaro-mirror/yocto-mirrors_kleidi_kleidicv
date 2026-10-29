// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <type_traits>

#include "framework/array.h"
#include "framework/utils.h"
#include "kleidicv/conversions/yuv_to_rgb.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/utils.h"
#include "test_config.h"

class YuvSpTest final {
 public:
  YuvSpTest(size_t channel_number, bool switch_blue)
      : channel_number_(channel_number), switch_blue_(switch_blue) {}

  template <typename F>
  void execute_scalar_test(F impl, kleidicv_color_conversion_t color_format) {
    size_t scalar_path_width = 5;
    execute_test(impl, scalar_path_width, color_format, 0);
    // Padding version
    execute_test(impl, scalar_path_width, color_format,
                 test::Options::vector_length());
  }

  template <typename F>
  void execute_vector_test(F impl, kleidicv_color_conversion_t color_format) {
    size_t vector_path_width = (2 * test::Options::vector_lanes<uint8_t>()) - 3;
    execute_test(impl, vector_path_width, color_format, 0);
    // Padding version
    execute_test(impl, vector_path_width, color_format,
                 test::Options::vector_length());
  }

 private:
  using SingleArrayFunc = decltype(&kleidicv_yuv_to_rgb_u8);

  template <typename F>
  void execute_test(F impl, size_t logical_width,
                    kleidicv_color_conversion_t color_format, size_t padding) {
    test::Array2D<uint8_t> input_y{logical_width, 5, padding};
    input_y.fill(96);
    input_y.set(0, 0, {10, 20, 255, 199});
    input_y.set(1, 0, {1, 120, 0, 17});
    input_y.set(2, 0, {2, 3, 240, 228});
    input_y.set(4, 0, {7, 11, 128, 129});

    // the width of the UV input must be even
    test::Array2D<uint8_t> input_uv{
        KLEIDICV_TARGET_NAMESPACE::align_up(logical_width, 2), 3, padding};
    input_uv.fill([](size_t row, size_t column) {
      return static_cast<uint8_t>(64 + (row * 37 + column * 29) % 128);
    });
    input_uv.set(0, 0, {100, 130, 255, 255});
    input_uv.set(1, 0, {0, 1, 3, 4});
    input_uv.set(2, 0, {7, 8, 9, 10});

    test::Array2D<uint8_t> input_yuv{
        input_uv.width(), input_y.height() + input_uv.height(), padding};
    input_yuv.fill(0);
    input_yuv.set(0, 0, &input_y);
    input_yuv.set(input_y.height(), 0, &input_uv);

    test::Array2D<uint8_t> expected{logical_width * channel_number_,
                                    input_y.height(), padding};
    expected.fill(0);
    calculate_expected(input_y, input_uv, expected, color_format);

    test::Array2D<uint8_t> actual{logical_width * channel_number_,
                                  input_y.height(), padding};
    actual.fill(42);

    auto invoke = [&](size_t width, size_t height,
                      kleidicv_color_conversion_t format) {
      if constexpr (std::is_same_v<F, SingleArrayFunc>) {
        return impl(input_yuv.data(), input_yuv.stride(), actual.data(),
                    actual.stride(), width, height, format);
      } else {
        return impl(input_y.data(), input_y.stride(), input_uv.data(),
                    input_uv.stride(), actual.data(), actual.stride(), width,
                    height, format);
      }
    };

    auto err = invoke(logical_width, expected.height(), color_format);

    ASSERT_EQ(KLEIDICV_OK, err);
    EXPECT_EQ_ARRAY2D(expected, actual);

    if constexpr (std::is_same_v<F, SingleArrayFunc>) {
      test::test_null_args(impl, input_yuv.data(), input_yuv.stride(),
                           actual.data(), actual.stride(), logical_width,
                           expected.height(), color_format);
    } else {
      test::test_null_args(impl, input_y.data(), input_y.stride(),
                           input_uv.data(), input_uv.stride(), actual.data(),
                           actual.stride(), logical_width, expected.height(),
                           color_format);
    }

    EXPECT_EQ(KLEIDICV_ERROR_RANGE, invoke(0, 1, color_format));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE, invoke(1, 0, color_format));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              invoke(KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, color_format));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              invoke(KLEIDICV_MAX_IMAGE_PIXELS, KLEIDICV_MAX_IMAGE_PIXELS,
                     color_format));

    EXPECT_EQ(
        KLEIDICV_ERROR_NOT_IMPLEMENTED,
        invoke(actual.width(), actual.height(), kleidicv_color_conversion_t{}));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              invoke(actual.width(), actual.height(),
                     static_cast<kleidicv_color_conversion_t>(
                         KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
                         KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST)));
  }

  void calculate_expected(test::Array2D<uint8_t>& y_arr,
                          test::Array2D<uint8_t>& uv_arr,
                          test::Array2D<uint8_t>& exp_arr,
                          kleidicv_color_conversion_t color_format) const {
    bool is_nv21 = false;
    switch (color_format) {
      case KLEIDICV_NV21_TO_BGR:
      case KLEIDICV_NV21_TO_RGB:
      case KLEIDICV_NV21_TO_BGRA:
      case KLEIDICV_NV21_TO_RGBA:
        is_nv21 = true;
        break;
      case KLEIDICV_NV12_TO_BGR:
      case KLEIDICV_NV12_TO_RGB:
      case KLEIDICV_NV12_TO_BGRA:
      default /* KLEIDICV_NV12_TO_RGBA */:
        is_nv21 = false;
        break;
    }
    for (size_t vindex = 0; vindex < exp_arr.height(); vindex++) {
      for (size_t hindex = 0; hindex < exp_arr.width() / channel_number_;
           hindex++) {
        // NOLINTBEGIN(clang-analyzer-core.uninitialized.Assign)
        int32_t y = *y_arr.at(vindex, hindex);
        // NOLINTEND(clang-analyzer-core.uninitialized.Assign)
        y = std::max(0, y - 16);
        int32_t u =
            *uv_arr.at(vindex / 2, (hindex - hindex % 2) + (is_nv21 ? 1 : 0));
        u -= 128;
        int32_t v =
            *uv_arr.at(vindex / 2, (hindex - hindex % 2) + (is_nv21 ? 0 : 1));
        v -= 128;

        int32_t r = ((1220542 * y) + (1673527 * v) + (1 << 19)) >> 20;
        int32_t g =
            ((1220542 * y) - (409993 * u) - (852492 * v) + (1 << 19)) >> 20;
        int32_t b = ((1220542 * y) + (2116026 * u) + (1 << 19)) >> 20;

        uint8_t r_u8 = saturate_cast_s32_to_u8(r);
        uint8_t g_u8 = saturate_cast_s32_to_u8(g);
        uint8_t b_u8 = saturate_cast_s32_to_u8(b);

        if (switch_blue_) {
          std::swap(b_u8, r_u8);
        }

        if (channel_number_ == 4) {
          exp_arr.set(vindex, hindex * channel_number_,
                      {r_u8, g_u8, b_u8, 0xff});
        } else {
          exp_arr.set(vindex, hindex * channel_number_, {r_u8, g_u8, b_u8});
        }
      }
    }
  }

  static uint8_t saturate_cast_s32_to_u8(int32_t rhs) {
    return static_cast<uint8_t>(
        std::min(std::max(0, rhs),
                 static_cast<int32_t>(std::numeric_limits<uint8_t>::max())));
  }

  size_t channel_number_;
  bool switch_blue_;
};

TEST(YuvSp, NV12_TO_RGB_SCALAR) {
  YuvSpTest yuv_test(3, false);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_RGB);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGB);
}

TEST(YuvSp, NV12_TO_RGB_VECTOR) {
  YuvSpTest yuv_test(3, false);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_RGB);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGB);
}

TEST(YuvSp, NV21_TO_RGB_SCALAR) {
  YuvSpTest yuv_test(3, false);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_RGB);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGB);
}

TEST(YuvSp, NV21_TO_RGB_VECTOR) {
  YuvSpTest yuv_test(3, false);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_RGB);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGB);
}

TEST(YuvSp, NV12_TO_BGR_SCALAR) {
  YuvSpTest yuv_test(3, true);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_BGR);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGR);
}

TEST(YuvSp, NV12_TO_BGR_VECTOR) {
  YuvSpTest yuv_test(3, true);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_BGR);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGR);
}

TEST(YuvSp, NV21_TO_BGR_SCALAR) {
  YuvSpTest yuv_test(3, true);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_BGR);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGR);
}

TEST(YuvSp, NV21_TO_BGR_VECTOR) {
  YuvSpTest yuv_test(3, true);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_BGR);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGR);
}

TEST(YuvSp, NV12_TO_RGBA_SCALAR) {
  YuvSpTest yuv_test(4, false);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_RGBA);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGBA);
}

TEST(YuvSp, NV12_TO_RGBA_VECTOR) {
  YuvSpTest yuv_test(4, false);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_RGBA);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGBA);
}

TEST(YuvSp, NV21_TO_RGBA_SCALAR) {
  YuvSpTest yuv_test(4, false);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_RGBA);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGBA);
}

TEST(YuvSp, NV21_TO_RGBA_VECTOR) {
  YuvSpTest yuv_test(4, false);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_RGBA);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGBA);
}

TEST(YuvSp, NV12_TO_BGRA_SCALAR) {
  YuvSpTest yuv_test(4, true);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_BGRA);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGRA);
}

TEST(YuvSp, NV12_TO_BGRA_VECTOR) {
  YuvSpTest yuv_test(4, true);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV12_TO_BGRA);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGRA);
}

TEST(YuvSp, NV21_TO_BGRA_SCALAR) {
  YuvSpTest yuv_test(4, true);
  yuv_test.execute_scalar_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_BGRA);
  yuv_test.execute_scalar_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGRA);
}

TEST(YuvSp, NV21_TO_BGRA_VECTOR) {
  YuvSpTest yuv_test(4, true);
  yuv_test.execute_vector_test(kleidicv_yuv_semiplanar_to_rgb_u8,
                               KLEIDICV_NV21_TO_BGRA);
  yuv_test.execute_vector_test(kleidicv_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGRA);
}

namespace {

TEST(Yuv420Validation, SemiplanarToRgbPointersAndPrecedence) {
  uint8_t src_y = 0;
  uint8_t src_uv = 0;
  uint8_t dst = 0;

  for (auto function : {kleidicv_yuv_semiplanar_to_rgb_u8,
                        kleidicv_yuv_semiplanar_to_rgb_u8_sme}) {
    test::test_null_args(function, &src_y, 1, &src_uv, 1, &dst, 1,
                         KLEIDICV_MAX_IMAGE_PIXELS, 2, KLEIDICV_NV12_TO_RGB);
    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              function(&src_y, 1, &src_uv, 1, &dst, 1,
                       KLEIDICV_MAX_IMAGE_PIXELS, 2, KLEIDICV_NV12_TO_RGB));
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              function(nullptr, 1, nullptr, 1, nullptr, 1,
                       KLEIDICV_MAX_IMAGE_PIXELS, 2, KLEIDICV_IYUV_TO_RGB));
  }
  for (auto function : {kleidicv_yuv_to_rgb_u8, kleidicv_yuv_to_rgb_u8_sme}) {
    test::test_null_args(function, &src_y, 1, &dst, 1,
                         KLEIDICV_MAX_IMAGE_PIXELS, 2, KLEIDICV_NV12_TO_RGB);
    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              function(&src_y, 1, &dst, 1, KLEIDICV_MAX_IMAGE_PIXELS, 2,
                       KLEIDICV_NV12_TO_RGB));
  }
  // Even if the packed UV address wraps to zero, a non-null source must
  // retain the normal format -> pointers -> image-size error precedence.
  const size_t wrapping_stride =
      uintptr_t{0} - reinterpret_cast<uintptr_t>(&src_y);
  constexpr auto kUnsupportedFormat = static_cast<kleidicv_color_conversion_t>(
      KLEIDICV_NV12_TO_RGB | KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST);
  for (auto function : {kleidicv_yuv_to_rgb_u8, kleidicv_yuv_to_rgb_u8_sme}) {
    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              function(&src_y, wrapping_stride, &dst, 1,
                       KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, KLEIDICV_NV12_TO_RGB));
    EXPECT_EQ(KLEIDICV_ERROR_NULL_POINTER,
              function(&src_y, wrapping_stride, nullptr, 1,
                       KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, KLEIDICV_NV12_TO_RGB));
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              function(&src_y, wrapping_stride, nullptr, 1,
                       KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, kUnsupportedFormat));
  }
  for (unsigned flags :
       {static_cast<unsigned>(KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST),
        0xFFU}) {
    SCOPED_TRACE(flags);
    const auto format =
        static_cast<kleidicv_color_conversion_t>(KLEIDICV_NV12_TO_RGB | flags);
    for (auto function : {kleidicv_yuv_semiplanar_to_rgb_u8,
                          kleidicv_yuv_semiplanar_to_rgb_u8_sme}) {
      EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                function(nullptr, 1, nullptr, 1, nullptr, 1,
                         KLEIDICV_MAX_IMAGE_PIXELS, 2, format));
    }
    for (auto function : {kleidicv_yuv_to_rgb_u8, kleidicv_yuv_to_rgb_u8_sme}) {
      EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                function(nullptr, 1, nullptr, 1, KLEIDICV_MAX_IMAGE_PIXELS, 2,
                         format));
    }
  }
}

}  // namespace

TEST(YuvSpToRgb, EmptyImages) {
  constexpr size_t kMaxSize = std::numeric_limits<size_t>::max();
  constexpr uint8_t kUntouched = 0xFE;

  for (auto shape :
       {std::array<size_t, 2>{0, 0}, std::array<size_t, 2>{0, 1},
        std::array<size_t, 2>{1, 0}, std::array<size_t, 2>{0, kMaxSize},
        std::array<size_t, 2>{kMaxSize, 0}}) {
    const size_t width = shape[0];
    const size_t height = shape[1];
    SCOPED_TRACE(testing::Message()
                 << "width=" << width << ", height=" << height);
    const uint8_t src_y = 0;
    const uint8_t src_uv = 0;
    uint8_t dst = kUntouched;

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              kleidicv_yuv_semiplanar_to_rgb_u8(&src_y, kMaxSize, &src_uv,
                                                kMaxSize, &dst, kMaxSize, width,
                                                height, KLEIDICV_NV12_TO_RGB));
    EXPECT_EQ(kUntouched, dst);
    EXPECT_EQ(KLEIDICV_ERROR_NULL_POINTER,
              kleidicv_yuv_semiplanar_to_rgb_u8(
                  nullptr, kMaxSize, nullptr, kMaxSize, nullptr, kMaxSize,
                  width, height, KLEIDICV_NV12_TO_RGB));
  }
}
