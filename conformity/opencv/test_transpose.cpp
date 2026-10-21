// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <vector>

#include "empty_input.h"
#include "opencv2/core.hpp"
#include "tests.h"

static cv::Mat exec_transpose(cv::Mat& input_mat) {
  cv::Mat result;
  cv::transpose(input_mat, result);
  return result;
}

static cv::Mat exec_transpose_inplace(cv::Mat& input_mat) {
  cv::Mat result = input_mat.clone();
  cv::transpose(result, result);
  return result;
}

#if MANAGER
template <typename TypeParam, size_t Channels>
bool test_transpose(int index, RecreatedMessageQueue& request_queue,
                    RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t height = 5; height <= 16; ++height) {
    for (size_t width = 5; width <= 16; ++width) {
      cv::Mat input(height, width,
                    get_opencv_matrix_type<TypeParam, Channels>());
      rng.fill(input, cv::RNG::UNIFORM, std::numeric_limits<TypeParam>::min(),
               std::numeric_limits<TypeParam>::max());

      cv::Mat actual = exec_transpose(input);
      cv::Mat expected = get_expected_from_subordinate(index, request_queue,
                                                       reply_queue, input);

      if (are_matrices_different<TypeParam>(0, actual, expected)) {
        fail_print_matrices(input.rows, input.cols, input, actual, expected);
        return true;
      }
    }
  }

  return false;
}

template <typename TypeParam, size_t Channels>
bool test_transpose_inplace(int index, RecreatedMessageQueue& request_queue,
                            RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t size = 5; size <= 16; ++size) {
    cv::Mat input(size, size, get_opencv_matrix_type<TypeParam, Channels>());
    rng.fill(input, cv::RNG::UNIFORM, std::numeric_limits<TypeParam>::min(),
             std::numeric_limits<TypeParam>::max());

    cv::Mat actual = exec_transpose_inplace(input);
    cv::Mat expected =
        get_expected_from_subordinate(index, request_queue, reply_queue, input);

    if (are_matrices_different<TypeParam>(0, actual, expected)) {
      fail_print_matrices(input.rows, input.cols, input, actual, expected);
      return true;
    }
  }

  return false;
}
#endif

#define TRANSPOSE_TEST(type, channels, label)          \
  TEST("Transpose, " #channels " channel (" label ")", \
       (test_transpose<type, channels>), (exec_transpose))

#define TRANSPOSE_INPLACE_TEST(type, channels, label)          \
  TEST("Transpose Inplace, " #channels " channel (" label ")", \
       (test_transpose_inplace<type, channels>), (exec_transpose_inplace))

static void empty_transpose(const cv::Mat& input, cv::Mat& output) {
  cv::transpose(input, output);
}

std::vector<test>& transpose_tests_get() {
  static std::vector<test> tests = {
      EMPTY_INPUT_TEST("transpose", CV_8UC1, exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_8UC3, exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_8UC4, exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_8SC1, exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_16UC1,
                       exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_16SC1,
                       exec_empty_input<empty_transpose>),
      EMPTY_INPUT_TEST("transpose", CV_32FC1,
                       exec_empty_input<empty_transpose>),

      TRANSPOSE_TEST(uint8_t, 1, "U8"),
      TRANSPOSE_TEST(uint8_t, 2, "U8"),
      TRANSPOSE_TEST(uint8_t, 3, "U8"),
      TRANSPOSE_TEST(uint8_t, 4, "U8"),
      TRANSPOSE_TEST(uint16_t, 1, "U16"),
      TRANSPOSE_TEST(uint16_t, 2, "U16"),
      TRANSPOSE_TEST(uint16_t, 3, "U16"),
      TRANSPOSE_TEST(uint16_t, 4, "U16"),
      TRANSPOSE_INPLACE_TEST(uint8_t, 1, "U8"),
      TRANSPOSE_INPLACE_TEST(uint8_t, 2, "U8"),
      TRANSPOSE_INPLACE_TEST(uint8_t, 3, "U8"),
      TRANSPOSE_INPLACE_TEST(uint8_t, 4, "U8"),
      TRANSPOSE_INPLACE_TEST(uint16_t, 1, "U16"),
      TRANSPOSE_INPLACE_TEST(uint16_t, 2, "U16"),
      TRANSPOSE_INPLACE_TEST(uint16_t, 3, "U16"),
      TRANSPOSE_INPLACE_TEST(uint16_t, 4, "U16"),
  };
  return tests;
}
