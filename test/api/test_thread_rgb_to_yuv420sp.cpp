// SPDX-FileCopyrightText: 2025 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <thread>

#include "framework/array.h"
#include "framework/generator.h"
#include "framework/utils.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/utils.h"
#include "kleidicv_thread/kleidicv_thread.h"
#include "multithreading_fake.h"
#include "test_config.h"
#include "thread_validation.h"

// Tuple of width, height, thread count.
typedef std::tuple<unsigned, unsigned, unsigned> P;

class RgbToYuv420SpThread : public testing::TestWithParam<P> {
 public:
  static inline int get_scn(kleidicv_color_conversion_t color) {
    return (color & KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA) ? 4 : 3;
  }
  template <typename SingleThreadedFunc, typename MultithreadedFunc>
  void check(SingleThreadedFunc single_threaded_func,
             MultithreadedFunc multithreaded_func,
             kleidicv_color_conversion_t color_format) {
    unsigned width = 0, height = 0, thread_count = 0;
    std::tie(width, height, thread_count) = GetParam();
    size_t channels = get_scn(color_format);
    test::Array2D<uint8_t> src(size_t{width} * channels, height, channels),
        y_dst_single(width, height),
        uv_dst_single(KLEIDICV_TARGET_NAMESPACE::align_up(width, 2),
                      (height + 1) / 2),
        y_dst_multi(width, height),
        uv_dst_multi(KLEIDICV_TARGET_NAMESPACE::align_up(width, 2),
                     (height + 1) / 2);

    test::PseudoRandomNumberGenerator<uint8_t> generator;
    src.fill(generator);

    kleidicv_error_t single_result = single_threaded_func(
        src.data(), src.stride(), y_dst_single.data(), y_dst_single.stride(),
        uv_dst_single.data(), uv_dst_single.stride(), width, height,
        color_format);

    kleidicv_error_t multi_result = multithreaded_func(
        src.data(), src.stride(), y_dst_multi.data(), y_dst_multi.stride(),
        uv_dst_multi.data(), uv_dst_multi.stride(), width, height, color_format,
        get_multithreading_fake(thread_count));

    EXPECT_EQ(KLEIDICV_OK, single_result);
    EXPECT_EQ(KLEIDICV_OK, multi_result);
    EXPECT_EQ_ARRAY2D(y_dst_multi, y_dst_single);
    EXPECT_EQ_ARRAY2D(uv_dst_multi, uv_dst_single);
  }

  template <typename Func>
  void run_unsupported(Func impl, kleidicv_color_conversion_t color_format) {
    size_t channels = get_scn(color_format);
    test::Array2D<uint8_t> src{20 * channels, 10, 0, channels};
    test::Array2D<uint8_t> y_dst(20, 10),
        uv_dst(KLEIDICV_TARGET_NAMESPACE::align_up(20, 2), (10 + 1) / 2);

    EXPECT_EQ(KLEIDICV_OK,
              impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
                   uv_dst.data(), uv_dst.stride(), 0, 1, color_format,
                   get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_OK,
              impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
                   uv_dst.data(), uv_dst.stride(), 1, 0, color_format,
                   get_multithreading_fake(2)));

    EXPECT_EQ(
        KLEIDICV_ERROR_RANGE,
        impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
             uv_dst.data(), uv_dst.stride(), KLEIDICV_MAX_IMAGE_PIXELS + 1, 1,
             color_format, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
                   uv_dst.data(), uv_dst.stride(), KLEIDICV_MAX_IMAGE_PIXELS,
                   KLEIDICV_MAX_IMAGE_PIXELS, color_format,
                   get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
                   uv_dst.data(), uv_dst.stride(), src.width(), src.height(),
                   kleidicv_color_conversion_t{}, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              impl(src.data(), src.stride(), y_dst.data(), y_dst.stride(),
                   uv_dst.data(), uv_dst.stride(), src.width(), src.height(),
                   static_cast<kleidicv_color_conversion_t>(
                       KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
                       KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST),
                   get_multithreading_fake(2)));
  }
};

TEST_P(RgbToYuv420SpThread, BGR_TO_NV21) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_BGR_TO_NV21);
}

TEST_P(RgbToYuv420SpThread, RGB_TO_NV21) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_RGB_TO_NV21);
}

TEST_P(RgbToYuv420SpThread, BGR_TO_NV12) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_BGR_TO_NV12);
}

TEST_P(RgbToYuv420SpThread, RGB_TO_NV1) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_RGB_TO_NV12);
}

TEST_P(RgbToYuv420SpThread, BGRA_TO_NV21) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_BGRA_TO_NV21);
}

TEST_P(RgbToYuv420SpThread, RGBA_TO_NV21) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_RGBA_TO_NV21);
}

TEST_P(RgbToYuv420SpThread, BGRA_TO_NV12) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_BGRA_TO_NV12);
}

TEST_P(RgbToYuv420SpThread, RGBA_TO_NV1) {
  check(kleidicv_rgb_to_yuv_semiplanar_u8,
        kleidicv_thread_rgb_to_yuv_semiplanar_u8, KLEIDICV_RGBA_TO_NV12);
}

