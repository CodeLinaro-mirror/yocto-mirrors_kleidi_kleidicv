// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_TEST_FRAMEWORK_FLOAT_ENVIRONMENT_H_
#define KLEIDICV_TEST_FRAMEWORK_FLOAT_ENVIRONMENT_H_

#include <cstdint>

namespace test {

// Sets floating-point modes for a test and restores control and exception
// status on scope exit, including after a fatal assertion.
class FloatEnvironment final {
 public:
  FloatEnvironment(bool default_nan, bool flush_to_zero) {
    constexpr uint64_t kExceptionTraps =
        (uint64_t{0x1F} << 8) | (uint64_t{1} << 15);
    constexpr uint64_t kAlternateHandling = uint64_t{1} << 1;
    constexpr uint64_t kFlushInputsToZero = uint64_t{1};
    constexpr uint64_t kFlushToZero16 = uint64_t{1} << 19;
    constexpr uint64_t kDefaultNaN = uint64_t{1} << 25;
    constexpr uint64_t kFlushToZero = uint64_t{1} << 24;

    asm volatile("mrs %0, fpcr" : "=r"(saved_fpcr_));
    asm volatile("mrs %0, fpsr" : "=r"(saved_fpsr_));

    // Disable exception traps, alternate handling and input flushing.
    uint64_t control = saved_fpcr_ & ~(kExceptionTraps | kAlternateHandling |
                                       kFlushInputsToZero | kFlushToZero16 |
                                       kDefaultNaN | kFlushToZero);
    control |= default_nan ? kDefaultNaN : 0;
    control |= flush_to_zero ? kFlushToZero : 0;
    asm volatile("msr fpcr, %0" : : "r"(control) : "memory");
  }

  ~FloatEnvironment() {
    asm volatile("msr fpcr, %0" : : "r"(saved_fpcr_) : "memory");
    asm volatile("msr fpsr, %0" : : "r"(saved_fpsr_) : "memory");
  }

  FloatEnvironment(const FloatEnvironment&) = delete;
  FloatEnvironment& operator=(const FloatEnvironment&) = delete;

 private:
  uint64_t saved_fpcr_{};
  uint64_t saved_fpsr_{};
};  // end of class FloatEnvironment

}  // namespace test

#endif  // KLEIDICV_TEST_FRAMEWORK_FLOAT_ENVIRONMENT_H_
