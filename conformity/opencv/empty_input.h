// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_OPENCV_CONFORMITY_EMPTY_INPUT_H_
#define KLEIDICV_OPENCV_CONFORMITY_EMPTY_INPUT_H_

#include "utils.h"

// Keep the reply nonempty even if the operation throws or releases its output.
// Result columns: error code (0 on success), output rows, output columns,
// output type, output empty flag, and output byte checksum.
inline cv::Mat empty_output_result(const cv::Mat& output,
                                   const cv::Mat& initial_output, int error) {
  double checksum = 0;
  // Checksum the current output on success or while it still references the
  // initial sentinel buffer. Newly allocated contents may be unspecified after
  // an exception.
  if (error == 0 || output.data == initial_output.data) {
    for (int row = 0; row < output.rows; ++row) {
      const auto* bytes = output.ptr<uint8_t>(row);
      for (size_t column = 0; column < output.cols * output.elemSize();
           ++column) {
        checksum += bytes[column];
      }
    }
  }
  cv::Mat result(1, 6, CV_64FC1);
  result.at<double>(0, 0) = static_cast<double>(error);
  result.at<double>(0, 1) = output.rows;
  result.at<double>(0, 2) = output.cols;
  result.at<double>(0, 3) = output.type();
  result.at<double>(0, 4) = output.empty();
  result.at<double>(0, 5) = checksum;
  return result;
}

template <void (*Operation)(const cv::Mat&, cv::Mat&)>
static cv::Mat exec_empty_input(cv::Mat& input) {
  // Shared-memory transport supplies a data address even for an empty image.
  // Reconstruct it locally so the operation receives a null data pointer.
  const cv::Mat empty(input.rows, input.cols, input.type());
  cv::Mat output(2, 3, CV_8UC1, cv::Scalar(37));
  const cv::Mat initial_output = output;
  int error = 0;
  try {
    Operation(empty, output);
  } catch (const cv::Exception& exception) {
    error = exception.code;
  }
  return empty_output_result(output, initial_output, error);
}

#if MANAGER
template <int Type, cv::Mat (*Exec)(cv::Mat&)>
static bool test_empty_input(int index, RecreatedMessageQueue& request_queue,
                             RecreatedMessageQueue& reply_queue) {
  bool failed = false;
  for (cv::Size size : {cv::Size(0, 0), cv::Size(0, 4), cv::Size(4, 0)}) {
    cv::Mat input(size, Type);
    cv::Mat actual = Exec(input);
    cv::Mat expected =
        get_expected_from_subordinate(index, request_queue, reply_queue, input);
    if (are_matrices_different<double>(0, actual, expected)) {
      std::cout
          << "Empty input mismatch: type=" << Type << '\n'
          << "Actual = OpenCV with KleidiCV; expected = reference OpenCV.\n";
      fail_print_matrices(size.height, size.width, input, actual, expected);
      failed = true;
    }
  }
  return failed;
}
#endif

#define EMPTY_INPUT_TEST(name, type, ...)                                 \
  TEST(name " empty input " #type, (test_empty_input<type, __VA_ARGS__>), \
       (__VA_ARGS__))

#endif  // KLEIDICV_OPENCV_CONFORMITY_EMPTY_INPUT_H_
