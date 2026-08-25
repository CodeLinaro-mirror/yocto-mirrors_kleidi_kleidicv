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
#include "kleidicv/kleidicv.h"
#include "kleidicv_thread/kleidicv_thread.h"
#include "multithreading_fake.h"
#include "thread_validation.h"

// Tuple of width, height, thread count.
typedef std::tuple<unsigned, unsigned, unsigned> P;

class YuvpThread : public testing::TestWithParam<P> {
 public:
  static inline int get_dcn(kleidicv_color_conversion_t color) {
    return (color & KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA) ? 4 : 3;
  }
  template <typename SingleThreadedFunc, typename MultithreadedFunc>
  void check(SingleThreadedFunc single_threaded_func,
             MultithreadedFunc multithreaded_func,
             kleidicv_color_conversion_t color_format) {
    unsigned width = 0, height = 0, thread_count = 0;
    std::tie(width, height, thread_count) = GetParam();
    size_t channels = get_dcn(color_format);
    test::Array2D<uint8_t> src(width, (height * 3 + 1) / 2),
        dst_single(size_t{width} * channels, height),
        dst_multi(size_t{width} * channels, height);

    test::PseudoRandomNumberGenerator<uint8_t> generator;
    src.fill(generator);

    kleidicv_error_t single_result =
        single_threaded_func(src.data(), src.stride(), dst_single.data(),
                             dst_single.stride(), width, height, color_format);

    kleidicv_error_t multi_result = multithreaded_func(
        src.data(), src.stride(), dst_multi.data(), dst_multi.stride(), width,
        height, color_format, get_multithreading_fake(thread_count));

    EXPECT_EQ(KLEIDICV_OK, single_result);
    EXPECT_EQ(KLEIDICV_OK, multi_result);
    EXPECT_EQ_ARRAY2D(dst_multi, dst_single);
  }

  template <typename Func>
  void run_unsupported(Func impl, kleidicv_color_conversion_t color_format) {
    test::Array2D<uint8_t> src{20, (10 * 3 + 1) / 2};
    size_t channels = get_dcn(color_format);
    test::Array2D<uint8_t> dst{20 * channels, 10, 0, channels};

    EXPECT_EQ(KLEIDICV_OK,
              impl(src.data(), src.stride(), dst.data(), dst.stride(), 0, 1,
                   color_format, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_OK,
              impl(src.data(), src.stride(), dst.data(), dst.stride(), 1, 0,
                   color_format, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              impl(src.data(), src.stride(), dst.data(), dst.stride(),
                   KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, color_format,
                   get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              impl(src.data(), src.stride(), dst.data(), dst.stride(),
                   KLEIDICV_MAX_IMAGE_PIXELS, KLEIDICV_MAX_IMAGE_PIXELS,
                   color_format, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              impl(src.data(), src.stride(), dst.data(), dst.stride(),
                   KLEIDICV_MAX_IMAGE_PIXELS, KLEIDICV_MAX_IMAGE_PIXELS,
                   kleidicv_color_conversion_t{}, get_multithreading_fake(2)));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              impl(src.data(), src.stride(), dst.data(), dst.stride(),
                   KLEIDICV_MAX_IMAGE_PIXELS, KLEIDICV_MAX_IMAGE_PIXELS,
                   static_cast<kleidicv_color_conversion_t>(
                       KLEIDICV_COLOR_CONVERSION_FMT_YUV420P |
                       KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST),
                   get_multithreading_fake(2)));
  }
};

TEST_P(YuvpThread, YV12_TO_BGR) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_YV12_TO_BGR);
}

TEST_P(YuvpThread, YV12_TO_BGRA) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_YV12_TO_BGRA);
}

TEST_P(YuvpThread, YV12_TO_RGB) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_YV12_TO_RGB);
}

TEST_P(YuvpThread, YV12_TO_RGBA) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_YV12_TO_RGBA);
}

TEST_P(YuvpThread, IYUV_TO_BGR) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_IYUV_TO_BGR);
}

TEST_P(YuvpThread, IYUV_TO_BGRA) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_IYUV_TO_BGRA);
}

TEST_P(YuvpThread, IYUV_TO_RGB) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_IYUV_TO_RGB);
}

TEST_P(YuvpThread, IYUV_TO_RGBA) {
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_IYUV_TO_RGBA);
}

TEST_F(YuvpThread, ReturnsErrorForUnsupportedCombinations) {
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_YV12_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_YV12_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_YV12_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_YV12_TO_RGBA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_IYUV_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_IYUV_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_IYUV_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_IYUV_TO_RGBA);
}

