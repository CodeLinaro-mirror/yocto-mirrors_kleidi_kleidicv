// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <type_traits>
#include <vector>

#include "empty_input.h"
#include "opencv2/core/hal/interface.h"
#include "tests.h"

static cv::Mat add(cv::Mat& a, cv::Mat& b) { return a + b; }

static cv::Mat sub(cv::Mat& a, cv::Mat& b) { return a - b; }

static cv::Mat mul(cv::Mat& a, cv::Mat& b) { return a.mul(b); }

static cv::Mat absdiff(cv::Mat& a, cv::Mat& b) {
  cv::Mat dst;
  cv::absdiff(a, b, dst);
  return dst;
}

static cv::Mat bitwise_and(cv::Mat& a, cv::Mat& b) {
  cv::Mat dst;
  cv::bitwise_and(a, b, dst);
  return dst;
}

static cv::Mat compare_equal(cv::Mat& a, cv::Mat& b) {
  cv::Mat dst;
  cv::compare(a, b, dst, cv::CMP_EQ);
  return dst;
}

static cv::Mat compare_greater(cv::Mat& a, cv::Mat& b) {
  cv::Mat dst;
  cv::compare(a, b, dst, cv::CMP_GT);
  return dst;
}

template <cv::Mat (*Operation)(cv::Mat&, cv::Mat&)>
static cv::Mat exec_binary_op(cv::Mat& input_mat) {
  int mid = input_mat.rows / 2;
  cv::Mat a = input_mat.rowRange(0, mid);
  cv::Mat b = input_mat.rowRange(mid, input_mat.rows);
  return Operation(a, b);
}

#if MANAGER
template <typename T, cv::Mat (*Operation)(cv::Mat&, cv::Mat&), size_t Format>
static bool test_binary_op(int index, RecreatedMessageQueue& request_queue,
                           RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t height = 5; height <= 16; ++height) {
    for (size_t width = 5; width <= 16; ++width) {
      cv::Mat input(height * 2, width, Format);
      if constexpr (Operation == mul) {
        // Limiting input values, otherewise results overflow all the time
        rng.fill(input, cv::RNG::UNIFORM, std::numeric_limits<T>::lowest() / 16,
                 std::numeric_limits<T>::max() / 16);
      } else {
        rng.fill(input, cv::RNG::UNIFORM, std::numeric_limits<T>::lowest(),
                 std::numeric_limits<T>::max());
      }

      cv::Mat actual = exec_binary_op<Operation>(input);
      cv::Mat expected = get_expected_from_subordinate(index, request_queue,
                                                       reply_queue, input);

      if (are_matrices_different<T>(0, actual, expected)) {
        fail_print_matrices(height, width, input, actual, expected);
        return true;
      }
    }
  }

  return false;
}
#endif

#define BINARY_OP_TEST(op, format, t) \
  TEST(#op " " #format, (test_binary_op<t, op, format>), exec_binary_op<op>)

static void empty_add(const cv::Mat& input, cv::Mat& output) {
  cv::add(input, input, output);
}

static void empty_add_one_empty(const cv::Mat& input, cv::Mat& output) {
  cv::add(input, cv::Mat(4, 4, input.type(), cv::Scalar::all(1)), output);
}

static void empty_subtract(const cv::Mat& input, cv::Mat& output) {
  cv::subtract(input, input, output);
}

static void empty_multiply(const cv::Mat& input, cv::Mat& output) {
  cv::multiply(input, input, output);
}

static void empty_absdiff(const cv::Mat& input, cv::Mat& output) {
  cv::absdiff(input, input, output);
}

static void empty_bitwise_and(const cv::Mat& input, cv::Mat& output) {
  cv::bitwise_and(input, input, output);
}

static void empty_compare(const cv::Mat& input, cv::Mat& output) {
  cv::compare(input, input, output, cv::CMP_EQ);
}

std::vector<test>& binary_op_tests_get() {
  static std::vector<test> tests = {
      EMPTY_INPUT_TEST("add", CV_8UC1, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add", CV_8UC3, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add", CV_8UC4, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add", CV_8SC1, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add", CV_16UC1, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add", CV_16SC1, exec_empty_input<empty_add>),
      EMPTY_INPUT_TEST("add_one_empty", CV_8UC1,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("add_one_empty", CV_8UC3,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("add_one_empty", CV_8UC4,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("add_one_empty", CV_8SC1,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("add_one_empty", CV_16UC1,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("add_one_empty", CV_16SC1,
                       exec_empty_input<empty_add_one_empty>),
      EMPTY_INPUT_TEST("subtract", CV_8UC1, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("subtract", CV_8UC3, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("subtract", CV_8UC4, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("subtract", CV_8SC1, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("subtract", CV_16UC1, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("subtract", CV_16SC1, exec_empty_input<empty_subtract>),
      EMPTY_INPUT_TEST("multiply", CV_8UC1, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("multiply", CV_8UC3, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("multiply", CV_8UC4, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("multiply", CV_8SC1, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("multiply", CV_16UC1, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("multiply", CV_16SC1, exec_empty_input<empty_multiply>),
      EMPTY_INPUT_TEST("absdiff", CV_8UC1, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("absdiff", CV_8UC3, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("absdiff", CV_8UC4, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("absdiff", CV_8SC1, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("absdiff", CV_16UC1, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("absdiff", CV_16SC1, exec_empty_input<empty_absdiff>),
      EMPTY_INPUT_TEST("bitwise_and", CV_8UC1,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_8UC3,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_8UC4,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_8SC1,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_16UC1,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_16SC1,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("bitwise_and", CV_32FC1,
                       exec_empty_input<empty_bitwise_and>),
      EMPTY_INPUT_TEST("compare", CV_8UC1, exec_empty_input<empty_compare>),
      EMPTY_INPUT_TEST("compare", CV_8UC3, exec_empty_input<empty_compare>),
      EMPTY_INPUT_TEST("compare", CV_8UC4, exec_empty_input<empty_compare>),

      BINARY_OP_TEST(add, CV_8SC1, int8_t),
      BINARY_OP_TEST(add, CV_32SC1, int32_t),
      BINARY_OP_TEST(sub, CV_8UC2, uint8_t),
      BINARY_OP_TEST(sub, CV_32SC1, int32_t),
      BINARY_OP_TEST(mul, CV_8SC2, int8_t),
      BINARY_OP_TEST(mul, CV_16UC3, uint16_t),
      BINARY_OP_TEST(mul, CV_32SC1, int32_t),
      BINARY_OP_TEST(absdiff, CV_16SC4, int16_t),
      BINARY_OP_TEST(absdiff, CV_32SC1, int32_t),
      BINARY_OP_TEST(bitwise_and, CV_32SC2, int32_t),
      BINARY_OP_TEST(compare_equal, CV_8UC1, uint8_t),
      BINARY_OP_TEST(compare_greater, CV_8UC1, uint8_t),
  };
  return tests;
}
