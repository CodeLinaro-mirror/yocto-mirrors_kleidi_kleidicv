// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_REMAP_REMAP_H
#define KLEIDICV_REMAP_REMAP_H

#include <limits>
#include <type_traits>

#include "kleidicv/ctypes.h"
#include "kleidicv/utils.h"

namespace KLEIDICV_TARGET_NAMESPACE {

template <typename T>
inline kleidicv_error_t remap_s16_validate(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t channels, const int16_t *mapxy, size_t mapxy_stride,
    kleidicv_border_type_t border_type,
    const T *border_value) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src, src_stride, src_height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, dst_height);
  CHECK_POINTER_AND_STRIDE(mapxy, mapxy_stride, dst_height);
  CHECK_IMAGE_SIZE(src_width, src_height);
  CHECK_IMAGE_SIZE(dst_width, dst_height);
  if (border_type == KLEIDICV_BORDER_TYPE_CONSTANT && border_value == nullptr) {
    return KLEIDICV_ERROR_NULL_POINTER;
  }

  if constexpr (std::is_same<T, uint8_t>::value ||
                std::is_same<T, uint16_t>::value) {
    if (src_stride / sizeof(T) <= std::numeric_limits<uint16_t>::max() &&
        dst_width >= 8 &&
        src_width <= std::numeric_limits<int16_t>::max() + 1 &&
        src_height <= std::numeric_limits<int16_t>::max() + 1 &&
        (border_type == KLEIDICV_BORDER_TYPE_REPLICATE ||
         border_type == KLEIDICV_BORDER_TYPE_CONSTANT) &&
        channels == 1) {
      return KLEIDICV_OK;
    }
  }
  return KLEIDICV_ERROR_NOT_IMPLEMENTED;
}

// These validators are just above the complexity threshold because CHECK_*
// macros add do/while nesting. Keeping the macros makes the validation order
// and error precedence easier to follow, so ignore the warning.
// NOLINTBEGIN(readability-function-cognitive-complexity)
template <typename T>
inline kleidicv_error_t remap_s16point5_validate(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t channels, const int16_t *mapxy, size_t mapxy_stride,
    const uint16_t *mapfrac, size_t mapfrac_stride,
    kleidicv_border_type_t border_type,
    const T *border_value) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src, src_stride, src_height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, dst_height);
  CHECK_POINTER_AND_STRIDE(mapxy, mapxy_stride, dst_height);
  CHECK_POINTER_AND_STRIDE(mapfrac, mapfrac_stride, dst_height);
  CHECK_IMAGE_SIZE(src_width, src_height);
  CHECK_IMAGE_SIZE(dst_width, dst_height);
  if (border_type == KLEIDICV_BORDER_TYPE_CONSTANT && border_value == nullptr) {
    return KLEIDICV_ERROR_NULL_POINTER;
  }

  if constexpr (std::is_same<T, uint8_t>::value ||
                std::is_same<T, uint16_t>::value) {
    if ((channels == 1 || channels == 4) &&
        src_stride / sizeof(T) <=
            (std::numeric_limits<uint16_t>::max() / channels) &&
        dst_width >= 8 &&
        src_width <= std::numeric_limits<int16_t>::max() + 1 &&
        src_height <= std::numeric_limits<int16_t>::max() + 1 &&
        (border_type == KLEIDICV_BORDER_TYPE_REPLICATE ||
         border_type == KLEIDICV_BORDER_TYPE_CONSTANT)) {
      return KLEIDICV_OK;
    }
  }
  return KLEIDICV_ERROR_NOT_IMPLEMENTED;
}

