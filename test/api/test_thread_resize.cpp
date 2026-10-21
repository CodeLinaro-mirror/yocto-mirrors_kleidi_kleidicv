// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gtest/gtest.h>

#include <array>
#include <functional>

#include "framework/array.h"
#include "framework/generator.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv_thread/kleidicv_thread.h"
#include "multithreading_fake.h"
#include "thread_validation.h"

// Tuple of width, height, thread count, channel count.
typedef std::tuple<size_t, size_t, size_t, size_t> P;

using ResizeThreadValidation = test::ThreadValidation;

class ResizeThread : public testing::TestWithParam<P> {
 public:
  void test_resize_to_quarter() {
    size_t dst_width = 0, dst_height = 0, thread_count = 0, channels = 0;
    std::tie(dst_width, dst_height, thread_count, channels) = GetParam();
    check<uint8_t>(kleidicv_resize_linear_u8, kleidicv_thread_resize_linear_u8,
                   thread_count, 2 * dst_width, 2 * dst_height, channels,
                   dst_width, dst_height);
  }
  void test_resize_u8_2x2() {
    size_t src_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(src_width, src_height, thread_count, channels) = GetParam();
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, 2 * src_width, 2 * src_height);
    }
  }
  void test_resize_u8_4x4() {
    size_t src_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(src_width, src_height, thread_count, channels) = GetParam();
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, 4 * src_width, 4 * src_height);
    }
  }
  void test_resize_u8_up_1_05() {
    size_t test_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(test_width, src_height, thread_count, channels) = GetParam();
    size_t src_width = 16 + test_width, dst_width = 21 * src_width / 20;
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, dst_width, src_height + 1);
    }
  }
  void test_resize_u8_up_1_05_vertical_down() {
    size_t test_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(test_width, src_height, thread_count, channels) = GetParam();
    size_t src_width = 16 + test_width, dst_width = 21 * src_width / 20;
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, dst_width,
                     std::max<size_t>(1, src_height - 1));
    }
  }
  void test_resize_u8_up_1_6() {
    size_t test_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(test_width, src_height, thread_count, channels) = GetParam();
    size_t src_width = 16 + test_width, dst_width = 8 * src_width / 5;
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, dst_width, src_height + 3);
    }
  }
  void test_resize_u8_up_1_6_vertical_down() {
    size_t test_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(test_width, src_height, thread_count, channels) = GetParam();
    size_t src_width = 16 + test_width, dst_width = 8 * src_width / 5;
    if (channels <= 1) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, src_width,
                     src_height, channels, dst_width,
                     std::max<size_t>(4, src_height) - 3);
    }
  }
  void test_resize_u8_down2() {
    size_t width = 0, height = 0, thread_count = 0, channels = 0;
    std::tie(width, height, thread_count, channels) = GetParam();
    if (channels <= 3) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, 16 * width,
                     3 * height, channels, 9 * width, 2 * height);
    }
  }
  void test_resize_u8_down3() {
    size_t width = 0, height = 0, thread_count = 0, channels = 0;
    std::tie(width, height, thread_count, channels) = GetParam();
    if (channels <= 3) {
      check<uint8_t>(kleidicv_resize_linear_u8,
                     kleidicv_thread_resize_linear_u8, thread_count, 32 * width,
                     3 * height, channels, 13 * width, 2 * height);
    }
  }

  void test_resize_f32_2x2() {
    size_t src_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(src_width, src_height, thread_count, channels) = GetParam();
    if (channels <= 1) {
      check<float>(kleidicv_resize_linear_f32,
                   kleidicv_thread_resize_linear_f32, thread_count, src_width,
                   src_height, channels, 2 * src_width, 2 * src_height);
    }
  }
  void test_resize_f32_4x4() {
    size_t src_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(src_width, src_height, thread_count, channels) = GetParam();
    if (channels <= 1) {
      check<float>(kleidicv_resize_linear_f32,
                   kleidicv_thread_resize_linear_f32, thread_count, src_width,
                   src_height, 1, 4 * src_width, 4 * src_height);
    }
  }
  void test_resize_f32_8x8() {
    size_t src_width = 0, src_height = 0, thread_count = 0, channels = 0;
    std::tie(src_width, src_height, thread_count, channels) = GetParam();
    if (channels <= 1) {
      check<float>(kleidicv_resize_linear_f32,
                   kleidicv_thread_resize_linear_f32, thread_count, src_width,
                   src_height, 1, 8 * src_width, 8 * src_height);
    }
  }

 private:
  template <typename ScalarType, typename SingleThreadedFunc,
            typename MultithreadedFunc>
  static void check(SingleThreadedFunc single_threaded_func,
                    MultithreadedFunc multithreaded_func, size_t thread_count,
                    size_t src_width, size_t src_height,
                    [[maybe_unused]] size_t channels, size_t dst_width,
                    size_t dst_height) {
    test::Array2D<ScalarType> src(src_width * channels, src_height),
        dst_single(dst_width * channels, dst_height),
        dst_multi(dst_width * channels, dst_height);
    test::PseudoRandomNumberGenerator<ScalarType> generator;
    src.fill(generator);

    EXPECT_EQ(KLEIDICV_OK, single_threaded_func(
                               src.data(), src.stride(), src_width, src_height,
                               dst_single.data(), dst_single.stride(),
                               dst_width, dst_height, channels));
    EXPECT_EQ(KLEIDICV_OK,
              multithreaded_func(
                  src.data(), src.stride(), src_width, src_height,
                  dst_multi.data(), dst_multi.stride(), dst_width, dst_height,
                  channels, get_multithreading_fake(thread_count)));

    if (dst_single.compare_to(dst_multi)) {
      std::cout << "source:\n";
      dump(&src);
      std::cout << "dst_single:\n";
      dump(&dst_single);
      std::cout << "dst_multi:\n";
      dump(&dst_multi);
    }

    EXPECT_EQ_ARRAY2D(dst_single, dst_multi);
  }
};

