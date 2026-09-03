// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_YUV420_H
#define KLEIDICV_CONVERSIONS_YUV420_H

#include "kleidicv/utils.h"

namespace kleidicv {

inline bool is_yuv420p_format_supported(
    kleidicv_color_conversion_t color_format) {
  switch (color_format) {
    case KLEIDICV_IYUV_TO_RGB:
    case KLEIDICV_IYUV_TO_BGR:
    case KLEIDICV_IYUV_TO_RGBA:
    case KLEIDICV_IYUV_TO_BGRA:
    case KLEIDICV_YV12_TO_RGB:
    case KLEIDICV_YV12_TO_BGR:
    case KLEIDICV_YV12_TO_RGBA:
    case KLEIDICV_YV12_TO_BGRA:
      return true;
    default:
      return false;
  }
}

inline bool is_yuv420sp_format_supported(
    kleidicv_color_conversion_t color_format) {
  switch (color_format) {
    case KLEIDICV_NV12_TO_RGB:
    case KLEIDICV_NV12_TO_BGR:
    case KLEIDICV_NV12_TO_RGBA:
    case KLEIDICV_NV12_TO_BGRA:
    case KLEIDICV_NV21_TO_RGB:
    case KLEIDICV_NV21_TO_BGR:
    case KLEIDICV_NV21_TO_RGBA:
    case KLEIDICV_NV21_TO_BGRA:
      return true;
    default:
      return false;
  }
}

}  // namespace kleidicv

#endif  // KLEIDICV_CONVERSIONS_YUV420_H
