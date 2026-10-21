// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <arm_sme.h>

#include <climits>

#include "kleidicv/transform/rotate.h"
#include "kleidicv/utils.h"
#include "rotate_transpose_common_sme.h"

namespace kleidicv::sme {

template <size_t kPixelSize>
static inline kleidicv_error_t rotate_by_angle(
    const uint8_t *src, size_t src_stride, uint8_t *dst, size_t dst_stride,
    size_t src_width, size_t src_height,
    int normalized_angle) KLEIDICV_STREAMING {
  if (normalized_angle == 90) {
    return rotate_cw<kPixelSize>(src, src_stride, dst, dst_stride, src_width,
                                 src_height);
  }
  return rotate_ccw<kPixelSize>(src, src_stride, dst, dst_stride, src_width,
                                src_height);
}

KLEIDICV_LOCALLY_STREAMING KLEIDICV_TARGET_FN_ATTRS kleidicv_error_t
rotate(const void *src_void, size_t src_stride, size_t src_width,
       size_t src_height, void *dst_void, size_t dst_stride, int angle,
       size_t pixel_size) {
  if (kleidicv_error_t err =
          rotate_validate(src_void, src_stride, src_width, src_height, dst_void,
                          dst_stride, angle, pixel_size)) {
    return err;
  }
  const uint8_t *src = reinterpret_cast<const uint8_t *>(src_void);
  uint8_t *dst = reinterpret_cast<uint8_t *>(dst_void);
  const int normalized_angle = (angle == 270) ? -90 : angle;

  switch (pixel_size) {
    case 1:
      return rotate_by_angle<1>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    case 2:
      return rotate_by_angle<2>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    case 3:
      return rotate_by_angle<3>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    case 4:
      return rotate_by_angle<4>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    case 6:
      return rotate_by_angle<6>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    case 8:
      return rotate_by_angle<8>(src, src_stride, dst, dst_stride, src_width,
                                src_height, normalized_angle);
    default:
      return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
}

}  // namespace kleidicv::sme
