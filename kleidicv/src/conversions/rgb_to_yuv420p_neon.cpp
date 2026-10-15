// SPDX-FileCopyrightText: 2025 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/conversions/rgb_to_yuv.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/neon.h"
#include "rgb_to_yuv420_neon.h"

namespace kleidicv::neon {

#define KLEIDICV_RGB_TO_YUV420P_CASE(FORMAT)                                   \
  case FORMAT:                                                                 \
    return rgb_to_yuv420_for_format<FORMAT>(src, src_stride, dst, dst_stride,  \
                                            uv_dst, dst_stride, width, height, \
                                            begin, end);

KLEIDICV_TARGET_FN_ATTRS
kleidicv_error_t rgb_to_yuv420p_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t width, size_t height, kleidicv_color_conversion_t color_format,
    size_t begin, size_t end) {
  uint8_t *uv_dst = dst + dst_stride * height;
  switch (color_format) {
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_BGR_TO_YV12)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_RGB_TO_YV12)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_BGRA_TO_YV12)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_RGBA_TO_YV12)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_BGR_TO_IYUV)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_RGB_TO_IYUV)
    KLEIDICV_RGB_TO_YUV420P_CASE(KLEIDICV_BGRA_TO_IYUV)
    default:  // KLEIDICV_RGBA_TO_IYUV, validated by the caller.
      return rgb_to_yuv420_for_format<KLEIDICV_RGBA_TO_IYUV>(
          src, src_stride, dst, dst_stride, uv_dst, dst_stride, width, height,
          begin, end);
  }
}

#undef KLEIDICV_RGB_TO_YUV420P_CASE

}  // namespace kleidicv::neon
