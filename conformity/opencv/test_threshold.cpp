// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "empty_input.h"
#include "tests.h"

static cv::Mat exec_empty_threshold(cv::Mat& input) {
  const cv::Mat empty(input.rows, input.cols, input.type());
  cv::Mat output(2, 3, CV_8UC1, cv::Scalar(37));
  const cv::Mat initial_output = output;
  double threshold = -99;
  int error = 0;
  try {
    threshold = cv::threshold(empty, output, 128, 255, cv::THRESH_BINARY);
  } catch (const cv::Exception& exception) {
    error = exception.code;
  }
  cv::Mat result(1, 7, CV_64FC1);
  empty_output_result(output, initial_output, error)
      .copyTo(result.colRange(0, 6));
  result.at<double>(0, 6) = threshold;
  return result;
}

std::vector<test>& threshold_tests_get() {
  static std::vector<test> tests = {
      EMPTY_INPUT_TEST("threshold", CV_8UC1, exec_empty_threshold),
      EMPTY_INPUT_TEST("threshold", CV_8UC3, exec_empty_threshold),
      EMPTY_INPUT_TEST("threshold", CV_8UC4, exec_empty_threshold),
  };
  return tests;
}
