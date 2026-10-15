// SPDX-FileCopyrightText: 2025 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/conversions/rgb_to_yuv.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/neon.h"
#include "rgb_to_yuv420_neon.h"

namespace kleidicv::neon {

#define KLEIDICV_RGB_TO_YUV420SP_CASE(FORMAT)                                 \
  case FORMAT:                                                                \
    return rgb_to_yuv420_for_format<FORMAT>(src, src_stride, y_dst, y_stride, \
                                            uv_dst, uv_stride, width, height, \
                                            begin, end);

KLEIDICV_TARGET_FN_ATTRS
kleidicv_error_t rgb_to_yuv420sp_stripe_u8(
    const uint8_t *src, size_t src_stride, uint8_t *y_dst, size_t y_stride,
    uint8_t *uv_dst, size_t uv_stride, size_t width, size_t height,
    kleidicv_color_conversion_t color_format, size_t begin, size_t end) {
  switch (color_format) {
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_BGR_TO_NV21)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_RGB_TO_NV21)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_BGRA_TO_NV21)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_RGBA_TO_NV21)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_BGR_TO_NV12)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_RGB_TO_NV12)
    KLEIDICV_RGB_TO_YUV420SP_CASE(KLEIDICV_BGRA_TO_NV12)
    default:  // KLEIDICV_RGBA_TO_NV12, validated by the caller.
      return rgb_to_yuv420_for_format<KLEIDICV_RGBA_TO_NV12>(
          src, src_stride, y_dst, y_stride, uv_dst, uv_stride, width, height,
          begin, end);
  }
}

#undef KLEIDICV_RGB_TO_YUV420SP_CASE

}  // namespace kleidicv::neon
