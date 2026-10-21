// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <limits>
#include <type_traits>
#include <vector>

#include "empty_input.h"
#include "opencv2/core/hal/interface.h"
#include "opencv2/imgproc/hal/interface.h"
#include "utils.h"

// Factor is interpreted as 1/1000, i.e. 500 for 0.5
template <int Factor, int Type>
cv::Mat exec_resize(cv::Mat& input_mat) {
  cv::Mat result(input_mat.size().height * Factor / 1000,
                 input_mat.size().width * Factor / 1000, input_mat.type());
  resize(input_mat, result, result.size(), 0, 0, Type);
  return result;
}

template <int Factor, int Type>
cv::Mat exec_resize_with_scale(cv::Mat& input_mat) {
  cv::Mat result;
  resize(input_mat, result, cv::Size(), Factor / 1000.0, Factor / 1000.0, Type);
  return result;
}

cv::Mat exec_resize_scale_tolerance_boundary(cv::Mat&) {
  constexpr int kHeight = 1048575;
  constexpr int kWidth = 17;
  cv::Mat input_mat(kHeight, kWidth, CV_8UC1, cv::Scalar(0));
  input_mat.row(kHeight / 2).setTo(255);

  cv::Mat result;
  // The old HAL check rejected only when inv_scale_y != 0 and
  // std::abs(inv_scale_y * src_height - dst_height) > FLT_EPSILON.
  // Here the difference is about 1e-7, so it accepted fy. KleidiCV then
  // used 1/kHeight and sampled the white row exactly, while vanilla OpenCV
  // used fy and blended it with a black row.
  resize(input_mat, result, cv::Size(), 1.0, (1.0 + 1e-7) / kHeight,
         CV_HAL_INTER_LINEAR);
  return result;
}

template <int Type>
cv::Mat exec_resize_random_scale(cv::Mat& input_mat) {
  double scale_x, scale_y;
  memcpy(&scale_x, &input_mat.at<uint8_t>(input_mat.rows - 2, 0),
         sizeof(scale_x));
  memcpy(&scale_y, &input_mat.at<uint8_t>(input_mat.rows - 1, 0),
         sizeof(scale_y));

  cv::Mat input_mat_real = input_mat.rowRange(0, input_mat.rows - 2);
  cv::Mat result;
  resize(input_mat_real, result,
         cv::Size(input_mat.cols * scale_x, input_mat.rows * scale_y), 0, 0,
         Type);
  return result;
}

template <int Type>
cv::Mat exec_resize_to_third(cv::Mat& input_mat) {
  cv::Mat result;
  resize(input_mat, result, cv::Size(input_mat.cols / 3, input_mat.rows / 3), 0,
         0, Type);
  return result;
}

template <size_t DstWidth, size_t DstHeight, int Type>
cv::Mat exec_resize_to_size(cv::Mat& input_mat) {
  cv::Mat result;
  resize(input_mat, result,
         cv::Size(static_cast<int>(DstWidth), static_cast<int>(DstHeight)), 0,
         0, Type);
  return result;
}

#if MANAGER
template <typename T>
static T get_threshold(int, size_t);

template <>
float get_threshold(int, size_t) {
  return 0.01F;
}

template <>
uint8_t get_threshold(int Factor, size_t channels) {
  if (Factor == 500 ||
      ((Factor == 2000 || Factor == 4000) && (channels == 1))) {
    // These tested scale factors differ by at most one intensity level.
    return 1;
  }

  if (channels == 2 || channels == 3) {
    // Carotene does not handle these channel counts; the tested OpenCV core
    // and KleidiCV results differ by at most one intensity level.
    return 1;
  }

  // Empirical tolerance for the registered OpenCV tests. Carotene's Q7
  // interpolation for 1- and 4-channel images can differ from KleidiCV by four
  // levels.
  return 4;
}

template <size_t SrcWidth, size_t SrcHeight, size_t DstWidth, size_t DstHeight,
          int Type, size_t Format>
