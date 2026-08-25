// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_RGB_TO_YUV_H
#define KLEIDICV_CONVERSIONS_RGB_TO_YUV_H

#include "kleidicv/conversions/yuv_common.h"
#include "kleidicv/kleidicv.h"

extern "C" {
// For internal use only. See instead kleidicv_rgb_to_yuv_semiplanar_u8.
// Converts a stripe (range of rows) of an interleaved RGB-family image
// (RGB, RGBA, BGR, or BGRA) to semi-planar YUV420 (nv12 or nv21).
// All channels are 8-bit wide. [begin, end) indexes chroma-plane rows.
// Handle empty images and validate the complete image before invoking this
// worker.
KLEIDICV_API_DECLARATION(kleidicv_rgb_to_yuv420sp_stripe_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *y_dst, size_t y_stride,
                         uint8_t *uv_dst, size_t uv_stride, size_t width,
                         size_t height,
                         kleidicv_color_conversion_t color_format, size_t begin,
                         size_t end);

// For internal use only. See instead kleidicv_rgb_to_yuv_u8.
// Converts a stripe (range of rows) of an interleaved RGB-family image
// (RGB, RGBA, BGR, or BGRA) to planar YUV420 (I420 or YV12).
// All channels are 8-bit wide. [begin, end) indexes chroma-plane rows.
// Handle empty images and validate the complete image before invoking this
// worker.
KLEIDICV_API_DECLARATION(kleidicv_rgb_to_yuv420p_stripe_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format, size_t begin,
                         size_t end);

// For internal use only. See instead kleidicv_rgb_to_yuv_u8.
// Converts an interleaved RGB-family image (RGB, RGBA, BGR, or BGRA)
// to an interleaved YUV444 image. All channels are 8-bit wide.
// Handle empty images and validate the complete image before invoking this
// worker.
KLEIDICV_API_DECLARATION(kleidicv_rgb_to_yuv444_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);

// For internal use only. See instead kleidicv_rgb_to_yuv_u8.
// Converts an interleaved RGB-family image (RGB, RGBA, BGR, or BGRA)
// to an interleaved YUV422 image. All channels are 8-bit wide.
// Handle empty images and validate the complete image before invoking this
// worker.
KLEIDICV_API_DECLARATION(kleidicv_rgb_to_yuv422_u8, const uint8_t *src,
                         size_t src_stride, uint8_t *dst, size_t dst_stride,
                         size_t width, size_t height,
                         kleidicv_color_conversion_t color_format);
}

namespace KLEIDICV_TARGET_NAMESPACE {

inline kleidicv_error_t rgb_to_yuv444_validate(
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

inline kleidicv_error_t rgb_to_yuv422_validate(
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

inline kleidicv_error_t rgb_to_yuv420p_validate(
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

inline kleidicv_error_t rgb_to_yuv420sp_validate(
    const uint8_t *src, uint8_t *y_dst, uint8_t *uv_dst, size_t width,
    size_t height,
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
  if (!is_yuv_format_supported<KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP>(
          color_format)) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  CHECK_POINTERS(src, y_dst, uv_dst);
  CHECK_IMAGE_SIZE(width, height);
  return KLEIDICV_OK;
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

namespace kleidicv {

namespace neon {

kleidicv_error_t rgb_to_yuv444_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t rgb_to_yuv420p_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv420sp_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format, size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv422_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);
}  // namespace neon

namespace sve2 {

kleidicv_error_t rgb_to_yuv444_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t rgb_to_yuv420p_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv420sp_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format, size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv422_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);
}  // namespace sve2

namespace sme {

kleidicv_error_t rgb_to_yuv444_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t rgb_to_yuv420p_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv420sp_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format, size_t begin, size_t end);

kleidicv_error_t rgb_to_yuv422_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);
}  // namespace sme

namespace sme2 {

kleidicv_error_t rgb_to_yuv444_u8(const uint8_t *src, size_t src_stride,
                                  uint8_t *dst, size_t dst_stride, size_t width,
                                  size_t height,
                                  kleidicv_color_conversion_t color_format);

kleidicv_error_t rgb_to_yuv420p_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end);
}  // namespace sme2

}  // namespace kleidicv

#endif  // KLEIDICV_CONVERSIONS_RGB_TO_YUV_H
