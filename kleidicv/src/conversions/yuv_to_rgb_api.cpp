// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/conversions/yuv_to_rgb.h"
#include "kleidicv/dispatch.h"
#include "kleidicv/kleidicv.h"

#define KLEIDICV_DEFINE_C_API_WITH_SME2(name, partialname)              \
  KLEIDICV_MULTIVERSION_C_API_WITH_SME(                                 \
      name, &kleidicv::neon::partialname, &kleidicv::sve2::partialname, \
      &kleidicv::sme::partialname, &kleidicv::sme2::partialname)

#define KLEIDICV_DEFINE_C_API_WITHOUT_SME2(name, partialname)              \
  KLEIDICV_MULTIVERSION_C_API_WITH_SME(name, &kleidicv::neon::partialname, \
                                       &kleidicv::sve2::partialname,       \
                                       &kleidicv::sme::partialname, nullptr)

KLEIDICV_DEFINE_C_API_WITH_SME2(kleidicv_yuv420p_to_rgb_stripe_u8,
                                yuv420p_to_rgb_stripe_u8);
KLEIDICV_DEFINE_C_API_WITHOUT_SME2(kleidicv_yuv444_to_rgb_u8, yuv444_to_rgb_u8);
KLEIDICV_DEFINE_C_API_WITHOUT_SME2(kleidicv_yuv422_to_rgb_u8, yuv422_to_rgb_u8);
KLEIDICV_DEFINE_C_API_WITHOUT_SME2(kleidicv_yuv420sp_to_rgb_u8,
                                   yuv420sp_to_rgb_u8);

namespace kleidicv {

template <bool kUseSME>
kleidicv_error_t kleidicv_yuv_to_rgb_u8(
    const uint8_t* src, size_t src_stride, uint8_t* dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  // Extract the base format
  const size_t base_format = static_cast<size_t>(
      color_format & KLEIDICV_COLOR_CONVERSION_YUV_FMT_MASK);

  if (base_format == KLEIDICV_COLOR_CONVERSION_FMT_YUV444) {
    if constexpr (kUseSME) {
      return kleidicv_yuv444_to_rgb_u8_sme(src, src_stride, dst, dst_stride,
                                           width, height, color_format);
    }
    return kleidicv_yuv444_to_rgb_u8(src, src_stride, dst, dst_stride, width,
                                     height, color_format);
  }

  if (base_format == KLEIDICV_COLOR_CONVERSION_FMT_YUV420P) {
    if (kleidicv_error_t err = yuv420p_to_rgb_validate(
            src, src_stride, dst, dst_stride, width, height, color_format)) {
      return err;
    }

    // YUV420 relies on 2x2 chroma blocks shared between lines, so the stripe
    // API ensures each invocation receives contiguous rows to preserve that
    // neighborhood context and compute the correct per-plane offsets for the
    // current stripe.

    if constexpr (kUseSME) {
      return kleidicv_yuv420p_to_rgb_stripe_u8_sme(src, src_stride, dst,
                                                   dst_stride, width, height,
                                                   color_format, 0, height);
    }
    return kleidicv_yuv420p_to_rgb_stripe_u8(src, src_stride, dst, dst_stride,
                                             width, height, color_format, 0,
                                             height);
  }

  if (base_format == KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP) {
    if (kleidicv_error_t err = yuv420sp_to_rgb_validate(
            src, src_stride, dst, dst_stride, width, height, color_format)) {
      return err;
    }

    const uint8_t* src_uv = src + src_stride * height;
    if constexpr (kUseSME) {
      return kleidicv_yuv420sp_to_rgb_u8_sme(src, src_stride, src_uv,
                                             src_stride, dst, dst_stride, width,
                                             height, color_format);
    }
    return kleidicv_yuv420sp_to_rgb_u8(src, src_stride, src_uv, src_stride, dst,
                                       dst_stride, width, height, color_format);
  }

  if constexpr (kUseSME) {
    return kleidicv_yuv422_to_rgb_u8_sme(src, src_stride, dst, dst_stride,
                                         width, height, color_format);
  }

  return kleidicv_yuv422_to_rgb_u8(src, src_stride, dst, dst_stride, width,
                                   height, color_format);
}

}  // namespace kleidicv

extern "C" {

kleidicv_error_t kleidicv_yuv_semiplanar_to_rgb_u8(
    const uint8_t* src_y, size_t src_y_stride, const uint8_t* src_uv,
    size_t src_uv_stride, uint8_t* dst, size_t dst_stride, size_t width,
    size_t height, kleidicv_color_conversion_t color_format) {
  if (kleidicv_error_t err = kleidicv::yuv_semiplanar_to_rgb_validate(
          src_y, src_y_stride, src_uv, src_uv_stride, dst, dst_stride, width,
          height, color_format)) {
    return err;
  }
  return kleidicv_yuv420sp_to_rgb_u8(src_y, src_y_stride, src_uv, src_uv_stride,
                                     dst, dst_stride, width, height,
                                     color_format);
}

kleidicv_error_t kleidicv_yuv_semiplanar_to_rgb_u8_sme(
    const uint8_t* src_y, size_t src_y_stride, const uint8_t* src_uv,
    size_t src_uv_stride, uint8_t* dst, size_t dst_stride, size_t width,
    size_t height, kleidicv_color_conversion_t color_format) {
  if (kleidicv_error_t err = kleidicv::yuv_semiplanar_to_rgb_validate(
          src_y, src_y_stride, src_uv, src_uv_stride, dst, dst_stride, width,
          height, color_format)) {
    return err;
  }
  return kleidicv_yuv420sp_to_rgb_u8_sme(src_y, src_y_stride, src_uv,
                                         src_uv_stride, dst, dst_stride, width,
                                         height, color_format);
}

kleidicv_error_t kleidicv_yuv_to_rgb_u8(
    const uint8_t* src, size_t src_stride, uint8_t* dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  return kleidicv::kleidicv_yuv_to_rgb_u8<false>(
      src, src_stride, dst, dst_stride, width, height, color_format);
}

kleidicv_error_t kleidicv_yuv_to_rgb_u8_sme(
    const uint8_t* src, size_t src_stride, uint8_t* dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  return kleidicv::kleidicv_yuv_to_rgb_u8<true>(
      src, src_stride, dst, dst_stride, width, height, color_format);
}

}  // extern "C"
