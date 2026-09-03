// SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#ifndef KLEIDICV_TEST_API_THREAD_VALIDATION_H
#define KLEIDICV_TEST_API_THREAD_VALIDATION_H

#include <gtest/gtest.h>

#include <cstddef>

#include "kleidicv_thread/kleidicv_thread.h"

namespace test {

// Records scheduling without executing callbacks, so invalid-input tests do
// not need image-sized buffers.
class ThreadValidation : public testing::Test {
 protected:
  kleidicv_thread_multithreading scheduler() {
    parallel_call_count_ = 0;
    return {record_parallel, this};
  }

  void expect_rejected(kleidicv_error_t expected,
                       kleidicv_error_t actual) const {
    EXPECT_EQ(expected, actual);
    EXPECT_EQ(size_t{0}, parallel_call_count_);
  }

 private:
  static kleidicv_error_t record_parallel(kleidicv_thread_callback /*callback*/,
                                          void * /*callback_data*/,
                                          void *parallel_data,
                                          size_t /*task_count*/) {
    auto *test = static_cast<ThreadValidation *>(parallel_data);
    ++test->parallel_call_count_;
    return KLEIDICV_ERROR_CONTEXT_MISMATCH;
  }

  size_t parallel_call_count_ = 0;
};

}  // namespace test

#endif  // KLEIDICV_TEST_API_THREAD_VALIDATION_H