TEST_P(ResizeThread, ResizeToQuarter) { test_resize_to_quarter(); }

TEST_P(ResizeThread, ResizeUint2x2) { test_resize_u8_2x2(); }

TEST_P(ResizeThread, ResizeUint4x4) { test_resize_u8_4x4(); }

TEST_P(ResizeThread, ResizeUintDown2) { test_resize_u8_down2(); }

TEST_P(ResizeThread, ResizeUintDown3) { test_resize_u8_down3(); }

TEST_P(ResizeThread, ResizeUintUp105) { test_resize_u8_up_1_05(); }

TEST_P(ResizeThread, ResizeUintUp105Down) {
  test_resize_u8_up_1_05_vertical_down();
}
TEST_P(ResizeThread, ResizeUintUp16) { test_resize_u8_up_1_6(); }

TEST_P(ResizeThread, ResizeUintUp16Down) {
  test_resize_u8_up_1_6_vertical_down();
}

TEST_P(ResizeThread, ResizeFloat2x2) { test_resize_f32_2x2(); }

TEST_P(ResizeThread, ResizeFloat4x4) { test_resize_f32_4x4(); }

TEST_P(ResizeThread, ResizeFloat8x8) { test_resize_f32_8x8(); }

// clang-format off
INSTANTIATE_TEST_SUITE_P(, ResizeThread,
                         testing::Values(P{1, 1, 1, 1}, P{ 1,  2, 1, 2},
                                         P{1, 2, 2, 1}, P{ 2,  1, 2, 2}, P{ 2,  1, 2, 3},
                                         P{2, 2, 1, 1}, P{ 1,  3, 2, 2},
                                         P{2, 3, 1, 1}, P{ 6,  4, 1, 2}, P{ 6,  4, 1, 3},
                                         P{4, 5, 2, 1}, P{ 2,  6, 3, 2}, P{ 2,  6, 3, 3}, P{ 3,  5, 2, 4},
                                         P{1, 7, 4, 1}, P{12, 34, 5, 2}, P{ 7, 17, 5, 3}, P{11, 19, 3, 4}));