bool test_resize_checkerboard_compatibility(
    int index, RecreatedMessageQueue& request_queue,
    RecreatedMessageQueue& reply_queue) {
  cv::Mat input_mat(static_cast<int>(SrcHeight), static_cast<int>(SrcWidth),
                    Format);
  const size_t channels = CV_MAT_CN(Format);

  // Checkerboard values exercise deterministic high-delta cases without
  // relying on random data.
  for (size_t y = 0; y < SrcHeight; ++y) {
    uint8_t* src = input_mat.ptr<uint8_t>(static_cast<int>(y));
    for (size_t x = 0; x < SrcWidth; ++x) {
      for (size_t channel = 0; channel < channels; ++channel) {
        src[x * channels + channel] = ((x + y + channel) & 1U) ? 255 : 0;
      }
    }
  }

  cv::Mat actual_mat =
      exec_resize_to_size<DstWidth, DstHeight, Type>(input_mat);
  cv::Mat expected_mat = get_expected_from_subordinate(index, request_queue,
                                                       reply_queue, input_mat);
  return are_matrices_different<uint8_t>(get_threshold<uint8_t>(0, channels),
                                         actual_mat, expected_mat);
}

bool test_resize_odd_source_rounded_scale_fallback(
    int index, RecreatedMessageQueue& request_queue,
    RecreatedMessageQueue& reply_queue) {
  cv::Mat input_mat(91, 71, CV_8UC1);
  cv::RNG rng(0);
  rng.fill(input_mat, cv::RNG::UNIFORM, 0, 255);

  // Half-scale rounding produces a 36x46 image from an odd-sized source, so
  // neither destination-to-source dimension ratio is 0.5.
  cv::Mat actual_mat =
      exec_resize_with_scale<500, CV_HAL_INTER_LINEAR>(input_mat);
  cv::Mat expected_mat = get_expected_from_subordinate(index, request_queue,
                                                       reply_queue, input_mat);
  return actual_mat.size() != cv::Size(36, 46) ||
         are_matrices_different<uint8_t>(0, actual_mat, expected_mat);
}

bool test_resize_scale_tolerance_boundary(int index,
                                          RecreatedMessageQueue& request_queue,
                                          RecreatedMessageQueue& reply_queue) {
  // The image is larger than the shared-memory buffer. Both processes build it
  // locally; only this control matrix and the 17-byte result cross the queue.
  cv::Mat control(1, 1, CV_8UC1, cv::Scalar(0));
  cv::Mat actual_mat = exec_resize_scale_tolerance_boundary(control);
  cv::Mat expected_mat =
      get_expected_from_subordinate(index, request_queue, reply_queue, control);
  return actual_mat.size() != cv::Size(17, 1) ||
         are_matrices_different<uint8_t>(0, actual_mat, expected_mat);
}

template <int Factor, int Type, int MinSize, int MaxSize, size_t Format>
bool test_resize(int index, RecreatedMessageQueue& request_queue,
                 RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t h = 3; h <= 9; h += 2) {
    for (size_t w = MinSize; w <= MaxSize; w += 3) {
      cv::Mat input_mat;
      if constexpr (Factor < 1000) {
        input_mat.create(h * 1000 / Factor, w * 1000 / Factor, Format);
      } else {
        input_mat.create(h, w, Format);
      }
      if constexpr (CV_MAT_DEPTH(Format) == CV_32F) {
        rng.fill(input_mat, cv::RNG::UNIFORM, 1, 1000000);
      } else if constexpr (CV_MAT_DEPTH(Format) == CV_8U) {
        rng.fill(input_mat, cv::RNG::UNIFORM, 0, 255);
      } else {
        return true;
      }
      cv::Mat actual_mat = exec_resize<Factor, Type>(input_mat);
      cv::Mat expected_mat = get_expected_from_subordinate(
          index, request_queue, reply_queue, input_mat);

      bool success = (CV_MAT_DEPTH(Format) == CV_32F &&
                      !are_float_matrices_different<float>(
                          get_threshold<float>(Factor, CV_MAT_CN(Format)),
                          actual_mat, expected_mat)) ||
                     (CV_MAT_DEPTH(Format) == CV_8U &&
                      !are_matrices_different<uint8_t>(
                          get_threshold<uint8_t>(Factor, CV_MAT_CN(Format)),
                          actual_mat, expected_mat));
      if (!success) {
        std::cout << "Failed for src_width=" << input_mat.cols
                  << " src_height=" << input_mat.rows
                  << "  dst_width=" << expected_mat.cols
                  << " dst_height=" << expected_mat.rows << std::endl;
        fail_print_matrices(h, w, input_mat, actual_mat, expected_mat);
        return true;
      }
    }
  }
  return false;
}

