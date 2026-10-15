// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_CONVERSIONS_YUV_COMMON_H
#define KLEIDICV_CONVERSIONS_YUV_COMMON_H

#include "kleidicv/operation_validation.h"
#include "kleidicv/utils.h"

// Format validation and YUV layout helpers shared by both conversion
// directions.
namespace KLEIDICV_TARGET_NAMESPACE {

// Compile-time layout properties shared by RGB-to-YUV and YUV-to-RGB backends.
template <kleidicv_color_conversion_t kFormat>
struct YUVFormatTraits {
  static constexpr bool kBGR =
      (kFormat & KLEIDICV_COLOR_CONVERSION_FLAG_BGR) != 0;
  static constexpr bool kAlpha =
      (kFormat & KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA) != 0;
  static constexpr bool kVFirst =
      (kFormat & KLEIDICV_COLOR_CONVERSION_FLAG_VU) != 0;
  static constexpr bool kInterleave =
      (kFormat & KLEIDICV_COLOR_CONVERSION_YUV_FMT_MASK) ==
      KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP;
  static constexpr size_t kBlueIndex = kBGR ? 0 : 2;
  static constexpr size_t kChannels = kAlpha ? 4 : 3;

  // Packed YUV422 byte indexes. U is at byte 0 in UYVY, byte 1 in YUYV,
  // and byte 3 in YVYU.
  static constexpr bool kChromaFirst =
      (kFormat & KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST) != 0;
  static constexpr size_t kLumaFirstUIndex = kVFirst ? 3 : 1;
  static constexpr size_t kUIndex = kChromaFirst ? 0 : kLumaFirstUIndex;
  static constexpr size_t kYIndex = kChromaFirst ? 1 : 0;
};

template <kleidicv_color_conversion_t kBaseFormat>
struct YUVAllowedFlags;

template <>
struct YUVAllowedFlags<KLEIDICV_COLOR_CONVERSION_FMT_YUV444> {
  static constexpr size_t kValue =
      KLEIDICV_COLOR_CONVERSION_FLAG_BGR | KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA;
};

template <>
struct YUVAllowedFlags<KLEIDICV_COLOR_CONVERSION_FMT_YUV420SP> {
  static constexpr size_t kValue = KLEIDICV_COLOR_CONVERSION_FLAG_BGR |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_VU;
};

template <>
struct YUVAllowedFlags<KLEIDICV_COLOR_CONVERSION_FMT_YUV420P> {
  static constexpr size_t kValue = KLEIDICV_COLOR_CONVERSION_FLAG_BGR |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_VU;
};

template <>
struct YUVAllowedFlags<KLEIDICV_COLOR_CONVERSION_FMT_YUV422> {
  static constexpr size_t kValue = KLEIDICV_COLOR_CONVERSION_FLAG_BGR |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_ALPHA |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_VU |
                                   KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST;
};

template <kleidicv_color_conversion_t kBaseFormat>
bool is_yuv_format_supported(kleidicv_color_conversion_t color_format)
    KLEIDICV_STREAMING {
  constexpr size_t kAllowedFlags = YUVAllowedFlags<kBaseFormat>::kValue;
  const size_t format = static_cast<size_t>(color_format);
  // Check the base and reject every bit outside this family's modifiers.
  if ((format & ~kAllowedFlags) != kBaseFormat) {
    return false;
  }
  if constexpr (kBaseFormat == KLEIDICV_COLOR_CONVERSION_FMT_YUV422) {
    constexpr size_t kUnsupportedFlags =
        KLEIDICV_COLOR_CONVERSION_FLAG_VU |
        KLEIDICV_COLOR_CONVERSION_FLAG_CHROMA_FIRST;
    // VYUY is not implemented; YUYV, YVYU and UYVY are supported.
    if ((format & kUnsupportedFlags) == kUnsupportedFlags) {
      return false;
    }
  }
  return true;
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

#endif  // KLEIDICV_CONVERSIONS_YUV_COMMON_H
