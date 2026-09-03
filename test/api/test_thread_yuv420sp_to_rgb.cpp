// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <thread>
#include <type_traits>

#include "framework/array.h"
#include "framework/generator.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv_thread/kleidicv_thread.h"
#include "multithreading_fake.h"
#include "thread_validation.h"

// Tuple of width, height, thread count.
typedef std::tuple<unsigned, unsigned, unsigned> P;

class YuvSpThread : public testing::TestWithParam<P> {
 private:
  using SingleArrayFunc = decltype(&kleidicv_thread_yuv_to_rgb_u8);

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
    test::Array2D<uint8_t> dst_single(size_t{width} * channels, height),
        dst_multi(size_t{width} * channels, height);

    test::PseudoRandomNumberGenerator<uint8_t> generator;
    kleidicv_error_t single_result = KLEIDICV_ERROR_NOT_IMPLEMENTED;
    kleidicv_error_t multi_result = KLEIDICV_ERROR_NOT_IMPLEMENTED;

    if constexpr (std::is_same_v<MultithreadedFunc, SingleArrayFunc>) {
      size_t src_width = (width + 1) & ~size_t{1};
      test::Array2D<uint8_t> src(src_width, height + (height + 1) / 2);
      src.fill(generator);

      single_result = single_threaded_func(
          src.data(), src.stride(), dst_single.data(), dst_single.stride(),
          width, height, color_format);
      multi_result = multithreaded_func(
          src.data(), src.stride(), dst_multi.data(), dst_multi.stride(), width,
          height, color_format, get_multithreading_fake(thread_count));
    } else {
      test::Array2D<uint8_t> src_y(width, height);
      test::Array2D<uint8_t> src_uv((width + 1) & ~1U, (height + 1) / 2);
      src_y.fill(generator);
      src_uv.fill(generator);

      single_result = single_threaded_func(
          src_y.data(), src_y.stride(), src_uv.data(), src_uv.stride(),
          dst_single.data(), dst_single.stride(), width, height, color_format);
      multi_result = multithreaded_func(
          src_y.data(), src_y.stride(), src_uv.data(), src_uv.stride(),
          dst_multi.data(), dst_multi.stride(), width, height, color_format,
          get_multithreading_fake(thread_count));
    }

    EXPECT_EQ(KLEIDICV_OK, single_result);
    EXPECT_EQ(KLEIDICV_OK, multi_result);
    EXPECT_EQ_ARRAY2D(dst_multi, dst_single);
  }

  template <typename Func>
  void run_unsupported(Func impl, kleidicv_color_conversion_t color_format) {
    size_t channels = get_dcn(color_format);
    test::Array2D<uint8_t> src_y{20, 10};
    test::Array2D<uint8_t> src_uv{20, 5};
    test::Array2D<uint8_t> src{20, 15};
    test::Array2D<uint8_t> dst{20 * channels, 10, 0, channels};

    auto invoke = [&](size_t width, size_t height,
                      kleidicv_color_conversion_t format,
                      unsigned thread_count) {
      if constexpr (std::is_same_v<Func, SingleArrayFunc>) {
        return impl(src.data(), src.stride(), dst.data(), dst.stride(), width,
                    height, format, get_multithreading_fake(thread_count));
      } else {
        return impl(src_y.data(), src_y.stride(), src_uv.data(),
                    src_uv.stride(), dst.data(), dst.stride(), width, height,
                    format, get_multithreading_fake(thread_count));
      }
    };

    EXPECT_EQ(KLEIDICV_OK, invoke(0, 1, color_format, 2));

    EXPECT_EQ(KLEIDICV_OK, invoke(1, 0, color_format, 2));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              invoke(KLEIDICV_MAX_IMAGE_PIXELS + 1, 1, color_format, 2));

    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              invoke(KLEIDICV_MAX_IMAGE_PIXELS, KLEIDICV_MAX_IMAGE_PIXELS - 1,
                     color_format, 1));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              invoke(20, 10, kleidicv_color_conversion_t{}, 2));

    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              invoke(20, 10,
                     static_cast<kleidicv_color_conversion_t>(
                         KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
                         KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST),
                     2));
  }
};

TEST_P(YuvSpThread, NV12_TO_BGR) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV12_TO_BGR);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV12_TO_BGR);
}

TEST_P(YuvSpThread, NV21_TO_BGR) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV21_TO_BGR);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV21_TO_BGR);
}

TEST_P(YuvSpThread, NV12_TO_BGRA) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV12_TO_BGRA);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV12_TO_BGRA);
}

TEST_P(YuvSpThread, NV21_TO_BGRA) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV21_TO_BGRA);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV21_TO_BGRA);
}

