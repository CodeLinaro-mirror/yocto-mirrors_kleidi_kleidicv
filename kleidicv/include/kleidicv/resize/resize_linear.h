// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_RESIZE_RESIZE_LINEAR_H
#define KLEIDICV_RESIZE_RESIZE_LINEAR_H

#include <algorithm>
#include <array>
#include <cstddef>

#include "kleidicv/kleidicv.h"
#include "kleidicv/utils.h"

namespace kleidicv {

inline kleidicv_error_t resize_linear_u8_validate(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    uint8_t *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t channels) {
  if (src_width > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      src_height > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      dst_width > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      dst_height > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT || channels == 0) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  const bool is_empty = src_width == 0 || src_height == 0;
  const bool is_half_downscale = src_width == 2 * dst_width &&
                                 src_height == 2 * dst_height && channels <= 4;

  const std::array<size_t, 2> implemented_ratios = {2, 4};
  const bool is_fixed_upscale =
      channels == 1 && std::any_of(implemented_ratios.begin(),
                                   implemented_ratios.end(), [&](size_t ratio) {
                                     return src_width * ratio == dst_width &&
                                            src_height * ratio == dst_height;
                                   });

  // Generic upscaling needs a minimum width for vector operations.
  const bool is_generic_upscale =
      channels <= 4 && dst_width >= src_width && src_width * channels >= 16;

  // Generic downscaling supports horizontal ratios from 1/3 to 1/1. The
  // required source width depends on the selected vector path.
  const bool is_generic_downscale_shape =
      channels <= 4 && dst_width * 3 >= src_width && dst_width < src_width &&
      dst_width * channels >= 8;
  const bool uses_two_vectors = dst_width * 2 >= src_width && channels != 3;
  const bool is_generic_downscale =
      is_generic_downscale_shape &&
      src_width * channels >= (uses_two_vectors ? 16 : 32);

  const bool is_implemented = is_empty || is_half_downscale ||
                              is_fixed_upscale || is_generic_upscale ||
                              is_generic_downscale;

  if (!is_implemented) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  if (kleidicv_error_t err =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(src, src_stride,
                                                              src_height)) {
    return err;
  }
  if (kleidicv_error_t err =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(dst, dst_stride,
                                                              dst_height)) {
    return err;
  }
  return KLEIDICV_OK;
}

inline bool resize_linear_u8_is_2x2(size_t src_width, size_t src_height,
                                    size_t dst_width, size_t dst_height,
                                    size_t channels) {
  return channels == 1 && src_width * 2 == dst_width &&
         src_height * 2 == dst_height;
}

inline bool resize_linear_u8_is_4x4(size_t src_width, size_t src_height,
                                    size_t dst_width, size_t dst_height,
                                    size_t channels) {
  return channels == 1 && src_width * 4 == dst_width &&
         src_height * 4 == dst_height;
}

inline kleidicv_error_t resize_linear_f32_validate(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    float *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t channels) {
  if (src_width > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      src_height > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      dst_width > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT ||
      dst_height > KLEIDICV_RESIZE_MAX_WIDTH_OR_HEIGHT || channels != 1) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  const std::array<size_t, 3> implemented_ratios = {2, 4, 8};
  const bool has_implemented_ratio = std::any_of(
      implemented_ratios.begin(), implemented_ratios.end(), [&](size_t ratio) {
        return src_width * ratio == dst_width &&
               src_height * ratio == dst_height;
      });
  if (src_width != 0 && src_height != 0 && !has_implemented_ratio) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  if (kleidicv_error_t err =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(src, src_stride,
                                                              src_height)) {
    return err;
  }
  if (kleidicv_error_t err =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(dst, dst_stride,
                                                              dst_height)) {
    return err;
  }
  return KLEIDICV_OK;
}

namespace neon {
kleidicv_error_t resize_to_quarter_u8(const uint8_t *src, size_t src_stride,
                                      size_t src_width, size_t src_height,
                                      uint8_t *dst, size_t dst_stride,
                                      size_t channels);
kleidicv_error_t kleidicv_resize_2x2_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
kleidicv_error_t kleidicv_resize_4x4_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
template <int kRatio, int kChannels, bool kUpsize = false>
kleidicv_error_t kleidicv_resize_generic_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
kleidicv_error_t kleidicv_resize_linear_stripe_f32(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
}  // namespace neon

namespace sve2 {
kleidicv_error_t resize_to_quarter_u8(const uint8_t *src, size_t src_stride,
                                      size_t src_width, size_t src_height,
                                      uint8_t *dst, size_t dst_stride,
                                      size_t channels);
kleidicv_error_t kleidicv_resize_2x2_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
kleidicv_error_t kleidicv_resize_4x4_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
template <int kRatio, int kChannels, bool kUpsize = false>
kleidicv_error_t kleidicv_resize_generic_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
kleidicv_error_t kleidicv_resize_linear_stripe_f32(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
}  // namespace sve2

namespace sme {
kleidicv_error_t resize_to_quarter_u8(const uint8_t *src, size_t src_stride,
                                      size_t src_width, size_t src_height,
                                      uint8_t *dst, size_t dst_stride,
                                      size_t channels);
kleidicv_error_t kleidicv_resize_2x2_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
kleidicv_error_t kleidicv_resize_4x4_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
template <int kRatio, int kChannels, bool kUpsize = false>
kleidicv_error_t kleidicv_resize_generic_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
kleidicv_error_t kleidicv_resize_linear_stripe_f32(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
}  // namespace sme

namespace sme2 {
kleidicv_error_t resize_to_quarter_u8(const uint8_t *src, size_t src_stride,
                                      size_t src_width, size_t src_height,
                                      uint8_t *dst, size_t dst_stride,
                                      size_t channels);
kleidicv_error_t kleidicv_resize_2x2_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
kleidicv_error_t kleidicv_resize_4x4_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);
template <int kRatio, int kChannels, bool kUpsize = false>
kleidicv_error_t kleidicv_resize_generic_stripe_u8(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
kleidicv_error_t kleidicv_resize_linear_stripe_f32(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);
}  // namespace sme2

// Fixed-scale kernels use source-row bounds for y_begin and y_end.
using ResizeLinearFixedScaleStripeU8 = kleidicv_error_t (*)(
    const uint8_t *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, uint8_t *dst, size_t dst_stride);

// The generic stripe dispatcher uses destination-row bounds.
template <bool kUseSME>
kleidicv_error_t resize_linear_stripe_u8(const uint8_t *src, size_t src_stride,
                                         size_t src_width, size_t src_height,
                                         size_t y_begin, size_t y_end,
                                         uint8_t *dst, size_t dst_stride,
                                         size_t dst_width, size_t dst_height,
                                         size_t channels);

}  // namespace kleidicv

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// Internal - not part of the public API and its direct use is not
/// supported. These are used by the multithreaded function.
extern kleidicv::ResizeLinearFixedScaleStripeU8 kleidicv_resize_2x2_stripe_u8;
extern kleidicv::ResizeLinearFixedScaleStripeU8
    kleidicv_resize_2x2_stripe_u8_sme;
extern kleidicv::ResizeLinearFixedScaleStripeU8 kleidicv_resize_4x4_stripe_u8;
extern kleidicv::ResizeLinearFixedScaleStripeU8
    kleidicv_resize_4x4_stripe_u8_sme;

/// Internal - not part of the public API and its direct use is not
/// supported. It is used by the multithreaded function. The stripe range uses
/// source-row bounds.
extern kleidicv_error_t (*kleidicv_resize_linear_stripe_f32)(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);

/// Internal - not part of the public API and its direct use is not
/// supported. It is used by the multithreaded function. The stripe range uses
/// source-row bounds.
extern kleidicv_error_t (*kleidicv_resize_linear_stripe_f32_sme)(
    const float *src, size_t src_stride, size_t src_width, size_t src_height,
    size_t y_begin, size_t y_end, float *dst, size_t dst_stride,
    size_t dst_width, size_t dst_height);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // KLEIDICV_RESIZE_RESIZE_H
