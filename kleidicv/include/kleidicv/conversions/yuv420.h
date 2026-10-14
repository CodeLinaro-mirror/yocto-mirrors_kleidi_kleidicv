// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_YUV420_H
#define KLEIDICV_CONVERSIONS_YUV420_H

#include "kleidicv/utils.h"

namespace KLEIDICV_TARGET_NAMESPACE {

inline bool is_yuv420p_format_supported(
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
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
    kleidicv_color_conversion_t color_format) KLEIDICV_STREAMING {
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

inline constexpr size_t yuv420_chroma_height(size_t height) KLEIDICV_STREAMING {
  return height / 2 + height % 2;
}

// Converts a row index in a chroma plane to the corresponding row in the
// luma plane. Clamping before doubling keeps the final row representable when
// height is SIZE_MAX.
inline constexpr size_t yuv420_luma_row(size_t chroma_row,
                                        size_t height) KLEIDICV_STREAMING {
  if (chroma_row >= yuv420_chroma_height(height)) {
    return height;
  }
  return chroma_row * 2;
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

#endif  // KLEIDICV_CONVERSIONS_YUV420_H
