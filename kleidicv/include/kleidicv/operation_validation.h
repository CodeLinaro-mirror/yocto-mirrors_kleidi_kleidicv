// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_OPERATION_VALIDATION_H
#define KLEIDICV_OPERATION_VALIDATION_H

#include "kleidicv/utils.h"

namespace KLEIDICV_TARGET_NAMESPACE {

template <typename SrcType, typename DstType>
inline kleidicv_error_t unary_operation_validate(
    const SrcType *src, size_t src_stride, DstType *dst, size_t dst_stride,
    size_t width, size_t height) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

template <typename SrcType, typename DstType>
inline kleidicv_error_t binary_operation_validate(
    const SrcType *src_a, size_t src_a_stride, const SrcType *src_b,
    size_t src_b_stride, DstType *dst, size_t dst_stride, size_t width,
    size_t height) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src_a, src_a_stride, height);
  CHECK_POINTER_AND_STRIDE(src_b, src_b_stride, height);
  CHECK_POINTER_AND_STRIDE(dst, dst_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

#endif  // KLEIDICV_OPERATION_VALIDATION_H