template <typename T>
inline kleidicv_error_t remap_f32_validate(
    const T *src, size_t src_stride, size_t src_width, size_t src_height,
    T *dst, size_t dst_stride, size_t dst_width, size_t dst_height,
    size_t channels, const float *mapx, size_t mapx_stride, const float *mapy,
    size_t mapy_stride, kleidicv_interpolation_type_t interpolation,
    kleidicv_border_type_t border_type,
    const T *border_value) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src, src_stride, src_height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, dst_height);
  CHECK_POINTER_AND_STRIDE(mapx, mapx_stride, dst_height);
  CHECK_POINTER_AND_STRIDE(mapy, mapy_stride, dst_height);
  CHECK_IMAGE_SIZE(src_width, src_height);
  CHECK_IMAGE_SIZE(dst_width, dst_height);
  if (border_type == KLEIDICV_BORDER_TYPE_CONSTANT && border_value == nullptr) {
    return KLEIDICV_ERROR_NULL_POINTER;
  }

  if constexpr (std::is_same<T, uint8_t>::value ||
                std::is_same<T, uint16_t>::value) {
    if (src_stride <= std::numeric_limits<uint32_t>::max() && dst_width >= 4 &&
        src_width < (1ULL << 24) && src_height < (1ULL << 24) &&
        dst_width < (1ULL << 24) && dst_height < (1ULL << 24) &&
        src_width > 0 && src_height > 0 &&
        (border_type == KLEIDICV_BORDER_TYPE_REPLICATE ||
         border_type == KLEIDICV_BORDER_TYPE_CONSTANT) &&
        (channels == 1 || channels == 2) &&
        (interpolation == KLEIDICV_INTERPOLATION_LINEAR ||
         interpolation == KLEIDICV_INTERPOLATION_NEAREST)) {
      return KLEIDICV_OK;
    }
  }
  return KLEIDICV_ERROR_NOT_IMPLEMENTED;
}
// NOLINTEND(readability-function-cognitive-complexity)

}  // namespace KLEIDICV_TARGET_NAMESPACE

namespace kleidicv {

// Constants for Remap16Point5
static const uint16_t REMAP16POINT5_FRAC_BITS = 5;
static const uint16_t REMAP16POINT5_FRAC_MAX = 1 << REMAP16POINT5_FRAC_BITS;
static const uint16_t REMAP16POINT5_FRAC_MAX_SQUARE =
    REMAP16POINT5_FRAC_MAX * REMAP16POINT5_FRAC_MAX;

namespace neon {

template <typename T>
kleidicv_error_t remap_s16(const T *src, size_t src_stride, size_t src_width,
                           size_t src_height, T *dst, size_t dst_stride,
                           size_t dst_width, size_t dst_height, size_t channels,
                           const int16_t *mapxy, size_t mapxy_stride,
                           kleidicv_border_type_t border_type,
                           const T *border_value);

template <typename T>
kleidicv_error_t remap_s16point5(const T *src, size_t src_stride,
                                 size_t src_width, size_t src_height, T *dst,
                                 size_t dst_stride, size_t dst_width,
                                 size_t dst_height, size_t channels,
                                 const int16_t *mapxy, size_t mapxy_stride,
                                 const uint16_t *mapfrac, size_t mapfrac_stride,
                                 kleidicv_border_type_t border_type,
                                 const T *border_value);

template <typename T>
kleidicv_error_t remap_f32(const T *src, size_t src_stride, size_t src_width,
                           size_t src_height, T *dst, size_t dst_stride,
                           size_t dst_width, size_t dst_height, size_t channels,
                           const float *mapx, size_t mapx_stride,
                           const float *mapy, size_t mapy_stride,
                           kleidicv_interpolation_type_t interpolation,
                           kleidicv_border_type_t border_type,
                           const T *border_value);

}  // namespace neon

namespace sve2 {

template <typename T>
kleidicv_error_t remap_s16(const T *src, size_t src_stride, size_t src_width,
                           size_t src_height, T *dst, size_t dst_stride,
                           size_t dst_width, size_t dst_height, size_t channels,
                           const int16_t *mapxy, size_t mapxy_stride,
                           kleidicv_border_type_t border_type,
                           const T *border_value);

template <typename T>
kleidicv_error_t remap_s16point5(const T *src, size_t src_stride,
                                 size_t src_width, size_t src_height, T *dst,
                                 size_t dst_stride, size_t dst_width,
                                 size_t dst_height, size_t channels,
                                 const int16_t *mapxy, size_t mapxy_stride,
                                 const uint16_t *mapfrac, size_t mapfrac_stride,
                                 kleidicv_border_type_t border_type,
                                 const T *border_value);

template <typename T>
kleidicv_error_t remap_f32(const T *src, size_t src_stride, size_t src_width,
                           size_t src_height, T *dst, size_t dst_stride,
                           size_t dst_width, size_t dst_height, size_t channels,
                           const float *mapx, size_t mapx_stride,
                           const float *mapy, size_t mapy_stride,
                           kleidicv_interpolation_type_t interpolation,
                           kleidicv_border_type_t border_type,
                           const T *border_value);
}  // namespace sve2

}  // namespace kleidicv

#endif  // KLEIDICV_REMAP_REMAP_H
