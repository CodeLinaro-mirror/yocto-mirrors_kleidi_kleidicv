// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <thread>

#include "framework/array.h"
#include "framework/generator.h"
#include "framework/utils.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv_thread/kleidicv_thread.h"
#include "multithreading_fake.h"
#include "test_config.h"
#include "thread_validation.h"

#define KLEIDICV_MIN_MAX(type, suffix) \
  KLEIDICV_API(min_max, kleidicv_min_max_##suffix, type)

KLEIDICV_MIN_MAX(int8_t, s8);
KLEIDICV_MIN_MAX(uint8_t, u8);
KLEIDICV_MIN_MAX(int16_t, s16);
KLEIDICV_MIN_MAX(uint16_t, u16);
KLEIDICV_MIN_MAX(int32_t, s32);
KLEIDICV_MIN_MAX(float, f32);

#define KLEIDICV_THREAD_MIN_MAX(type, suffix) \
  KLEIDICV_API(thread_min_max, kleidicv_thread_min_max_##suffix, type)

KLEIDICV_THREAD_MIN_MAX(int8_t, s8);
KLEIDICV_THREAD_MIN_MAX(uint8_t, u8);
KLEIDICV_THREAD_MIN_MAX(int16_t, s16);
KLEIDICV_THREAD_MIN_MAX(uint16_t, u16);
KLEIDICV_THREAD_MIN_MAX(int32_t, s32);
KLEIDICV_THREAD_MIN_MAX(float, f32);

TEST(MinMaxThread, SimpleFloat) {
  const size_t width = 2 * test::Options::vector_lanes<float>() + 1;
  const size_t height = width;
  test::Array2D<float> src{width, height, 7};
  ASSERT_TRUE(src.valid());

  const float infinity = std::numeric_limits<float>::infinity();
  struct Elements {
    float first, second, expected_min, expected_max;
  };
  const Elements elements[] = {
      // Uniform finite inputs: both results equal the input value.
      {1.0F, 1.0F, 1.0F, 1.0F},
      {-1.0F, -1.0F, -1.0F, -1.0F},
      // Opposite zero signs: minimum selects -0 and maximum selects +0.
      {0.0F, -0.0F, -0.0F, 0.0F},
      // Reversing the zero order must preserve the selected result signs.
      {-0.0F, 0.0F, -0.0F, 0.0F},
      // Uniform positive zeros: both results retain the positive sign.
      {0.0F, 0.0F, 0.0F, 0.0F},
      // Uniform negative zeros: both results retain the negative sign.
      {-0.0F, -0.0F, -0.0F, -0.0F},
      // Uniform +infinity: unused batch slots must not lower the minimum.
      {infinity, infinity, infinity, infinity},
      // Uniform -infinity: unused batch slots must not raise the maximum.
      {-infinity, -infinity, -infinity, -infinity}};
  for (const auto& data : elements) {
    src.fill([&](size_t y, size_t x) {
      return (y * width + x) % 2 ? data.second : data.first;
    });
    for (size_t thread_count : {1, 3}) {
      SCOPED_TRACE(testing::Message()
                   << "first=" << data.first << ", second=" << data.second
                   << ", thread_count=" << thread_count);
      float minval = 10, maxval = 20;
      EXPECT_EQ(KLEIDICV_OK,
                thread_min_max<float>()(src.data(), src.stride(), width, height,
                                        &minval, &maxval,
                                        get_multithreading_fake(thread_count)));
      // Compare representations to distinguish positive and negative zero.
      EXPECT_EQ(test::float_to_bits(data.expected_min),
                test::float_to_bits(minval));
      EXPECT_EQ(test::float_to_bits(data.expected_max),
                test::float_to_bits(maxval));
    }
  }
}

TEST(MinMaxThread, OversizeImage) {
  float src = 0;
  float value = 0;
  const size_t excessive_height = std::numeric_limits<size_t>::max();
  const auto mt = get_multithreading_fake(2);

  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            kleidicv_thread_min_max_f32(&src, sizeof(src), 1, excessive_height,
                                        &value, nullptr, mt));
  EXPECT_EQ(KLEIDICV_ERROR_RANGE,
            kleidicv_thread_min_max_f32(&src, sizeof(src), 1, excessive_height,
                                        nullptr, &value, mt));
}

namespace {

using ThreadMinMaxValidation = test::ThreadValidation;

TEST_F(ThreadMinMaxValidation, NullSourcesReturnBeforeScheduling) {
  int8_t min_s8 = 1, max_s8 = 2;
  uint8_t min_u8 = 1, max_u8 = 2;
  int16_t min_s16 = 1, max_s16 = 2;
  uint16_t min_u16 = 1, max_u16 = 2;
  int32_t min_s32 = 1, max_s32 = 2;
  float min_f32 = 1.0F, max_f32 = 2.0F;

#define EXPECT_NULL(call) expect_rejected(KLEIDICV_ERROR_NULL_POINTER, call)

  EXPECT_NULL(kleidicv_thread_min_max_s8(nullptr, sizeof(int8_t), 1, 2, &min_s8,
                                         &max_s8, scheduler()));
  EXPECT_NULL(kleidicv_thread_min_max_u8(nullptr, sizeof(uint8_t), 1, 2,
                                         &min_u8, &max_u8, scheduler()));
  EXPECT_NULL(kleidicv_thread_min_max_s16(nullptr, sizeof(int16_t), 1, 2,
                                          &min_s16, &max_s16, scheduler()));
  EXPECT_NULL(kleidicv_thread_min_max_u16(nullptr, sizeof(uint16_t), 1, 2,
                                          &min_u16, &max_u16, scheduler()));
  EXPECT_NULL(kleidicv_thread_min_max_s32(nullptr, sizeof(int32_t), 1, 2,
                                          &min_s32, &max_s32, scheduler()));
  EXPECT_NULL(kleidicv_thread_min_max_f32(nullptr, sizeof(float), 1, 2,
                                          &min_f32, &max_f32, scheduler()));

#undef EXPECT_NULL

  EXPECT_EQ(int8_t{1}, min_s8);
  EXPECT_EQ(int8_t{2}, max_s8);
}

TEST_F(ThreadMinMaxValidation, AlignmentPrecedesImageRange) {
  int16_t src = 0;
  constexpr size_t kLargeWidth = static_cast<size_t>(KLEIDICV_MAX_IMAGE_PIXELS);

  expect_rejected(KLEIDICV_ERROR_NULL_POINTER,
                  kleidicv_thread_min_max_s16(nullptr, 1, kLargeWidth, 2,
                                              nullptr, nullptr, scheduler()));
  expect_rejected(KLEIDICV_ERROR_ALIGNMENT,
                  kleidicv_thread_min_max_s16(&src, 1, kLargeWidth, 2, nullptr,
                                              nullptr, scheduler()));

  // As in the ordinary API, a single-row image does not require an aligned
  // stride.
  expect_dispatched(1, kleidicv_thread_min_max_s16(&src, 1, 1, 1, nullptr,
                                                   nullptr, scheduler()));
}

TEST_F(ThreadMinMaxValidation, InvalidGeometryReturnsBeforeScheduling) {
  uint8_t src = 0;
  constexpr size_t kLargeWidth = static_cast<size_t>(KLEIDICV_MAX_IMAGE_PIXELS);
  constexpr size_t kMaxSize = std::numeric_limits<size_t>::max();

  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_min_max_u8(&src, 1, kLargeWidth, 2, nullptr,
                                             nullptr, scheduler()));
  expect_rejected(KLEIDICV_ERROR_RANGE,
                  kleidicv_thread_min_max_u8(&src, 1, kMaxSize, 2, nullptr,
                                             nullptr, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_RANGE,
      kleidicv_thread_min_max_u8(&src, 1, 0, 2, nullptr, nullptr, scheduler()));
  expect_rejected(
      KLEIDICV_ERROR_RANGE,
      kleidicv_thread_min_max_u8(&src, 1, 1, 0, nullptr, nullptr, scheduler()));
}

TEST_F(ThreadMinMaxValidation, OptionalOutputsStillDispatch) {
  uint8_t src[2] = {};

  expect_dispatched(2, kleidicv_thread_min_max_u8(src, 1, 1, 2, nullptr,
                                                  nullptr, scheduler()));
}

#ifdef KLEIDICV_ALLOCATION_TESTS
TEST_F(ThreadMinMaxValidation, MinMaxAllocationFailureReturnsBeforeScheduling) {
  const auto check = [this](const char* name, auto min_max, auto src) {
    SCOPED_TRACE(name);
    for (bool request_min : {true, false}) {
      SCOPED_TRACE(request_min ? "minimum" : "maximum");
      decltype(src) min_value = 1;
      decltype(src) max_value = 2;
      const auto mt = scheduler();

      AllocationFailureMock::enable();
      const kleidicv_error_t result =
          min_max(&src, sizeof(src), 1, 1, request_min ? &min_value : nullptr,
                  request_min ? nullptr : &max_value, mt);
      AllocationFailureMock::disable();

      expect_rejected(KLEIDICV_ERROR_ALLOCATION, result);
      EXPECT_EQ(1, min_value);
      EXPECT_EQ(2, max_value);
    }
  };

  check("s8", kleidicv_thread_min_max_s8, int8_t{0});
  check("u8", kleidicv_thread_min_max_u8, uint8_t{0});
  check("s16", kleidicv_thread_min_max_s16, int16_t{0});
  check("u16", kleidicv_thread_min_max_u16, uint16_t{0});
  check("s32", kleidicv_thread_min_max_s32, int32_t{0});
  check("f32", kleidicv_thread_min_max_f32, float{0});
}
#endif  // KLEIDICV_ALLOCATION_TESTS

}  // namespace

template <typename ElementType>
class MinMaxThread : public testing::Test {};

TYPED_TEST_SUITE_P(MinMaxThread);

// Tuple of width, height, thread count.
typedef std::tuple<size_t, size_t, size_t> P;

static const auto test_params = {
    P{1, 1, 1}, P{1, 2, 1}, P{1, 2, 2}, P{2, 1, 2}, P{2, 2, 1}, P{1, 3, 2},
    P{2, 3, 1}, P{6, 4, 1}, P{4, 5, 2}, P{2, 6, 3}, P{1, 7, 4}, P{12, 34, 5}};

TYPED_TEST_P(MinMaxThread, CompareWithSingle) {
  test::PseudoRandomNumberGenerator<TypeParam> generator;
  size_t width = 0, height = 0, thread_count = 0;
  for (auto params : test_params) {
    std::tie(width, height, thread_count) = params;
    test::Array2D<TypeParam> src(width, height);
    TypeParam min_single, max_single, min_multi, max_multi;

    src.fill(generator);

    kleidicv_error_t single_result = min_max<TypeParam>()(
        src.data(), src.stride(), width, height, &min_single, &max_single);

    kleidicv_error_t multi_result = thread_min_max<TypeParam>()(
        src.data(), src.stride(), width, height, &min_multi, &max_multi,
        get_multithreading_fake(thread_count));

    EXPECT_EQ(KLEIDICV_OK, single_result);
    EXPECT_EQ(KLEIDICV_OK, multi_result);
    EXPECT_EQ(min_multi, min_single);
    EXPECT_EQ(max_multi, max_single);
  }
}

TYPED_TEST_P(MinMaxThread, NullArguments) {
  size_t width = 1, height = 2, thread_count = 2;
  TypeParam src[2] = {1, 2}, min_value, max_value;

  min_value = max_value = 0;
  kleidicv_error_t res = thread_min_max<TypeParam>()(
      src, width * sizeof(TypeParam), width, height, nullptr, &max_value,
      get_multithreading_fake(thread_count));

  EXPECT_EQ(KLEIDICV_OK, res);
  EXPECT_EQ(0, min_value);
  EXPECT_EQ(2, max_value);

  min_value = max_value = 0;
  res = thread_min_max<TypeParam>()(src, width * sizeof(TypeParam), width,
                                    height, &min_value, nullptr,
                                    get_multithreading_fake(thread_count));

  EXPECT_EQ(KLEIDICV_OK, res);
  EXPECT_EQ(1, min_value);
  EXPECT_EQ(0, max_value);

  min_value = max_value = 0;
  res = thread_min_max<TypeParam>()(src, width * sizeof(TypeParam), width,
                                    height, nullptr, nullptr,
                                    get_multithreading_fake(thread_count));

  EXPECT_EQ(KLEIDICV_OK, res);
  EXPECT_EQ(0, min_value);
  EXPECT_EQ(0, max_value);
}

TYPED_TEST_P(MinMaxThread, MergeFailure) {
  // Plain function pointers require static state for the failure callback.
  static auto original = min_max<TypeParam>();
  static bool fail_minimum;
  static size_t merge_calls;
  struct RestoreBackend {
    ~RestoreBackend() { min_max<TypeParam>() = original; }
  } restore;
  original = min_max<TypeParam>();

  // CPU and SME batches both use the CPU backend for merging partial results.
  min_max<TypeParam>() = [](const TypeParam* src, size_t stride, size_t width,
                            size_t height, TypeParam* minimum,
                            TypeParam* maximum) {
    // The input batch is 3x2; each merge reduces a single row of two values.
    if (width == 2 && height == 1) {
      ++merge_calls;
      if ((minimum != nullptr) == fail_minimum) {
        return KLEIDICV_ERROR_CONTEXT_MISMATCH;
      }
    }
    return original(src, stride, width, height, minimum, maximum);
  };

  TypeParam src[] = {3, 1, 4, 2, 6, 5};
  for (bool fail_min : {true, false}) {
    SCOPED_TRACE(fail_min ? "minimum merge" : "maximum merge");
    fail_minimum = fail_min;
    merge_calls = 0;
    TypeParam minimum = 10, maximum = 20;
    EXPECT_EQ(
        KLEIDICV_ERROR_CONTEXT_MISMATCH,
        thread_min_max<TypeParam>()(src, 3 * sizeof(TypeParam), 3, 2, &minimum,
                                    &maximum, get_multithreading_fake(1)));
    EXPECT_EQ(fail_min ? 1U : 2U, merge_calls);
    EXPECT_EQ(fail_min ? 10 : 1, minimum);
    EXPECT_EQ(20, maximum);
  }
}

REGISTER_TYPED_TEST_SUITE_P(MinMaxThread, CompareWithSingle, NullArguments,
                            MergeFailure);

using MinMaxElementTypes =
    ::testing::Types<int8_t, uint8_t, int16_t, uint16_t, int32_t, float>;

INSTANTIATE_TYPED_TEST_SUITE_P(MinMax, MinMaxThread, MinMaxElementTypes);