INSTANTIATE_TEST_SUITE_P(, YuvpThread,
                         testing::Values(P{1, 1, 1}, P{1, 2, 1}, P{1, 2, 2},
                                         P{2, 1, 2}, P{2, 2, 1}, P{1, 3, 2},
                                         P{2, 3, 1}, P{6, 4, 1}, P{4, 5, 2},
                                         P{2, 6, 3}, P{1, 7, 4}, P{12, 34, 5},
                                         P{12, 37, 5}, P{16, 37, 5},
                                         P{2, 1000, 2}));

namespace {

using Yuv420PToRgbValidation = test::ThreadValidation;

constexpr size_t kLargeWidth = static_cast<size_t>(KLEIDICV_MAX_IMAGE_PIXELS);
constexpr size_t kMaxSize = std::numeric_limits<size_t>::max();

TEST_F(Yuv420PToRgbValidation, PlanarToRgb) {
  uint8_t src = 0;
  uint8_t dst = 0;
  constexpr auto kUnsupportedFormat = static_cast<kleidicv_color_conversion_t>(
      KLEIDICV_COLOR_CONVERSION_FMT_YUV420P |
      KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST);

  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(nullptr, 1, &dst, 1, kLargeWidth, 2,
                                    KLEIDICV_IYUV_TO_RGB, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(&src, 1, nullptr, 1, kLargeWidth, 2,
                                    KLEIDICV_IYUV_TO_RGB, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE, kleidicv_thread_yuv_to_rgb_u8(
                                            &src, 1, &dst, 1, kLargeWidth, 2,
                                            KLEIDICV_IYUV_TO_RGB, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE, kleidicv_thread_yuv_to_rgb_u8(
                                            &src, 1, &dst, 1, 2, kMaxSize,
                                            KLEIDICV_IYUV_TO_RGB, scheduler()));
  for (auto format :
       {kUnsupportedFormat, static_cast<kleidicv_color_conversion_t>(0xFF)}) {
    SCOPED_TRACE(format);
    expect_rejected(
        KLEIDICV_ERROR_NOT_IMPLEMENTED,
        kleidicv_thread_yuv_to_rgb_u8(nullptr, 1, nullptr, 1, kLargeWidth, 2,
                                      format, scheduler()));
  }

  expect_dispatched(
      2, kleidicv_thread_yuv_to_rgb_u8(&src, 1, &dst, 1, 2, 3,
                                       KLEIDICV_IYUV_TO_RGB, scheduler()));
}

TEST_F(Yuv420PToRgbValidation, EmptyImages) {
  constexpr uint8_t kUntouched = 0xFE;
  uint8_t src = 0;
  uint8_t dst = kUntouched;

  for (auto shape :
       {std::array<size_t, 2>{0, 0}, std::array<size_t, 2>{0, 1},
        std::array<size_t, 2>{1, 0}, std::array<size_t, 2>{0, kMaxSize},
        std::array<size_t, 2>{kMaxSize, 0}}) {
    const size_t width = shape[0];
    const size_t height = shape[1];
    for (auto format :
         {KLEIDICV_IYUV_TO_RGB, KLEIDICV_YV12_TO_RGB, KLEIDICV_NV12_TO_RGB,
          KLEIDICV_NV21_TO_RGB, KLEIDICV_YUV444_TO_RGB, KLEIDICV_YUYV_TO_RGB,
          kleidicv_color_conversion_t{},
          static_cast<kleidicv_color_conversion_t>(0xFF)}) {
      SCOPED_TRACE(testing::Message() << "width=" << width << ", height="
                                      << height << ", format=" << format);
      for (auto function :
           {kleidicv_yuv_to_rgb_u8, kleidicv_yuv_to_rgb_u8_sme}) {
        EXPECT_EQ(KLEIDICV_OK, function(&src, kMaxSize, &dst, kMaxSize, width,
                                        height, format));
        EXPECT_EQ(kUntouched, dst);
        EXPECT_EQ(KLEIDICV_OK, function(nullptr, kMaxSize, nullptr, kMaxSize,
                                        width, height, format));
      }
      expect_no_work(kleidicv_thread_yuv_to_rgb_u8(
          &src, kMaxSize, &dst, kMaxSize, width, height, format, scheduler()));
      EXPECT_EQ(kUntouched, dst);
      expect_no_work(kleidicv_thread_yuv_to_rgb_u8(nullptr, kMaxSize, nullptr,
                                                   kMaxSize, width, height,
                                                   format, scheduler()));
    }
  }
}

}  // namespace
