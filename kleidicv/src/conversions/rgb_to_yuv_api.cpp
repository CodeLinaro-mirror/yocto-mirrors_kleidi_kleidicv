// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/conversions/rgb_to_yuv.h"
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

KLEIDICV_DEFINE_C_API_WITH_SME2(kleidicv_rgb_to_yuv444_u8, rgb_to_yuv444_u8);
KLEIDICV_DEFINE_C_API_WITH_SME2(kleidicv_rgb_to_yuv420p_stripe_u8,
                                rgb_to_yuv420p_stripe_u8);
KLEIDICV_DEFINE_C_API_WITHOUT_SME2(kleidicv_rgb_to_yuv422_u8, rgb_to_yuv422_u8);
KLEIDICV_DEFINE_C_API_WITHOUT_SME2(kleidicv_rgb_to_yuv420sp_stripe_u8,
                                   rgb_to_yuv420sp_stripe_u8);

namespace kleidicv {

template <bool kUseSME>
kleidicv_error_t rgb_to_yuv_u8(const uint8_t *src, size_t src_stride,
                               uint8_t *dst, size_t dst_stride, size_t width,
                               size_t height,
                               kleidicv_color_conversion_t color_format) {
  if (width == 0 || height == 0) {
    return KLEIDICV_OK;
  }

  const size_t base_format = static_cast<size_t>(
      color_format & KLEIDICV_COLOR_CONVERSION_YUV_FMT_MASK);
  switch (base_format) {
    case KLEIDICV_COLOR_CONVERSION_FMT_YUV444:
      if (kleidicv_error_t err = rgb_to_yuv444_validate(
              src, src_stride, dst, dst_stride, width, height, color_format)) {
        return err;
      }
      if constexpr (kUseSME) {
        return kleidicv_rgb_to_yuv444_u8_sme(src, src_stride, dst, dst_stride,
                                             width, height, color_format);
      } else {
        return kleidicv_rgb_to_yuv444_u8(src, src_stride, dst, dst_stride,
                                         width, height, color_format);
      }
    case KLEIDICV_COLOR_CONVERSION_FMT_YUV422:
      if (kleidicv_error_t err = rgb_to_yuv422_validate(
              src, src_stride, dst, dst_stride, width, height, color_format)) {
        return err;
      }
      if constexpr (kUseSME) {
        return kleidicv_rgb_to_yuv422_u8_sme(src, src_stride, dst, dst_stride,
                                             width, height, color_format);
      } else {
        return kleidicv_rgb_to_yuv422_u8(src, src_stride, dst, dst_stride,
                                         width, height, color_format);
      }
    case KLEIDICV_COLOR_CONVERSION_FMT_YUV420P:
      if (kleidicv_error_t err = rgb_to_yuv420p_validate(
              src, src_stride, dst, dst_stride, width, height, color_format)) {
        return err;
      }
      // Use the stripe worker for the full image as well as threaded work:
      // splitting by chroma rows keeps each shared 2x2 pixel block together.
      if constexpr (kUseSME) {
        return kleidicv_rgb_to_yuv420p_stripe_u8_sme(
            src, src_stride, dst, dst_stride, width, height, color_format, 0,
            yuv420_chroma_height(height));
      } else {
        return kleidicv_rgb_to_yuv420p_stripe_u8(
            src, src_stride, dst, dst_stride, width, height, color_format, 0,
            yuv420_chroma_height(height));
      }
    default:
      return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
}

template <auto &StripeFunction>
kleidicv_error_t rgb_to_yuv_semiplanar_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format) {
  if (width == 0 || height == 0) {
    return KLEIDICV_OK;
  }

  if (kleidicv_error_t err = rgb_to_yuv420sp_validate(src, y_dst, uv_dst, width,
                                                      height, color_format)) {
    return err;
  }
  // Adapt the whole image to the chroma-row range expected by the stripe
  // worker.
  return StripeFunction(src, src_stride, y_dst, y_stride, uv_dst, uv_stride,
                        width, height, color_format, 0,
                        yuv420_chroma_height(height));
}

}  // namespace kleidicv

extern "C" {

kleidicv_error_t kleidicv_rgb_to_yuv_semiplanar_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format) {
  return kleidicv::rgb_to_yuv_semiplanar_u8<kleidicv_rgb_to_yuv420sp_stripe_u8>(
      src, src_stride, y_dst, y_stride, uv_dst, uv_stride, width, height,
      color_format);
}

kleidicv_error_t kleidicv_rgb_to_yuv_semiplanar_u8_sme(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format) {
  return kleidicv::rgb_to_yuv_semiplanar_u8<
      kleidicv_rgb_to_yuv420sp_stripe_u8_sme>(src, src_stride, y_dst, y_stride,
                                              uv_dst, uv_stride, width, height,
                                              color_format);
}

kleidicv_error_t kleidicv_rgb_to_yuv_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  return kleidicv::rgb_to_yuv_u8<false>(src, src_stride, dst, dst_stride, width,
                                        height, color_format);
}

kleidicv_error_t kleidicv_rgb_to_yuv_u8_sme(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format) {
  return kleidicv::rgb_to_yuv_u8<true>(src, src_stride, dst, dst_stride, width,
                                       height, color_format);
}

}  // extern "C"
