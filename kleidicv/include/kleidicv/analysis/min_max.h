// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_ANALYSIS_MIN_MAX_H
#define KLEIDICV_ANALYSIS_MIN_MAX_H

#include <limits>
#include <type_traits>

#include "kleidicv/ctypes.h"
#include "kleidicv/utils.h"

namespace KLEIDICV_TARGET_NAMESPACE {

// Start at the highest value so initialization cannot lower the minimum.
// For floating-point T, numeric_limits<T>::max() is the largest finite value,
// which is smaller than +infinity. If every input is +infinity, starting at
// that finite value repeatedly computes min(largest_finite, +infinity) and
// keeps largest_finite. The result would be wrong and absent from the input.
// Starting at +infinity lets smaller inputs replace it, while an input made
// entirely of +infinity correctly leaves the result at +infinity. This also
// makes unused vector lanes and batch slots neutral during the final merge.
// Integer types have no infinity, so their largest value is sufficient.
template <typename T>
constexpr T min_max_initial_minimum() KLEIDICV_STREAMING {
  if constexpr (std::numeric_limits<T>::has_infinity) {
    return std::numeric_limits<T>::infinity();
  } else {
    return std::numeric_limits<T>::max();
  }
}

// Start at the lowest value so initialization cannot raise the maximum.
// For floating-point T, numeric_limits<T>::lowest() is the lowest finite value,
// which is greater than -infinity. If every input is -infinity, starting at
// that finite value repeatedly computes max(lowest_finite, -infinity) and
// keeps lowest_finite. The result would be wrong and absent from the input.
// Starting at -infinity lets larger inputs replace it, while an input made
// entirely of -infinity correctly leaves the result at -infinity. This also
// makes unused vector lanes and batch slots neutral during the final merge.
// Integer types have no infinity, so their lowest value is sufficient.
template <typename T>
constexpr T min_max_initial_maximum() KLEIDICV_STREAMING {
  if constexpr (std::numeric_limits<T>::has_infinity) {
    return -std::numeric_limits<T>::infinity();
  } else {
    return std::numeric_limits<T>::lowest();
  }
}

// Replace any floating-point NaN result with one standard quiet NaN
// (0x7fc00000 for float32); leave other values and integer types unchanged.
// NaNs can carry different sign and payload bits. Hardware reductions can
// select different NaNs as vector length, row layout or thread partitioning
// changes. Standardizing the final NaN makes the output bits agree even when
// FPCR.DN (default NaN mode) is disabled.
// The check value != value detects NaN: NaN compares unequal to itself,
// whereas ordinary numbers and infinities compare equal to themselves.
template <typename T>
T min_max_canonicalize_nan(T value) KLEIDICV_STREAMING {
  if constexpr (std::is_floating_point_v<T>) {
    if (value != value) {
      return std::numeric_limits<T>::quiet_NaN();
    }
    return value;
  } else {
    return value;
  }
}

template <typename T>
inline kleidicv_error_t min_max_validate(const T *src, size_t src_stride,
                                         size_t width,
                                         size_t height) KLEIDICV_STREAMING {
  CHECK_POINTER_AND_STRIDE(src, src_stride, height);
  CHECK_IMAGE_SIZE(width, height);

  return KLEIDICV_OK;
}

}  // namespace KLEIDICV_TARGET_NAMESPACE

namespace kleidicv {

namespace neon {

template <typename T>
kleidicv_error_t min_max(const T *src, size_t src_stride, size_t width,
                         size_t height, T *min_value, T *max_value);

template <typename T>
kleidicv_error_t min_max_loc(const T *src, size_t src_stride, size_t width,
                             size_t height, size_t *min_offset,
                             size_t *max_offset);

}  // namespace neon

namespace sve2 {

template <typename T>
kleidicv_error_t min_max(const T *src, size_t src_stride, size_t width,
                         size_t height, T *min_value, T *max_value);
}  // namespace sve2

namespace sme {

template <typename T>
kleidicv_error_t min_max(const T *src, size_t src_stride, size_t width,
                         size_t height, T *min_value, T *max_value);
}  // namespace sme

namespace sme2 {

template <typename T>
kleidicv_error_t min_max(const T *src, size_t src_stride, size_t width,
                         size_t height, T *min_value, T *max_value);
}  // namespace sme2

}  // namespace kleidicv

#endif  // KLEIDICV_ANALYSIS_MIN_MAX_H
