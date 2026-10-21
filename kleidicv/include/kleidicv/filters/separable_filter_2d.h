// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_FILTERS_SEPARABLE_FILTER_2D_H
#define KLEIDICV_FILTERS_SEPARABLE_FILTER_2D_H

#include "kleidicv/config.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/types.h"
#include "kleidicv/utils.h"
#include "kleidicv/workspace/border_types.h"

extern "C" {
// For internal use only. See instead kleidicv_separable_filter_2d_u8.
// Filter a horizontal stripe across an image. The stripe is defined by the
// range [y_begin, y_end).
KLEIDICV_API_DECLARATION(kleidicv_separable_filter_2d_stripe_u8,
                         const uint8_t *src, size_t src_stride, uint8_t *dst,
                         size_t dst_stride, size_t width, size_t height,
                         size_t y_begin, size_t y_end, size_t channels,
                         const uint8_t *kernel_x, size_t kernel_width,
                         const uint8_t *kernel_y, size_t kernel_height,
                         kleidicv::FixedBorderType border_type);
// Explicit SME entry point used by the threaded dispatcher.
KLEIDICV_API_DECLARATION(kleidicv_separable_filter_2d_stripe_u8_sme,
                         const uint8_t *src, size_t src_stride, uint8_t *dst,
                         size_t dst_stride, size_t width, size_t height,
                         size_t y_begin, size_t y_end, size_t channels,
                         const uint8_t *kernel_x, size_t kernel_width,
                         const uint8_t *kernel_y, size_t kernel_height,
                         kleidicv::FixedBorderType border_type);
// For internal use only. See instead kleidicv_separable_filter_2d_u16.
// Filter a horizontal stripe across an image. The stripe is defined by the
// range [y_begin, y_end).
KLEIDICV_API_DECLARATION(kleidicv_separable_filter_2d_stripe_u16,
                         const uint16_t *src, size_t src_stride, uint16_t *dst,
                         size_t dst_stride, size_t width, size_t height,
                         size_t y_begin, size_t y_end, size_t channels,
                         const uint16_t *kernel_x, size_t kernel_width,
                         const uint16_t *kernel_y, size_t kernel_height,
                         kleidicv::FixedBorderType border_type);
// Explicit SME entry point used by the threaded dispatcher.
KLEIDICV_API_DECLARATION(kleidicv_separable_filter_2d_stripe_u16_sme,
                         const uint16_t *src, size_t src_stride, uint16_t *dst,
                         size_t dst_stride, size_t width, size_t height,
                         size_t y_begin, size_t y_end, size_t channels,
                         const uint16_t *kernel_x, size_t kernel_width,
                         const uint16_t *kernel_y, size_t kernel_height,
                         kleidicv::FixedBorderType border_type);
}

namespace kleidicv {

template <typename T>
FixedBorderTypeValidationResult separable_filter_2d_validate(
    const T *src, size_t src_stride, T *dst, size_t dst_stride, size_t width,
    size_t height, size_t channels, const T *kernel_x, size_t kernel_width,
    const T *kernel_y, size_t kernel_height,
    kleidicv_border_type_t border_type) {
  if (kleidicv_error_t check_error =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(src, src_stride,
                                                              height)) {
    return {check_error, FixedBorderType{}};
  }

  if (kleidicv_error_t check_error =
          KLEIDICV_TARGET_NAMESPACE::check_pointer_and_stride(dst, dst_stride,
                                                              height)) {
    return {check_error, FixedBorderType{}};
  }

  if (kleidicv_error_t check_error =
          KLEIDICV_TARGET_NAMESPACE::check_image_size(width, height,
                                                      channels)) {
    return {check_error, FixedBorderType{}};
  }

  if (kernel_width != 5 || kernel_height != 5) {
    return {KLEIDICV_ERROR_NOT_IMPLEMENTED, FixedBorderType{}};
  }

  if (width < kernel_width - 1 || height < kernel_width - 1) {
    return {KLEIDICV_ERROR_NOT_IMPLEMENTED, FixedBorderType{}};
  }

  auto fixed_border_type = get_fixed_border_type(border_type);
  if (!fixed_border_type) {
    return {KLEIDICV_ERROR_NOT_IMPLEMENTED, FixedBorderType{}};
  }

  if (kleidicv_error_t check_error =
          KLEIDICV_TARGET_NAMESPACE::check_pointers(kernel_x, kernel_y)) {
    return {check_error, FixedBorderType{}};
  }

  return {KLEIDICV_OK, *fixed_border_type};
}

namespace neon {

template <typename T>
kleidicv_error_t separable_filter_2d_stripe(
    const T *src, size_t src_stride, T *dst, size_t dst_stride, size_t width,
    size_t height, size_t y_begin, size_t y_end, size_t channels,
    const T *kernel_x, size_t kernel_width, const T *kernel_y,
    size_t kernel_height, FixedBorderType border_type);

}  // namespace neon

namespace sve2 {

template <typename T>
kleidicv_error_t separable_filter_2d_stripe(
    const T *src, size_t src_stride, T *dst, size_t dst_stride, size_t width,
    size_t height, size_t y_begin, size_t y_end, size_t channels,
    const T *kernel_x, size_t kernel_width, const T *kernel_y,
    size_t kernel_height, FixedBorderType border_type);

}  // namespace sve2

namespace sme {

template <typename T>
kleidicv_error_t separable_filter_2d_stripe(
    const T *src, size_t src_stride, T *dst, size_t dst_stride, size_t width,
    size_t height, size_t y_begin, size_t y_end, size_t channels,
    const T *kernel_x, size_t kernel_width, const T *kernel_y,
    size_t kernel_height, FixedBorderType border_type);

}  // namespace sme

}  // namespace kleidicv

#endif  // KLEIDICV_FILTERS_SEPARABLE_FILTER_2D_H
