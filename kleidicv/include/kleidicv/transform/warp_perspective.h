// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_TRANSFORM_WARP_PERSPECTIVE_H
#define KLEIDICV_TRANSFORM_WARP_PERSPECTIVE_H

#include <type_traits>

#include "kleidicv/ctypes.h"
#include "kleidicv/utils.h"

extern "C" {
// For internal use only. See kleidicv_warp_perspective_u8 instead.
// Calculates a stripe of the `dst` image that is a transformed part of `src`.
// The stripe is defined by the range [y_begin, y_end).
KLEIDICV_API_DECLARATION(kleidicv_warp_perspective_stripe_u8,
                         const uint8_t *src, size_t src_stride,
                         size_t src_width, size_t src_height, uint8_t *dst,
                         size_t dst_stride, size_t dst_width, size_t dst_height,
                         size_t y_begin, size_t y_end,
                         const float transformation[9], size_t channels,
                         kleidicv_interpolation_type_t interpolation,
                         kleidicv_border_type_t border_type,
                         const uint8_t *border_value);
}
namespace kleidicv {

template <typename T>
inline kleidicv_error_t warp_perspective_validate(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    const float transformation[9], size_t channels,
    kleidicv_interpolation_type_t interpolation,
    kleidicv_border_type_t border_type, const T *border_value) {
  if constexpr (!std::is_same<T, uint8_t>::value) {
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
  if (transformation == nullptr) {
    return KLEIDICV_ERROR_NULL_POINTER;
  }

  if (border_type == KLEIDICV_BORDER_TYPE_CONSTANT && border_value == nullptr) {
    return KLEIDICV_ERROR_NULL_POINTER;
  }

  CHECK_IMAGE_SIZE(src_width, src_height, channels);
  CHECK_IMAGE_SIZE(dst_width, dst_height);

  if (dst_width < 8 ||
      (interpolation != KLEIDICV_INTERPOLATION_NEAREST &&
       interpolation != KLEIDICV_INTERPOLATION_LINEAR) ||
      (border_type != KLEIDICV_BORDER_TYPE_REPLICATE &&
       border_type != KLEIDICV_BORDER_TYPE_CONSTANT) ||
      channels != 1) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  // Calculating in float32_t will only be precise until 24 bits, and
  // multiplication can only be done with 32x32 bits.
  if (src_width >= (1ULL << 24) || src_height >= (1ULL << 24) ||
      dst_width >= (1ULL << 24) || dst_height >= (1ULL << 24) ||
      src_stride >= (1ULL << 32)) {
    return KLEIDICV_ERROR_RANGE;
  }

  return KLEIDICV_OK;
}

namespace neon {

template <typename T>
kleidicv_error_t warp_perspective_stripe(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t y_begin, size_t y_end, const float transformation[9],
    size_t channels, kleidicv_interpolation_type_t interpolation,
    kleidicv_border_type_t border_type, const T *border_value);
}  // namespace neon

namespace sve2 {

template <typename T>
kleidicv_error_t warp_perspective_stripe(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t y_begin, size_t y_end, const float transformation[9],
    size_t channels, kleidicv_interpolation_type_t interpolation,
    kleidicv_border_type_t border_type, const T *border_value);
}  // namespace sve2

}  // namespace kleidicv

#endif  // KLEIDICV_TRANSFORM_WARP_PERSPECTIVE_H