TEST_P(YuvSpThread, NV12_TO_RGB) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV12_TO_RGB);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV12_TO_RGB);
}

TEST_P(YuvSpThread, NV21_TO_RGB) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV21_TO_RGB);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV21_TO_RGB);
}

TEST_P(YuvSpThread, NV12_TO_RGBA) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV12_TO_RGBA);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV12_TO_RGBA);
}

TEST_P(YuvSpThread, NV21_TO_RGBA) {
  check(kleidicv_yuv_semiplanar_to_rgb_u8,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8, KLEIDICV_NV21_TO_RGBA);
  check(kleidicv_yuv_to_rgb_u8, kleidicv_thread_yuv_to_rgb_u8,
        KLEIDICV_NV21_TO_RGBA);
}

TEST_F(YuvSpThread, NullPointers) {
  constexpr size_t kWidth = 20;
  constexpr size_t kHeight = 10;
  test::Array2D<uint8_t> src_y{kWidth, kHeight},
      src_uv{kWidth, (kHeight + 1) / 2},
      src{kWidth, kHeight + (kHeight + 1) / 2}, dst{kWidth * 3, kHeight};
  const auto mt = get_multithreading_fake(2);

  test::test_null_args(kleidicv_thread_yuv_semiplanar_to_rgb_u8, src_y.data(),
                       src_y.stride(), src_uv.data(), src_uv.stride(),
                       dst.data(), dst.stride(), kWidth, kHeight,
                       KLEIDICV_NV12_TO_BGR, mt);

  test::test_null_args(kleidicv_thread_yuv_to_rgb_u8, src.data(), src.stride(),
                       dst.data(), dst.stride(), kWidth, kHeight,
                       KLEIDICV_NV12_TO_BGR, mt);
}

TEST_F(YuvSpThread, ReturnsErrorForUnsupportedCombinations) {
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV12_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV12_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV12_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV12_TO_RGBA);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV21_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV21_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV21_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_semiplanar_to_rgb_u8,
                  KLEIDICV_NV21_TO_RGBA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV12_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV12_TO_RGBA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGR);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGB);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV21_TO_BGRA);
  run_unsupported(kleidicv_thread_yuv_to_rgb_u8, KLEIDICV_NV21_TO_RGBA);
}

INSTANTIATE_TEST_SUITE_P(, YuvSpThread,
                         testing::Values(P{1, 1, 1}, P{1, 2, 1}, P{1, 2, 2},
                                         P{2, 1, 2}, P{2, 2, 1}, P{1, 3, 2},
                                         P{2, 3, 1}, P{6, 4, 1}, P{4, 5, 2},
                                         P{2, 6, 3}, P{1, 7, 4}, P{12, 34, 5}));