// clang-format on

TEST_F(ResizeThreadValidation,
       EmptyInputsAndZeroChannelsReturnBeforeScheduling) {
  uint8_t src = 0;
  uint8_t dst = 42;
  float src_f32 = 0.0F;
  float dst_f32 = 42.0F;
  for (const auto &shape : std::array<std::array<size_t, 3>, 4>{
           {{0, 0, 1}, {0, 4, 1}, {4, 0, 1}, {4, 4, 0}}}) {
    const auto [width, height, channels] = shape;
    SCOPED_TRACE(testing::Message() << "width=" << width << ", height="
                                    << height << ", channels=" << channels);
    expect_rejected(KLEIDICV_ERROR_RANGE, kleidicv_thread_resize_linear_u8(
                                              &src, 1, width, height, &dst, 1,
                                              8, 8, channels, scheduler()));
    expect_rejected(KLEIDICV_ERROR_RANGE,
                    kleidicv_thread_resize_linear_f32(
                        &src_f32, sizeof(src_f32), width, height, &dst_f32,
                        sizeof(dst_f32), 8, 8, channels, scheduler()));
    EXPECT_EQ(42, dst);
    EXPECT_EQ(42.0F, dst_f32);
  }
}

TEST_F(ResizeThreadValidation,
       EmptyDestinationRetainsValidationAndDoesNotSchedule) {
  uint8_t src = 0;
  uint8_t dst = 42;
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_resize_linear_u8(&src, 16, 16, 4, &dst, 16,
                                                   16, 0, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_resize_linear_u8(nullptr, 16, 16, 4, &dst, 16,
                                                   16, 0, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_resize_linear_u8(&src, 16, 16, 4, &dst, 16,
                                                   16, 0, 0, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_resize_linear_u8(&src, 16, 16, 0, &dst, 16,
                                                   16, 0, 1, scheduler()));
  EXPECT_EQ(42, dst);
}

TEST(ResizeThreadTest, NullPointer) {
  uint8_t src[2] = {}, dst[8] = {};

  test::test_null_args(kleidicv_thread_resize_linear_u8, src, 1, 1, 2, dst, 2,
                       2, 4, 1, get_multithreading_fake(2));
}

TEST(ResizeThreadTest, NotImplemented) {
  for (size_t channels = 1; channels <= 4; ++channels) {
    // Too small images
    uint8_t src[1] = {}, dst[1] = {};
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              kleidicv_thread_resize_linear_u8(src, sizeof(src), 2, 2, dst,
                                               sizeof(dst), 3, 7, channels,
                                               get_multithreading_fake(2)));
  }

  {
    // Too small images
    float src[1] = {}, dst[1] = {};
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              kleidicv_thread_resize_linear_f32(src, sizeof(src), 2, 2, dst,
                                                sizeof(dst), 3, 7, 1,
                                                get_multithreading_fake(2)));
  }

  {
    // Invalid channel count
    size_t channels = 2;
    std::vector<uint8_t> src(2UL * 2UL * channels);
    std::vector<uint8_t> dst(4UL * 4UL * channels);
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              kleidicv_thread_resize_linear_u8(
                  src.data(), sizeof(uint8_t) * channels * 2, 2, 2, dst.data(),
                  sizeof(uint8_t) * channels * 4, 4, 4, channels,
                  get_multithreading_fake(2)));
  }

  {
    // Invalid channel count
    size_t channels = 5;
    std::vector<uint8_t> src(48UL * 2UL * channels);
    std::vector<uint8_t> dst(32UL * 1UL * channels);
    EXPECT_EQ(KLEIDICV_ERROR_RANGE,
              kleidicv_thread_resize_linear_u8(
                  src.data(), sizeof(uint8_t) * channels * 48, 48, 2,
                  dst.data(), sizeof(uint8_t) * channels * 32, 32, 1, channels,
                  get_multithreading_fake(2)));
  }

  {
    // Invalid channel count
    size_t channels = 2;
    std::vector<float> src(2UL * 2UL * channels);
    std::vector<float> dst(4UL * 4UL * channels);
    EXPECT_EQ(KLEIDICV_ERROR_NOT_IMPLEMENTED,
              kleidicv_thread_resize_linear_f32(
                  src.data(), sizeof(float) * channels * 2, 2, 2, dst.data(),
                  sizeof(float) * channels * 4, 4, 4, channels,
                  get_multithreading_fake(2)));
  }
}

