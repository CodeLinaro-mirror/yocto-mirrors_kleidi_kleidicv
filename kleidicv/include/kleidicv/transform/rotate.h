// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_TRANSFORM_ROTATE_H
#define KLEIDICV_TRANSFORM_ROTATE_H

#include <cstddef>
#include <cstdint>

#include "kleidicv/ctypes.h"
#include "kleidicv/utils.h"

namespace KLEIDICV_TARGET_NAMESPACE {

// Check both pointer alignments before pointers, strides, and image size.
template <typename T>
kleidicv_error_t rotate_validate(const void *src, size_t src_stride, void *dst,
                                 size_t dst_stride, size_t width,
                                 size_t height) KLEIDICV_STREAMING {
  CHECK_ALIGNMENT(T, src);
  CHECK_ALIGNMENT(T, dst);
  CHECK_POINTER_AND_STRIDE(static_cast<const T *>(src), src_stride, height);
  CHECK_POINTER_AND_STRIDE(static_cast<T *>(dst), dst_stride, width);
  CHECK_IMAGE_SIZE(width, height);
  return KLEIDICV_OK;
}

inline kleidicv_error_t rotate_validate(const void *src, size_t src_stride,
                                        size_t width, size_t height, void *dst,
                                        size_t dst_stride, int angle,
                                        size_t pixel_size) KLEIDICV_STREAMING {
  if ((angle != 90 && angle != -90 && angle != 270) || src == dst) {
    return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
  switch (pixel_size) {
    case sizeof(uint8_t):
    case sizeof(uint8_t) * 3:
      return rotate_validate<uint8_t>(src, src_stride, dst, dst_stride, width,
                                      height);
    case sizeof(uint16_t):
    case sizeof(uint16_t) * 3:
      return rotate_validate<uint16_t>(src, src_stride, dst, dst_stride, width,
                                       height);
    case sizeof(uint32_t):
      return rotate_validate<uint32_t>(src, src_stride, dst, dst_stride, width,
                                       height);
    case sizeof(uint64_t):
      return rotate_validate<uint64_t>(src, src_stride, dst, dst_stride, width,
                                       height);
    default:
      return KLEIDICV_ERROR_NOT_IMPLEMENTED;
  }
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

namespace kleidicv {

namespace neon {

kleidicv_error_t rotate(const void *src, size_t src_stride, size_t width,
                        size_t height, void *dst, size_t dst_stride, int angle,
                        size_t pixel_size);
}  // namespace neon

namespace sme {
kleidicv_error_t rotate(const void *src, size_t src_stride, size_t width,
                        size_t height, void *dst, size_t dst_stride, int angle,
                        size_t pixel_size);
}  // namespace sme

namespace sme2 {
kleidicv_error_t rotate(const void *src, size_t src_stride, size_t width,
                        size_t height, void *dst, size_t dst_stride, int angle,
                        size_t pixel_size);
}  // namespace sme2

}  // namespace kleidicv

#endif  // KLEIDICV_TRANSFORM_ROTATE_H
