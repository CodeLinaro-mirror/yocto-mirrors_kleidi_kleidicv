// SPDX-FileCopyrightText: 2024 - 2026 Arm Limited and/or its affiliates <open-source-office@arm.com>
//
// SPDX-License-Identifier: Apache-2.0

#include "kleidicv/conversions/float_conversion.h"
#include "kleidicv/dispatch.h"
#include "kleidicv/kleidicv.h"
#include "kleidicv/types.h"

KLEIDICV_MULTIVERSION_C_API_WITH_SME(
    kleidicv_f32_to_s8, &kleidicv::neon::f32_to_s8,
    KLEIDICV_SVE2_IMPL_IF((&kleidicv::sve2::float_conversion<float, int8_t>)),
    (&kleidicv::sme::float_conversion<float, int8_t>),
    (&kleidicv::sme2::float_conversion<float, int8_t>));

KLEIDICV_MULTIVERSION_C_API_WITH_SME(
    kleidicv_f32_to_u8, &kleidicv::neon::f32_to_u8,
    KLEIDICV_SVE2_IMPL_IF((&kleidicv::sve2::float_conversion<float, uint8_t>)),
    (&kleidicv::sme::float_conversion<float, uint8_t>),
    (&kleidicv::sme2::float_conversion<float, uint8_t>));

KLEIDICV_MULTIVERSION_C_API_WITH_SME(
    kleidicv_s8_to_f32, &kleidicv::neon::s8_to_f32,
    (&kleidicv::sve2::float_conversion<int8_t, float>),
    (&kleidicv::sme::float_conversion<int8_t, float>),
    (&kleidicv::sme2::float_conversion<int8_t, float>));

KLEIDICV_MULTIVERSION_C_API_WITH_SME(
    kleidicv_u8_to_f32, &kleidicv::neon::u8_to_f32,
    (&kleidicv::sve2::float_conversion<uint8_t, float>),
    (&kleidicv::sme::float_conversion<uint8_t, float>),
    (&kleidicv::sme2::float_conversion<uint8_t, float>));
