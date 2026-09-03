// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_YUV_TO_RGB_H
#define KLEIDICV_CONVERSIONS_YUV_TO_RGB_H

#include "kleidicv/conversions/yuv420.h"
#include "kleidicv/kleidicv.h"

extern "C" {
// For internal use only. See instead kleidicv_yuv_semiplanar_to_rgb_u8.
// Converts semi-planar YUV420 (NV12 or NV21) to RGB, RGBA, BGR, or BGRA
// format. All channels are 8-bit wide. The planes may be sliced to a
// contiguous range of rows; validate the complete image before slicing.
KLEIDICV_API_DECLARATION(kleidicv_yuv420sp_to_rgb_u8, const uint8_t *src_y,
                         size_t src_y_stride, const uint8_t *src_uv,
                         size_t src_uv_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);
KLEIDICV_API_DECLARATION(kleidicv_yuv420sp_to_rgb_u8_sme, const uint8_t *src_y,
                         size_t src_y_stride, const uint8_t *src_uv,
                         size_t src_uv_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);

// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts a stripe (range of rows) of a planar YUV420 image (I420 or YV12)
// to RGB, RGBA, BGR, or BGRA format. All channels are 8-bit wide. The stripe
// range [begin, end) indexes chroma-plane rows. Validate the complete image
// before invoking this worker.
KLEIDICV_API_DECLARATION(kleidicv_yuv420p_to_rgb_stripe_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format, size_t begin,
                         size_t end);

// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts an interleaved YUV444 image to RGB, RGBA, BGR, or BGRA format.
// All channels are 8-bit wide.
KLEIDICV_API_DECLARATION(kleidicv_yuv444_to_rgb_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);

// For internal use only. See instead kleidicv_yuv_to_rgb_u8.
// Converts an interleaved YUV422 image to RGB, RGBA, BGR, or BGRA format.
// All channels are 8-bit wide.
KLEIDICV_API_DECLARATION(kleidicv_yuv422_to_rgb_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);
}

namespace kleidicv {

inline kleidicv_error_t yuv_semiplanar_to_rgb_validate(
    const uint8_t *src_y, size_t src_y_stride, const uint8_t *src_uv,
    size_t src_uv_stride, uint8_t *dst, size_t dst_stride, size_t width,
    size_t height, kleidicv_color_conversion_t color_format) {
  if (!is_yuv420sp_format_supported(color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  CHECK_POINTER_AND_STRIDE(src_y, src_y_stride, height);
  CHECK_POINTER_AND_STRIDE(src_uv, src_uv_stride, (height + 1) / 2);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

inline kleidicv_error_t yuv420sp_to_rgb_validate(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  CHECK_POINTER_AND_STRIDE(src, src_stride, (height * 3 + 1) / 2);
  if (!is_yuv420sp_format_supported(color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

inline kleidicv_error_t yuv420p_to_rgb_validate(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  if (!is_yuv420p_format_supported(color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  CHECK_POINTER_AND_STRIDE(src, src_stride, (height * 3 + 1) / 2);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

namespace neon {
kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

}  // namespace neon

namespace sve2 {
kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

}  // namespace sve2

namespace sme {
kleidicv_error_t yuv420sp_to_rgb_u8(const uint8_t *src_y, size_t src_y_stride,
                                    const uint8_t *src_uv, size_t src_uv_stride,
                                    uint8_t *dst, size_t dst_stride,
                                    size_t width, size_t height,
                                    kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t yuv444_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t yuv422_to_rgb_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

}  // namespace sme

namespace sme2 {

kleidicv_error_t yuv420p_to_rgb_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);
}  // namespace sme2

}  // namespace kleidicv

#endif  // KLEIDICV_CONVERSIONS_YUV_TO_RGB_H