template <int Type, size_t Format, int FactorMin, int FactorMax>
bool test_resize_random_scale(int index, RecreatedMessageQueue& request_queue,
                              RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t i = 0; i < 20; ++i) {
    size_t src_width = 40;
    size_t src_height = 12;
    // Extra lines contain the scaling factors
    cv::Mat input_mat(src_height + 2, src_width, Format);
    rng.fill(input_mat, cv::RNG::UNIFORM, 0, 255);

    double scale_x = rng.uniform(FactorMin / 1000.0, FactorMax / 1000.0);
    double scale_y = rng.uniform(FactorMin / 1000.0, FactorMax / 1000.0);

    // scale_x and scale_y is embedded into the input matrix
    memcpy(&input_mat.at<uint8_t>(input_mat.rows - 2, 0), &scale_x,
           sizeof(scale_x));
    memcpy(&input_mat.at<uint8_t>(input_mat.rows - 1, 0), &scale_y,
           sizeof(scale_y));

    cv::Mat actual_mat = exec_resize_random_scale<Type>(input_mat);
    cv::Mat expected_mat = get_expected_from_subordinate(
        index, request_queue, reply_queue, input_mat);

    uint8_t threshold = get_threshold<uint8_t>(0, input_mat.channels());
    if (are_matrices_different<uint8_t>(threshold, actual_mat, expected_mat)) {
      std::cout << "Failed for src_width=" << input_mat.cols
                << " src_height=" << (input_mat.rows - 2)
                << "  dst_width=" << expected_mat.cols
                << " dst_height=" << expected_mat.rows << std::endl;
      fail_print_matrices(actual_mat.rows, actual_mat.cols, input_mat,
                          actual_mat, expected_mat);
      return true;
    }
  }
  return false;
}

template <int Type, size_t Format>
bool test_resize_to_third(int index, RecreatedMessageQueue& request_queue,
                          RecreatedMessageQueue& reply_queue) {
  cv::RNG rng(0);

  for (size_t i = 0; i <= 10; ++i) {
    size_t dst_width = rng.uniform(100, 200);  // [100, 200)
    size_t dst_height = 10;
    cv::Mat input_mat(dst_height * 3, dst_width * 3, Format);
    rng.fill(input_mat, cv::RNG::UNIFORM, 0, 255);

    cv::Mat actual_mat = exec_resize_to_third<Type>(input_mat);
    cv::Mat expected_mat = get_expected_from_subordinate(
        index, request_queue, reply_queue, input_mat);

    uint8_t threshold = get_threshold<uint8_t>(0, input_mat.channels());
    if (are_matrices_different<uint8_t>(threshold, actual_mat, expected_mat)) {
      fail_print_matrices(dst_height, dst_width, input_mat, actual_mat,
                          expected_mat);
      return true;
    }
  }
  return false;
}
#endif

static void empty_resize(const cv::Mat& input, cv::Mat& output) {
  cv::resize(input, output, cv::Size(4, 4));
}

