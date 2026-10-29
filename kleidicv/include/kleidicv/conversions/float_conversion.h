// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_FLOAT_CONVERSION_H
#define KLEIDICV_FLOAT_CONVERSION_H

#include "kleidicv/kleidicv.h"

namespace kleidicv {
namespace neon {

kleidicv_error_t f32_to_s8(const float *src, size_t src_stride, int8_t *dst,
                           size_t dst_stride, size_t width, size_t height);

kleidicv_error_t f32_to_u8(const float *src, size_t src_stride, uint8_t *dst,
                           size_t dst_stride, size_t width, size_t height);

kleidicv_error_t s8_to_f32(const int8_t *src, size_t src_stride, float *dst,
                           size_t dst_stride, size_t width, size_t height);

kleidicv_error_t u8_to_f32(const uint8_t *src, size_t src_stride, float *dst,
                           size_t dst_stride, size_t width, size_t height);

}  // namespace neon

namespace sve2 {

template <typename InputType, typename OutputType>
kleidicv_error_t float_conversion(const InputType *src, size_t src_stride,
                                  OutputType *dst, size_t dst_stride,
                                  size_t width, size_t height);

}  // namespace sve2

namespace sme {

template <typename InputType, typename OutputType>
kleidicv_error_t float_conversion(const InputType *src, size_t src_stride,
                                  OutputType *dst, size_t dst_stride,
                                  size_t width, size_t height);

}  // namespace sme

namespace sme2 {

template <typename InputType, typename OutputType>
kleidicv_error_t float_conversion(const InputType *src, size_t src_stride,
                                  OutputType *dst, size_t dst_stride,
                                  size_t width, size_t height);

}  // namespace sme2

}  // namespace kleidicv

#endif  // KLEIDICV_FLOAT_CONVERSION_H
