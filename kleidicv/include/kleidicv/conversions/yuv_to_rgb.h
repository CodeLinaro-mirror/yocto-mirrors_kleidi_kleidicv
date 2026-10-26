// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_YUV_TO_RGB_H
#define KLEIDICV_CONVERSIONS_YUV_TO_RGB_H

#include "kleidicv/conversions/yuv_common.h"
#include "kleidicv/kleidicv.h"

extern "C" {
// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts an interleaved YUV444 image to RGB, RGBA, BGR, or BGRA format.
// All channels are 8-bit wide. Handle empty images and validate the complete
// image before invoking this worker.
KLEIDICV_API_DECLARATION(kleidicv_yuv444_to_rgb_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);

// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts an interleaved YUV422 image to RGB, RGBA, BGR, or BGRA format.
// All channels are 8-bit wide. Handle empty images and validate the complete
// image before invoking this worker.
KLEIDICV_API_DECLARATION(kleidicv_yuv422_to_rgb_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);

// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts a stripe (range of rows) of a planar YUV420 image (I420 or YV12)
// to RGB, RGBA, BGR, or BGRA format. All channels are 8-bit wide. The stripe
// range [begin, end) indexes chroma-plane rows. Handle empty images and
// validate the complete image before invoking this worker.
KLEIDICV_API_DECLARATION(kleidicv_yuv420p_to_rgb_stripe_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format, size_t begin,
                         size_t end);
}

namespace KLEIDICV_TARGET_NAMESPACE {

inline kleidicv_error_t yuv444_to_rgb_validate(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height,
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
  if (!is_yuv_format_supported<KLEIDICV_COLOR_CONVERSION_FMT_YUV444>(
          color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  return unary_operation_validate(src, src_stride, dst, dst_stride, width,
                                  height);
}

inline kleidicv_error_t yuv422_to_rgb_validate(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height,
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
  if (!is_yuv_format_supported<KLEIDICV_COLOR_CONVERSION_FMT_YUV422>(
          color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  if (kleidicv_error_t err = unary_operation_validate(
          src, src_stride, dst, dst_stride, width, height)) {
    return err;
  }

  // YUV422 stores pixel pairs, so the width must be at least two and even.
  if (width < 2 || (width % 2) != 0) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }

  return KLEIDICV_OK;
}

inline kleidicv_error_t yuv420p_to_rgb_validate(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height,
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
  if (!is_yuv_format_supported<KLEIDICV_COLOR_CONVERSION_FMT_YUV420P>(
          color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  return unary_operation_validate(src, src_stride, dst, dst_stride, width,
                                  height);
}

inline kleidicv_error_t yuv420sp_to_rgb_validate(
    const uint8_t *src_y, const uint8_t *src_uv, uint8_t *dst, size_t width,
    size_t height,
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
  if (!is_yuv_format_supported<KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP>(
          color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  CHECK_POINTERS(src_y, src_uv, dst);
  CHECK_IMAGE_SIZE(width, height);
  return KLEIDICV_OK;
}

// Form the UV address without pointer arithmetic on an unvalidated image.
// The semiplanar entry point validates both planes before accessing them.
inline const uint8_t *get_yuv420sp_uv_plane(const uint8_t *src,
                                            size_t src_stride, size_t width,
                                            size_t height) KLEIDICV_STREAMING {
  if (width == 0 || height == 0) {
    return src;
  }

  const uintptr_t uv_address =
      reinterpret_cast<uintptr_t>(src) + src_stride * height;
  // A wrapped address must not introduce a null UV plane and hide a size error.
  // NOLINTBEGIN(performance-no-int-to-ptr)
  return uv_address ? reinterpret_cast<const uint8_t *>(uv_address) : src;
  // NOLINTEND(performance-no-int-to-ptr)
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

namespace kleidicv {

namespace neon {
kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

}  // namespace neon

namespace sve2 {
kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

}  // namespace sve2

namespace sme {
kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

}  // namespace sme

}  // namespace kleidicv

#endif  // KLEIDICV_CONVERSIONS_YUV_TO_RGB_H