std::vector<test>& resize_tests_get() {
  // clang-format off
  static std::vector<test> tests = {
      EMPTY_INPUT_TEST("resize", CV_8UC1, exec_empty_input<empty_resize>),
      EMPTY_INPUT_TEST("resize", CV_8UC3, exec_empty_input<empty_resize>),
      EMPTY_INPUT_TEST("resize", CV_8UC4, exec_empty_input<empty_resize>),
      EMPTY_INPUT_TEST("resize", CV_32FC1, exec_empty_input<empty_resize>),

    TEST("Resize2x2 float32, INTER_LINEAR", (test_resize<2000, CV_HAL_INTER_LINEAR, 5, 16, CV_32FC1>), (exec_resize<2000, CV_HAL_INTER_LINEAR>)),
    TEST("Resize4x4 float32, INTER_LINEAR", (test_resize<4000, CV_HAL_INTER_LINEAR, 5, 16, CV_32FC1>), (exec_resize<4000, CV_HAL_INTER_LINEAR>)),
    TEST("Resize8x8 float32, INTER_LINEAR", (test_resize<8000, CV_HAL_INTER_LINEAR, 5, 16, CV_32FC1>), (exec_resize<8000, CV_HAL_INTER_LINEAR>)),

    TEST("Resize2x2 uint8, INTER_LINEAR", (test_resize<2000, CV_HAL_INTER_LINEAR, 5, 16, CV_8UC1>), (exec_resize<2000, CV_HAL_INTER_LINEAR>)),
    TEST("Resize4x4 uint8, INTER_LINEAR", (test_resize<4000, CV_HAL_INTER_LINEAR, 5, 16, CV_8UC1>), (exec_resize<4000, CV_HAL_INTER_LINEAR>)),

    TEST("Resize0.5x0.5 uint8, INTER_AREA, 1 channel", (test_resize<500, CV_HAL_INTER_AREA, 5, 32, CV_8UC1>), (exec_resize<500, CV_HAL_INTER_AREA>)),
    TEST("Resize0.5x0.5 uint8, INTER_LINEAR, 1 channel", (test_resize<500, CV_HAL_INTER_LINEAR, 5, 32, CV_8UC1>), (exec_resize<500, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.5x0.5 uint8, INTER_LINEAR, odd source and rounded scale", (test_resize_odd_source_rounded_scale_fallback), (exec_resize_with_scale<500, CV_HAL_INTER_LINEAR>)),
    TEST("Resize uint8, INTER_LINEAR, scale tolerance boundary", (test_resize_scale_tolerance_boundary), (exec_resize_scale_tolerance_boundary)),
    TEST("Resize0.777x0.777 uint8, INTER_LINEAR, 1 channel", (test_resize<777, CV_HAL_INTER_LINEAR, 21, 64, CV_8UC1>), (exec_resize<777, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.444x0.444 uint8, INTER_LINEAR, 1 channel", (test_resize<444, CV_HAL_INTER_LINEAR, 21, 64, CV_8UC1>), (exec_resize<444, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.3x0.3 uint8, INTER_LINEAR, 1 channel", (test_resize_to_third<CV_HAL_INTER_LINEAR, CV_8UC1>), (exec_resize_to_third<CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.05x1.05 uint8, INTER_LINEAR, 1 channel", (test_resize<1050, CV_HAL_INTER_LINEAR, 21, 64, CV_8UC1>), (exec_resize<1050, CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.55x1.55 uint8, INTER_LINEAR, 1 channel", (test_resize<1550, CV_HAL_INTER_LINEAR, 21, 64, CV_8UC1>), (exec_resize<1550, CV_HAL_INTER_LINEAR>)),
    // Temporary disable mixed horizontal upscaling with vertical downscaling or vice versa because a related issue is under investigation in OpenCV
    // TEST("Resize random scale uint8, INTER_LINEAR, 1 channel", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC1, 334, 1400>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize random scale uint8, INTER_LINEAR, 1 channel, downscale", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC1, 334, 999>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize random scale uint8, INTER_LINEAR, 1 channel, upscale", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC1, 1000, 1400>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize checkerboard compatibility uint8, INTER_LINEAR, 1 channel", (test_resize_checkerboard_compatibility<2852, 2, 3491, 53, CV_HAL_INTER_LINEAR, CV_8UC1>), (exec_resize_to_size<3491, 53, CV_HAL_INTER_LINEAR>)),

    TEST("Resize0.5x0.5 uint8, INTER_AREA, 2 channels", (test_resize<500, CV_HAL_INTER_AREA, 5, 32, CV_8UC2>), (exec_resize<500, CV_HAL_INTER_AREA>)),
    TEST("Resize0.5x0.5 uint8, INTER_LINEAR, 2 channels", (test_resize<500, CV_HAL_INTER_LINEAR, 5, 32, CV_8UC2>), (exec_resize<500, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.777x0.777 uint8, INTER_LINEAR, 2 channels", (test_resize<777, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC2>), (exec_resize<777, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.444x0.444 uint8, INTER_LINEAR, 2 channels", (test_resize<444, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC2>), (exec_resize<444, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.3x0.3 uint8, INTER_LINEAR, 2 channels", (test_resize_to_third<CV_HAL_INTER_LINEAR, CV_8UC2>), (exec_resize_to_third<CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.05x1.05 uint8, INTER_LINEAR, 2 channels", (test_resize<1050, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC2>), (exec_resize<1050, CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.55x1.55 uint8, INTER_LINEAR, 2 channels", (test_resize<1550, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC2>), (exec_resize<1550, CV_HAL_INTER_LINEAR>)),
    TEST("Resize random scale uint8, INTER_LINEAR, 2 channels", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC2, 334, 1999>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize checkerboard compatibility uint8, INTER_LINEAR, 2 channels", (test_resize_checkerboard_compatibility<11869, 1, 8987, 1, CV_HAL_INTER_LINEAR, CV_8UC2>), (exec_resize_to_size<8987, 1, CV_HAL_INTER_LINEAR>)),

    TEST("Resize0.5x0.5 uint8, INTER_AREA, 3 channels", (test_resize<500, CV_HAL_INTER_AREA, 5, 32, CV_8UC3>), (exec_resize<500, CV_HAL_INTER_AREA>)),
    TEST("Resize0.5x0.5 uint8, INTER_LINEAR, 3 channels", (test_resize<500, CV_HAL_INTER_LINEAR, 5, 32, CV_8UC3>), (exec_resize<500, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.777x0.777 uint8, INTER_LINEAR, 3 channels", (test_resize<777, CV_HAL_INTER_LINEAR, 11, 32, CV_8UC3>), (exec_resize<777, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.444x0.444 uint8, INTER_LINEAR, 3 channels", (test_resize<444, CV_HAL_INTER_LINEAR, 11, 32, CV_8UC3>), (exec_resize<444, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.333x0.333 uint8, INTER_LINEAR, 3 channels", (test_resize_to_third<CV_HAL_INTER_LINEAR, CV_8UC3>), (exec_resize_to_third<CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.05x1.05 uint8, INTER_LINEAR, 3 channels", (test_resize<1050, CV_HAL_INTER_LINEAR, 11, 32, CV_8UC3>), (exec_resize<1050, CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.55x1.55 uint8, INTER_LINEAR, 3 channels", (test_resize<1550, CV_HAL_INTER_LINEAR, 8, 32, CV_8UC3>), (exec_resize<1550, CV_HAL_INTER_LINEAR>)),
    TEST("Resize random scale uint8, INTER_LINEAR, 3 channels", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC3, 334, 1999>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize checkerboard compatibility uint8, INTER_LINEAR, 3 channels", (test_resize_checkerboard_compatibility<20540, 1, 9434, 1, CV_HAL_INTER_LINEAR, CV_8UC3>), (exec_resize_to_size<9434, 1, CV_HAL_INTER_LINEAR>)),

    TEST("Resize0.5x0.5 uint8, INTER_AREA, 4 channels", (test_resize<500, CV_HAL_INTER_AREA, 5, 32, CV_8UC4>), (exec_resize<500, CV_HAL_INTER_AREA>)),
    TEST("Resize0.5x0.5 uint8, INTER_LINEAR, 4 channels", (test_resize<500, CV_HAL_INTER_LINEAR, 5, 32, CV_8UC4>), (exec_resize<500, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.777x0.777 uint8, INTER_LINEAR, 4 channels", (test_resize<777, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC4>), (exec_resize<777, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.444x0.444 uint8, INTER_LINEAR, 4 channels", (test_resize<444, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC4>), (exec_resize<444, CV_HAL_INTER_LINEAR>)),
    TEST("Resize0.3x0.3 uint8, INTER_LINEAR, 4 channels", (test_resize_to_third<CV_HAL_INTER_LINEAR, CV_8UC4>), (exec_resize_to_third<CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.05x1.05 uint8, INTER_LINEAR, 4 channels", (test_resize<1050, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC4>), (exec_resize<1050, CV_HAL_INTER_LINEAR>)),
    TEST("Resize1.55x1.55 uint8, INTER_LINEAR, 4 channels", (test_resize<1550, CV_HAL_INTER_LINEAR, 16, 32, CV_8UC4>), (exec_resize<1550, CV_HAL_INTER_LINEAR>)),
    TEST("Resize random scale uint8, INTER_LINEAR, 4 channels", (test_resize_random_scale<CV_HAL_INTER_LINEAR, CV_8UC4, 334, 1999>), (exec_resize_random_scale<CV_HAL_INTER_LINEAR>)),
    TEST("Resize checkerboard compatibility uint8, INTER_LINEAR, 4 channels", (test_resize_checkerboard_compatibility<3614, 2, 4569, 13, CV_HAL_INTER_LINEAR, CV_8UC4>), (exec_resize_to_size<4569, 13, CV_HAL_INTER_LINEAR>)),
};
  // clang-format on
  return tests;
}