namespace {

TEST_F(ResizeThreadValidation, ResizeLinearU8) {
  uint8_t src = 0;
  uint8_t dst = 0;

  expect_rejected(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                  kleidicv_thread_resize_linear_u8(nullptr, 1, 1, 1, &dst, 8, 8,
                                                   8, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_resize_linear_u8(nullptr, 16, 16, 4, &dst, 24,
                                                   24, 6, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_resize_linear_u8(&src, 16, 16, 4, nullptr, 24,
                                                   24, 6, 1, scheduler()));
  expect_dispatched(6, kleidicv_thread_resize_linear_u8(
                           &src, 16, 16, 4, &dst, 24, 24, 6, 1, scheduler()));
}

TEST_F(ResizeThreadValidation, ResizeLinearF32) {
  float src = 0.0F;
  float dst = 0.0F;

  expect_rejected(KLEIDICV_ERROR_NOT_IMPLEMENTED,
                  kleidicv_thread_resize_linear_f32(nullptr, 1, 2, 2, &dst, 1,
                                                    3, 3, 1, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_resize_linear_f32(nullptr, sizeof(float), 2, 2, &dst,
                                        sizeof(float), 4, 4, 1, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_NULL_POINTER,
      kleidicv_thread_resize_linear_f32(&src, sizeof(float), 2, 2, nullptr,
                                        sizeof(float), 4, 4, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_ALIGNMENT,
                  kleidicv_thread_resize_linear_f32(&src, 1, 2, 2, nullptr, 1,
                                                    4, 4, 1, scheduler()));
  expect_rejected(KLEIDICV_ERROR_ALIGNMENT, kleidicv_thread_resize_linear_f32(
                                                &src, sizeof(float), 2, 2, &dst,
                                                1, 4, 4, 1, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_RANGE,
      kleidicv_thread_resize_linear_f32(&src, sizeof(float), 0, 2, &dst,
                                        sizeof(float), 4, 4, 1, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_RANGE,
      kleidicv_thread_resize_linear_f32(&src, sizeof(float), 2, 0, &dst,
                                        sizeof(float), 4, 4, 1, scheduler()));
  expect_dispatched(1, kleidicv_thread_resize_linear_f32(&src, sizeof(float), 2,
                                                         2, &dst, sizeof(float),
                                                         4, 4, 1, scheduler()));
}

// Check the output after one controlled stripe, then finish the other tasks
// so the scheduler covers every task exactly once.
struct PartialResizeScheduler {
  size_t begin;
  size_t end;
  size_t expected_task_count;
  std::function<void()> check_stripe;
  size_t call_count = 0;

  static kleidicv_error_t parallel(kleidicv_thread_callback callback,
                                   void *callback_data, void *parallel_data,
                                   size_t task_count) {
    auto &self = *static_cast<PartialResizeScheduler *>(parallel_data);
    ++self.call_count;
    EXPECT_EQ(self.expected_task_count, task_count);
    if (auto err = callback(self.begin, self.end, callback_data)) {
      return err;
    }
    self.check_stripe();
    if (self.begin != 0) {
      if (auto err = callback(0, self.begin, callback_data)) {
        return err;
      }
    }
    if (self.end != task_count) {
      return callback(self.end, task_count, callback_data);
    }
    return KLEIDICV_OK;
  }
};

TEST(ResizeThreadTest, U8PartialStripeMatchesPublicApi) {
  constexpr size_t kSrcWidth = 16;
  constexpr size_t kSrcHeight = 4;
  constexpr size_t kDstWidth = 24;
  constexpr size_t kDstHeight = 6;
  constexpr size_t kYBegin = 2;
  constexpr size_t kYEnd = 4;
  constexpr uint8_t kSentinel = 0xA5;
  std::array<uint8_t, kSrcWidth * kSrcHeight> src = {};
  std::array<uint8_t, kDstWidth * kDstHeight> expected = {};
  std::array<uint8_t, kDstWidth * kDstHeight> actual = {};

  for (size_t index = 0; index < src.size(); ++index) {
    src[index] = static_cast<uint8_t>(index);
  }
  actual.fill(kSentinel);
  ASSERT_EQ(KLEIDICV_OK,
            kleidicv_resize_linear_u8(src.data(), kSrcWidth, kSrcWidth,
                                      kSrcHeight, expected.data(), kDstWidth,
                                      kDstWidth, kDstHeight, 1));

  PartialResizeScheduler scheduler{
      kYBegin, kYEnd, kDstHeight, [&] {
        for (size_t y = 0; y < kDstHeight; ++y) {
          for (size_t x = 0; x < kDstWidth; ++x) {
            const size_t index = y * kDstWidth + x;
            EXPECT_EQ(y >= kYBegin && y < kYEnd ? expected[index] : kSentinel,
                      actual[index]);
          }
        }
      }};
  ASSERT_EQ(KLEIDICV_OK, kleidicv_thread_resize_linear_u8(
                             src.data(), kSrcWidth, kSrcWidth, kSrcHeight,
                             actual.data(), kDstWidth, kDstWidth, kDstHeight, 1,
                             {PartialResizeScheduler::parallel, &scheduler}));
  EXPECT_EQ(size_t{1}, scheduler.call_count);
  EXPECT_EQ(expected, actual);
}

TEST(ResizeThreadTest, F32PartialStripeMatchesPublicApi) {
  constexpr size_t kSrcWidth = 3;
  constexpr size_t kSrcHeight = 3;
  constexpr size_t kDstWidth = 6;
  constexpr size_t kDstHeight = 6;
  constexpr size_t kFirstWrittenDstRow = 3;
  constexpr float kSentinel = -1.0F;
  std::array<float, kSrcWidth * kSrcHeight> src = {0.0F, 1.0F, 2.0F, 3.0F, 4.0F,
                                                   5.0F, 6.0F, 7.0F, 8.0F};
  std::array<float, kDstWidth * kDstHeight> expected = {};
  std::array<float, kDstWidth * kDstHeight> actual = {};
  actual.fill(kSentinel);
  ASSERT_EQ(KLEIDICV_OK,
            kleidicv_resize_linear_f32(src.data(), sizeof(float) * kSrcWidth,
                                       kSrcWidth, kSrcHeight, expected.data(),
                                       sizeof(float) * kDstWidth, kDstWidth,
                                       kDstHeight, 1));

  // Task [1, 2) becomes source stripe [1, 3), which writes destination rows
  // [3, 6). The scheduler then processes the remaining task [0, 1).
  PartialResizeScheduler scheduler{
      1, 2, kSrcHeight - 1, [&] {
        for (size_t y = 0; y < kDstHeight; ++y) {
          for (size_t x = 0; x < kDstWidth; ++x) {
            const size_t index = y * kDstWidth + x;
            EXPECT_FLOAT_EQ(
                y >= kFirstWrittenDstRow ? expected[index] : kSentinel,
                actual[index]);
          }
        }
      }};
  ASSERT_EQ(KLEIDICV_OK,
            kleidicv_thread_resize_linear_f32(
                src.data(), sizeof(float) * kSrcWidth, kSrcWidth, kSrcHeight,
                actual.data(), sizeof(float) * kDstWidth, kDstWidth, kDstHeight,
                1, {PartialResizeScheduler::parallel, &scheduler}));
  EXPECT_EQ(size_t{1}, scheduler.call_count);
  for (size_t index = 0; index < actual.size(); ++index) {
    EXPECT_FLOAT_EQ(expected[index], actual[index]);
  }
}

}  // namespace