TEST_F(RgbToYuv420SpThread, ReturnsErrorForUnsupportedCombinations) {
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_RGB_TO_NV12);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_BGR_TO_NV12);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_RGBA_TO_NV12);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_BGRA_TO_NV12);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_RGB_TO_NV21);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_BGR_TO_NV21);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_RGBA_TO_NV21);
  run_unsupported(kleidicv_thread_rgb_to_yuv_semiplanar_u8,
                  KLEIDICV_BGRA_TO_NV21);
}

INSTANTIATE_TEST_SUITE_P(, RgbToYuv420SpThread,
                         testing::Values(P{1, 1, 1}, P{1, 2, 1}, P{1, 2, 2},
                                         P{2, 1, 2}, P{2, 2, 1}, P{1, 3, 2},
                                         P{2, 3, 1}, P{6, 4, 1}, P{4, 5, 2},
                                         P{2, 6, 3}, P{1, 7, 4}, P{12, 34, 5},
                                         P{12, 37, 5}, P{2, 1000, 2}));

namespace {

using RgbToYuv420SpValidation = test::ThreadValidation;

constexpr size_t kLargeWidth = static_cast<size_t>(KLEIDICV_MAX_IMAGE_PIXELS);
constexpr size_t kMaxSize = std::numeric_limits<size_t>::max();

TEST_F(RgbToYuv420SpValidation, RgbToSemiplanar) {
  uint8_t src = 0;
  uint8_t y_dst = 0;
  uint8_t uv_dst = 0;
  constexpr auto kUnsupportedFormat = static_cast<kleidicv_color_conversion_t>(
      KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
      KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST);

  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      nullptr, 1, &y_dst, 1, &uv_dst, 1, 2, 2,
                      KLEIDICV_RGB_TO_NV12, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, nullptr, 1, &uv_dst, 1, 2, 2,
                      KLEIDICV_RGB_TO_NV12, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, &y_dst, 1, nullptr, 1, 2, 2,
                      KLEIDICV_RGB_TO_NV12, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, &y_dst, 1, &uv_dst, 1, kLargeWidth, 2,
                      KLEIDICV_RGB_TO_NV12, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, &y_dst, 1, &uv_dst, 1, 2, kMaxSize,
                      KLEIDICV_RGB_TO_NV12, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, &y_dst, 1, &uv_dst, 1, 2, 2, kUnsupportedFormat,
                      scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      &src, 1, &y_dst, 1, &uv_dst, 1, kLargeWidth, 2,
                      kUnsupportedFormat, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                      nullptr, 1, &y_dst, 1, &uv_dst, 1, 2, 2,
                      kUnsupportedFormat, scheduler()));
  expect_dispatched(2, kleidicv_thread_rgb_to_yuv_semiplanar_u8(
                           &src, 1, &y_dst, 1, &uv_dst, 1, 2, 3,
                           KLEIDICV_RGB_TO_NV12, scheduler()));
}

TEST_F(RgbToYuv420SpValidation, EmptyImages) {
  constexpr uint8_t kUntouched = 0xFE;
  uint8_t src = 0;
  uint8_t y_dst = kUntouched;
  uint8_t uv_dst = kUntouched;

  for (auto shape :
       {std::array<size_t, 2>{0, 0}, std::array<size_t, 2>{0, 1},
        std::array<size_t, 2>{1, 0}, std::array<size_t, 2>{0, kMaxSize},
        std::array<size_t, 2>{kMaxSize, 0}}) {
    const size_t width = shape[0];
    const size_t height = shape[1];
    for (auto format : {KLEIDICV_RGB_TO_NV12, KLEIDICV_RGB_TO_NV21,
                        kleidicv_color_conversion_t{}}) {
      SCOPED_TRACE(testing::Message() << "width=" << width << ", height="
                                      << height << ", format=" << format);
      for (auto function : {kleidicv_rgb_to_yuv_semiplanar_u8,
                            kleidicv_rgb_to_yuv_semiplanar_u8_sme}) {
        EXPECT_EQ(KLEIDICV_OK,
                  function(&src, kMaxSize, &y_dst, kMaxSize, &uv_dst, kMaxSize,
                           width, height, format));
        EXPECT_EQ(kUntouched, y_dst);
        EXPECT_EQ(kUntouched, uv_dst);
        EXPECT_EQ(KLEIDICV_OK,
                  function(nullptr, kMaxSize, nullptr, kMaxSize, nullptr,
                           kMaxSize, width, height, format));
      }
      expect_no_work(kleidicv_thread_rgb_to_yuv_semiplanar_u8(
          &src, kMaxSize, &y_dst, kMaxSize, &uv_dst, kMaxSize, width, height,
          format, scheduler()));
      EXPECT_EQ(kUntouched, y_dst);
      EXPECT_EQ(kUntouched, uv_dst);
      expect_no_work(kleidicv_thread_rgb_to_yuv_semiplanar_u8(
          nullptr, kMaxSize, nullptr, kMaxSize, nullptr, kMaxSize, width,
          height, format, scheduler()));
    }
  }
}

}  // namespace