namespace {

using Yuv420SpToRgbValidation = test::ThreadValidation;

constexpr size_t kLargeWidth = static_cast<size_t>(KLEIDICV_MAX_IMAGE_PIXELS);
constexpr size_t kMaxSize = std::numeric_limits<size_t>::max();

TEST_F(Yuv420SpToRgbValidation, PackedSemiplanarToRgb) {
  uint8_t src = 0;
  uint8_t dst = 0;
  constexpr auto kUnsupportedFormat = static_cast<kleidicv_color_conversion_t>(
      KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
      KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST);

  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(nullptr, 1, &dst, 1, 2, 2,
                                    KLEIDICV_NV12_TO_RGB, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(&src, 1, nullptr, 1, 2, 2,
                                    KLEIDICV_NV12_TO_RGB, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE, kleidicv_thread_yuv_to_rgb_u8(
                                            &src, 1, &dst, 1, kLargeWidth, 2,
                                            KLEIDICV_NV12_TO_RGB, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE, kleidicv_thread_yuv_to_rgb_u8(
                                            &src, 1, &dst, 1, 2, kMaxSize,
                                            KLEIDICV_NV12_TO_RGB, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(nullptr, 1, &dst, 1, 2, 2,
                                    kUnsupportedFormat, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_yuv_to_rgb_u8(&src, 1, nullptr, 1, 2, 2,
                                    kUnsupportedFormat, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NOT_IMPLEMENTED,
      kleidicv_thread_yuv_to_rgb_u8(&src, 1, &dst, 1, kLargeWidth, 2,
                                    kUnsupportedFormat, scheduler()));
}

TEST_F(Yuv420SpToRgbValidation, SplitSemiplanarToRgb) {
  uint8_t src_y = 0;
  uint8_t src_uv = 0;
  uint8_t dst = 0;
  constexpr auto kUnsupportedFormat = static_cast<kleidicv_color_conversion_t>(
      KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP |
      KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST);

  for (auto format : {KLEIDICV_NV12_TO_RGB, kUnsupportedFormat}) {
    expect_rejected(
        KLEIDICV_ERROR_NULL_POINTER,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8(nullptr, 1, &src_uv, 1, &dst,
                                                 1, 2, 2, format, scheduler()));
    expect_rejected(
        KLEIDICV_ERROR_NULL_POINTER,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8(&src_y, 1, nullptr, 1, &dst, 1,
                                                 2, 2, format, scheduler()));
    expect_rejected(
        KLEIDICV_ERROR_NULL_POINTER,
        kleidicv_thread_yuv_semiplanar_to_rgb_u8(&src_y, 1, &src_uv, 1, nullptr,
                                                 1, 2, 2, format, scheduler()));
  }

  // Each two-row slice fits the image-size limit, but the complete image does
  // not. Reject it before the scheduler can split it into valid-sized slices.
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_yuv_semiplanar_to_rgb_u8(
                      &src_y, 1, &src_uv, 1, &dst, 1, kLargeWidth / 2, 4,
                      KLEIDICV_NV12_TO_RGB, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                  kleidicv_thread_yuv_semiplanar_to_rgb_u8(
                      &src_y, 1, &src_uv, 1, &dst, 1, kLargeWidth, 2,
                      kUnsupportedFormat, scheduler()));
  expect_dispatched(2, kleidicv_thread_yuv_semiplanar_to_rgb_u8(
                           &src_y, 1, &src_uv, 1, &dst, 1, 2, 3,
                           KLEIDICV_NV12_TO_RGB, scheduler()));
}

TEST_F(Yuv420SpToRgbValidation, EmptyImages) {
  constexpr uint8_t kUntouched = 0xFE;
  uint8_t src_y = 0;
  uint8_t src_uv = 0;
  uint8_t dst = kUntouched;

  for (auto shape :
       {std::array<size_t, 2>{0, 0}, std::array<size_t, 2>{0, 1},
        std::array<size_t, 2>{1, 0}, std::array<size_t, 2>{0, kMaxSize},
        std::array<size_t, 2>{kMaxSize, 0}}) {
    const size_t width = shape[0];
    const size_t height = shape[1];
    for (auto format : {KLEIDICV_NV12_TO_RGB, KLEIDICV_NV21_TO_RGB,
                        kleidicv_color_conversion_t{}}) {
      SCOPED_TRACE(testing::Message() << "width=" << width << ", height="
                                      << height << ", format=" << format);
      for (auto function : {kleidicv_yuv_semiplanar_to_rgb_u8,
                            kleidicv_yuv_semiplanar_to_rgb_u8_sme}) {
        EXPECT_EQ(KLEIDICV_OK, function(&src_y, kMaxSize, &src_uv, kMaxSize,
                                        &dst, kMaxSize, width, height, format));
        EXPECT_EQ(kUntouched, dst);
        EXPECT_EQ(KLEIDICV_OK,
                  function(nullptr, kMaxSize, nullptr, kMaxSize, nullptr,
                           kMaxSize, width, height, format));
      }
      expect_no_work(kleidicv_thread_yuv_semiplanar_to_rgb_u8(
          &src_y, kMaxSize, &src_uv, kMaxSize, &dst, kMaxSize, width, height,
          format, scheduler()));
      EXPECT_EQ(kUntouched, dst);
      expect_no_work(kleidicv_thread_yuv_semiplanar_to_rgb_u8(
          nullptr, kMaxSize, nullptr, kMaxSize, nullptr, kMaxSize, width,
          height, format, scheduler()));
    }
  }
}

TEST(Yuv420StripeWorkers, SplitSemiplanarToRgbHonorsChromaRowRange) {
  constexpr size_t kWidth = 18;
  constexpr size_t kHeight = 7;
  constexpr size_t kChromaHeight = 4;
  constexpr size_t kYStride = 20;
  constexpr size_t kUvStride = 24;
  constexpr size_t kDstStride = kWidth * 4 + 4;
  constexpr uint8_t kUntouched = 0xFE;
  std::array<uint8_t, kYStride * kHeight> src_y = {};
  std::array<uint8_t, kUvStride * kChromaHeight> src_uv = {};
  std::array<uint8_t, kDstStride * kHeight> expected = {};
  std::array<uint8_t, kDstStride * kHeight> actual = {};

  for (size_t index = 0; index < src_y.size(); ++index) {
    src_y[index] = static_cast<uint8_t>(index);
  }
  for (size_t index = 0; index < src_uv.size(); ++index) {
    src_uv[index] = static_cast<uint8_t>(3 * index + 17);
  }

  for (auto format : {KLEIDICV_NV12_TO_RGB, KLEIDICV_NV21_TO_RGBA}) {
    expected.fill(kUntouched);
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv_yuv_semiplanar_to_rgb_u8(
                  src_y.data(), kYStride, src_uv.data(), kUvStride,
                  expected.data(), kDstStride, kWidth, kHeight, format));

    // Exercise both an interior stripe and a stripe ending on the odd last row.
    for (size_t end : {size_t{3}, kChromaHeight}) {
      actual.fill(kUntouched);
      auto expected_partial = actual;

      const size_t row_end = end == kChromaHeight ? kHeight : end * 2;
      for (size_t row = 2; row < row_end; ++row) {
        for (size_t column = 0; column < kDstStride; ++column) {
          size_t index = row * kDstStride + column;
          expected_partial[index] = expected[index];
        }
      }
      struct StripeContext {
        size_t end;
        decltype(actual) *actual_output;
        const decltype(expected_partial) *partial_output;
      } context{end, &actual, &expected_partial};
      const kleidicv_thread_multithreading mt{
          [](kleidicv_thread_callback callback, void *callback_data,
             void *parallel_data, size_t task_count) {
            EXPECT_EQ(size_t{kChromaHeight}, task_count);
            const auto &context =
                *static_cast<const StripeContext *>(parallel_data);
            if (auto error = callback(1, context.end, callback_data)) {
              return error;
            }
            EXPECT_EQ(*context.partial_output, *context.actual_output);
            if (auto error = callback(0, 1, callback_data)) {
              return error;
            }
            return context.end == task_count
                       ? KLEIDICV_OK
                       : callback(context.end, task_count, callback_data);
          },
          &context};
      ASSERT_EQ(KLEIDICV_OK,
                kleidicv_thread_yuv_semiplanar_to_rgb_u8(
                    src_y.data(), kYStride, src_uv.data(), kUvStride,
                    actual.data(), kDstStride, kWidth, kHeight, format, mt));
      EXPECT_EQ(expected, actual);
    }
  }
}

}  // namespace

namespace {

struct ChromaRowScheduler {
  size_t expected_tasks = 0;
  size_t callback_count = 0;

  kleidicv_thread_multithreading scheduler() {
    callback_count = 0;
    return {[](kleidicv_thread_callback callback, void *callback_data,
               void *parallel_data, size_t task_count) {
              auto &state = *static_cast<ChromaRowScheduler *>(parallel_data);
              EXPECT_EQ(state.expected_tasks, task_count);
              // Each callback processes one non-empty chroma-row range.
              for (size_t begin = 0; begin < task_count; ++begin) {
                ++state.callback_count;
                if (auto error = callback(begin, begin + 1, callback_data)) {
                  return error;
                }
              }
              return KLEIDICV_OK;
            },
            this};
  }
};

TEST(Yuv420SpScheduling, ChromaRowRangesMatchPublicApi) {
  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 3;
  constexpr size_t kSrcStride = 4;
  constexpr size_t kDstStride = 8;
  constexpr uint8_t kUntouched = 0xFE;
  std::array<uint8_t, kSrcStride *(kHeight + 2)> src{};
  std::array<uint8_t, kDstStride * kHeight> expected{};
  std::array<uint8_t, kDstStride * kHeight> actual{};
  for (size_t i = 0; i < src.size(); ++i) {
    src[i] = static_cast<uint8_t>(17 * i);
  }

  for (auto format : {KLEIDICV_NV12_TO_RGB, KLEIDICV_NV21_TO_RGBA}) {
    expected.fill(kUntouched);
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv_yuv_to_rgb_u8(src.data(), kSrcStride, expected.data(),
                                     kDstStride, kWidth, kHeight, format));
    ChromaRowScheduler state{2};
    actual.fill(kUntouched);
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv_thread_yuv_to_rgb_u8(
                  src.data(), kSrcStride, actual.data(), kDstStride, kWidth,
                  kHeight, format, state.scheduler()));
    EXPECT_EQ(state.expected_tasks, state.callback_count);
    EXPECT_EQ(expected, actual);
    actual.fill(kUntouched);
    ASSERT_EQ(KLEIDICV_OK,
              kleidicv_thread_yuv_semiplanar_to_rgb_u8(
                  src.data(), kSrcStride, src.data() + kSrcStride * kHeight,
                  kSrcStride, actual.data(), kDstStride, kWidth, kHeight,
                  format, state.scheduler()));
    EXPECT_EQ(state.expected_tasks, state.callback_count);
    EXPECT_EQ(expected, actual);
  }
}

}  // namespace
