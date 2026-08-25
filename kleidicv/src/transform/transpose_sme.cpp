// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include <arm_sme.h>

#include <climits>

#include "kleidicv/transform/transpose.h"
#include "kleidicv/utils.h"
#include "rotate_transpose_common_sme.h"

namespace kleidicv::sme {

KLEIDICV_LOCALLY_STREAMING KLEIDICV_TARGET_FN_ATTRS kleidicv_error_t
transpose(const void *src_void, size_t src_stride, void *dst_void,
          size_t dst_stride, size_t width, size_t height, size_t pixel_size) {
  if (kleidicv_error_t err =
          transpose_validate(src_void, src_stride, dst_void, dst_stride, width,
                             height, pixel_size)) {
    return err;
  }
  if (width == 0 || height == 0) {
    return KLEIDICV_OK;
  }

  const uint8_t *src = reinterpret_cast<const uint8_t *>(src_void);
  uint8_t *dst = reinterpret_cast<uint8_t *>(dst_void);

  switch (pixel_size) {
    case 1:
      return transpose<1>(src, src_stride, dst, dst_stride, width, height);
    case 2:
      return transpose<2>(src, src_stride, dst, dst_stride, width, height);
    case 3:
      return transpose<3>(src, src_stride, dst, dst_stride, width, height);
    case 4:
      return transpose<4>(src, src_stride, dst, dst_stride, width, height);
    case 6:
      return transpose<6>(src, src_stride, dst, dst_stride, width, height);
    case 8:
    default:  // transpose_validate rejects invalid pixel sizes
      return transpose<8>(src, src_stride, dst, dst_stride, width, height);
  }
}

}  // namespace kleidicv::sme
