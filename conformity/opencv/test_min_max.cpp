// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <vector>

#include "tests.h"

template <bool GetIndex>
cv::Mat exec_min_max(cv::Mat& input) {
  double minVal, maxVal;
  if constexpr (GetIndex) {
    cv::Mat result(2 + 2 * input.dims, 1, CV_32SC1);
    int32_t* min_index = result.ptr<int32_t>(2, 0);
    int32_t* max_index = result.ptr<int32_t>(2 + input.dims, 0);
    cv::minMaxIdx(input, &minVal, &maxVal, min_index, max_index);
    result.at<int32_t>(0, 0) = minVal;
    result.at<int32_t>(1, 0) = maxVal;
    return result;
  } else {
    cv::minMaxIdx(input, &minVal, &maxVal);
    cv::Mat result(2, 1, CV_32SC1);
    result.at<int32_t>(0, 0) = minVal;
    result.at<int32_t>(1, 0) = maxVal;
    return result;
  }
}

#if MANAGER
template <typename T, size_t Format, bool GetIndex>
bool test_min_max(int index, RecreatedMessageQueue& request_queue,
                  RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  constexpr size_t Channels = (Format >> CV_CN_SHIFT) + 1;

  // Test 2D matrices
  for (size_t x = 5; x <= 16; ++x) {
    for (size_t y = 5; y <= 16; ++y) {
      cv::Mat input(x, y, Format, cv::Scalar::all(100));

      // Add a few random values at random locations.
      for (int i = 0; i < 3; ++i) {
        T* pixel = input.ptr<T>(rng.next() % x, rng.next() % y);
        pixel[rng.next() % Channels] = rng.next();
      }

      cv::Mat actual = exec_min_max<GetIndex>(input);
      cv::Mat expected = get_expected_from_subordinate(index, request_queue,
                                                       reply_queue, input);
      if (are_matrices_different<int32_t>(0, actual, expected)) {
        fail_print_matrices(x, y, input, actual, expected);
        return true;
      }
    }
  }

  // Test 3D matrix
  const int x = 4, y = 3, z = 2;
  cv::Mat input(std::vector<int>{x, y, z}, Format);
  rng.fill(input, cv::RNG::UNIFORM, std::numeric_limits<T>::lowest(),
           std::numeric_limits<T>::max());
  cv::Mat actual = exec_min_max<GetIndex>(input);
  cv::Mat expected =
      get_expected_from_subordinate(index, request_queue, reply_queue, input);
  if (are_matrices_different<int32_t>(0, actual, expected)) {
    std::cout << "[FAIL]\n";
    std::cout << "=== Input Matrix:\n";
    for (int i = 0; i < z; ++i) {
      // Output a slice of the 3D matrix.
      std::cout << cv::Mat(x, y, Format,
                           input.ptr<T>() + input.step[0] * input.step[1] * i)
                << "\n";
    }
    std::cout << "\n=== Manager result (actual):\n" << actual;
    std::cout << "\n\n=== Subordinate result (expected):\n" << expected;
    std::cout << "\n" << std::endl;
    return true;
  }

  return false;
}
#endif

static cv::Mat exec_empty_min_max(cv::Mat& input) {
  const cv::Mat empty(input.rows, input.cols, input.type());
  double minimum = -99, maximum = -99;
  cv::Point min_location(37, 37), max_location(37, 37);
  int error = 0;
  try {
    cv::minMaxLoc(empty, &minimum, &maximum, &min_location, &max_location);
  } catch (const cv::Exception& exception) {
    error = exception.code;
  }
  cv::Mat result(1, 7, CV_64FC1);
  result.at<double>(0, 0) = static_cast<double>(error);
  result.at<double>(0, 1) = minimum;
  result.at<double>(0, 2) = maximum;
  result.at<double>(0, 3) = min_location.x;
  result.at<double>(0, 4) = min_location.y;
  result.at<double>(0, 5) = max_location.x;
  result.at<double>(0, 6) = max_location.y;
  return result;
}

#if MANAGER
static bool test_empty_min_max(int index, RecreatedMessageQueue& request_queue,
                               RecreatedMessageQueue& reply_queue) {
  bool failed = false;
  for (cv::Size size : {cv::Size(0, 0), cv::Size(0, 4), cv::Size(4, 0)}) {
    cv::Mat input(size, CV_8UC1);
    cv::Mat actual = exec_empty_min_max(input);
    cv::Mat expected =
        get_expected_from_subordinate(index, request_queue, reply_queue, input);
    if (are_matrices_different<double>(0, actual, expected)) {
      fail_print_matrices(size.height, size.width, input, actual, expected);
      failed = true;
    }
  }
  return failed;
}
#endif

std::vector<test>& min_max_tests_get() {
  // clang-format off
  static std::vector<test> tests = {
    TEST("min_max empty input CV_8UC1", test_empty_min_max, exec_empty_min_max),

    TEST("min_max_s8, 1 channel", (test_min_max<int8_t, CV_8SC1, false>), (exec_min_max<false>)),
    TEST("min_max_u8, 1 channel", (test_min_max<uint8_t, CV_8UC1, false>), (exec_min_max<false>)),
    TEST("min_max_s16, 1 channel", (test_min_max<int16_t, CV_16SC1, false>), (exec_min_max<false>)),
    TEST("min_max_u16, 1 channel", (test_min_max<uint16_t, CV_16UC1, false>), (exec_min_max<false>)),
    TEST("min_max_s32, 1 channel", (test_min_max<int32_t, CV_32SC1, false>), (exec_min_max<false>)),
    TEST("min_max_f32, 1 channel", (test_min_max<float, CV_32FC1, false>), (exec_min_max<false>)),
    TEST("min_max_loc_u8", (test_min_max<uint8_t, CV_8UC1, true>), (exec_min_max<true>)),
  };
  // clang-format on
  return tests;
}
