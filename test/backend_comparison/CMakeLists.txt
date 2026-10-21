# SPDX-FileCopyrightText: 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
#
# SPDX-License-Identifier: Apache-2.0

# Compare backends directly for operations whose API tests allow an error
# tolerance instead of requiring bit-exact results. Operations with bit-exact
# API references are already checked against the same results on every backend.
file(GLOB kleidicv_backend_comparison_test_sources CONFIGURE_DEPENDS "*.h" "test_*.cpp")

list(APPEND kleidicv_backend_comparison_test_sources ${KLEIDICV_TEST_FRAMEWORK_SOURCES})

set_source_files_properties(
  ${kleidicv_backend_comparison_test_sources}
  PROPERTIES COMPILE_OPTIONS "${KLEIDICV_TEST_CXX_FLAGS}"
)

add_executable(
  kleidicv-backend-comparison-test
  ${kleidicv_backend_comparison_test_sources}
)

set_target_properties(
  kleidicv-backend-comparison-test
  PROPERTIES CXX_STANDARD 17
)

target_include_directories(
  kleidicv-backend-comparison-test
  PRIVATE ${KLEIDICV_INCLUDE_DIR}
  PRIVATE ${KLEIDICV_TEST_INCLUDE_DIR}
)

if (KLEIDICV_ALLOCATION_TESTS)
  target_link_options(
    kleidicv-backend-comparison-test
    PRIVATE -Wl,--wrap,malloc -Wl,--wrap,calloc
  )
endif()

target_link_libraries(
  kleidicv-backend-comparison-test
  kleidicv
  gtest
)
