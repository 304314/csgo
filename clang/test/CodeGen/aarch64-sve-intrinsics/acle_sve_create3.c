// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -target-feature +sve -S -disable-O0-optnone -Werror -Wall -emit-llvm -o - %s | opt -S -passes=mem2reg,tailcallelim | FileCheck %s
// RUN: %clang_cc1 -triple aarch64-none-linux-gnu -target-feature +sve -S -disable-O0-optnone -Werror -Wall -emit-llvm -o - -x c++ %s | opt -S -passes=mem2reg,tailcallelim | FileCheck %s -check-prefix=CPP-CHECK
// RUN: %clang_cc1 -DSVE_OVERLOADED_FORMS -triple aarch64-none-linux-gnu -target-feature +sve -S -disable-O0-optnone -Werror -Wall -emit-llvm -o - %s | opt -S -passes=mem2reg,tailcallelim | FileCheck %s
// RUN: %clang_cc1 -DSVE_OVERLOADED_FORMS -triple aarch64-none-linux-gnu -target-feature +sve -S -disable-O0-optnone -Werror -Wall -emit-llvm -o - -x c++ %s | opt -S -passes=mem2reg,tailcallelim | FileCheck %s -check-prefix=CPP-CHECK
// RUN: %clang_cc1 -DTEST_SME -triple aarch64-none-linux-gnu -target-feature +sve -target-feature +sme -S -disable-O0-optnone -Werror -Wall -o /dev/null %s

// REQUIRES: aarch64-registered-target

#include <arm_sve.h>

#ifdef SVE_OVERLOADED_FORMS
// A simple used,unused... macro, long enough to represent any SVE builtin.
#define SVE_ACLE_FUNC(A1,A2_UNUSED,A3,A4_UNUSED) A1##A3
#else
#define SVE_ACLE_FUNC(A1,A2,A3,A4) A1##A2##A3##A4
#endif

#ifndef TEST_SME
#define ATTR
#else
#define ATTR __arm_streaming
#endif

// CHECK-LABEL: @test_svcreate3_s8(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> poison, <vscale x 16 x i8> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP0]], <vscale x 16 x i8> [[X1:%.*]], i64 16)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP1]], <vscale x 16 x i8> [[X2:%.*]], i64 32)
// CHECK-NEXT:    ret <vscale x 48 x i8> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z17test_svcreate3_s8u10__SVInt8_tu10__SVInt8_tu10__SVInt8_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> poison, <vscale x 16 x i8> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP0]], <vscale x 16 x i8> [[X1:%.*]], i64 16)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP1]], <vscale x 16 x i8> [[X2:%.*]], i64 32)
// CPP-CHECK-NEXT:    ret <vscale x 48 x i8> [[TMP2]]
//
svint8x3_t test_svcreate3_s8(svint8_t x0, svint8_t x1, svint8_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_s8,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_s16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> poison, <vscale x 8 x i16> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP0]], <vscale x 8 x i16> [[X1:%.*]], i64 8)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP1]], <vscale x 8 x i16> [[X2:%.*]], i64 16)
// CHECK-NEXT:    ret <vscale x 24 x i16> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_s16u11__SVInt16_tu11__SVInt16_tu11__SVInt16_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> poison, <vscale x 8 x i16> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP0]], <vscale x 8 x i16> [[X1:%.*]], i64 8)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP1]], <vscale x 8 x i16> [[X2:%.*]], i64 16)
// CPP-CHECK-NEXT:    ret <vscale x 24 x i16> [[TMP2]]
//
svint16x3_t test_svcreate3_s16(svint16_t x0, svint16_t x1, svint16_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_s16,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_s32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> poison, <vscale x 4 x i32> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP0]], <vscale x 4 x i32> [[X1:%.*]], i64 4)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP1]], <vscale x 4 x i32> [[X2:%.*]], i64 8)
// CHECK-NEXT:    ret <vscale x 12 x i32> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_s32u11__SVInt32_tu11__SVInt32_tu11__SVInt32_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> poison, <vscale x 4 x i32> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP0]], <vscale x 4 x i32> [[X1:%.*]], i64 4)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP1]], <vscale x 4 x i32> [[X2:%.*]], i64 8)
// CPP-CHECK-NEXT:    ret <vscale x 12 x i32> [[TMP2]]
//
svint32x3_t test_svcreate3_s32(svint32_t x0, svint32_t x1, svint32_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_s32,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_s64(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> poison, <vscale x 2 x i64> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP0]], <vscale x 2 x i64> [[X1:%.*]], i64 2)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP1]], <vscale x 2 x i64> [[X2:%.*]], i64 4)
// CHECK-NEXT:    ret <vscale x 6 x i64> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_s64u11__SVInt64_tu11__SVInt64_tu11__SVInt64_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> poison, <vscale x 2 x i64> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP0]], <vscale x 2 x i64> [[X1:%.*]], i64 2)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP1]], <vscale x 2 x i64> [[X2:%.*]], i64 4)
// CPP-CHECK-NEXT:    ret <vscale x 6 x i64> [[TMP2]]
//
svint64x3_t test_svcreate3_s64(svint64_t x0, svint64_t x1, svint64_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_s64,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_u8(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> poison, <vscale x 16 x i8> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP0]], <vscale x 16 x i8> [[X1:%.*]], i64 16)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP1]], <vscale x 16 x i8> [[X2:%.*]], i64 32)
// CHECK-NEXT:    ret <vscale x 48 x i8> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z17test_svcreate3_u8u11__SVUint8_tu11__SVUint8_tu11__SVUint8_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> poison, <vscale x 16 x i8> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP0]], <vscale x 16 x i8> [[X1:%.*]], i64 16)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 48 x i8> @llvm.vector.insert.nxv48i8.nxv16i8(<vscale x 48 x i8> [[TMP1]], <vscale x 16 x i8> [[X2:%.*]], i64 32)
// CPP-CHECK-NEXT:    ret <vscale x 48 x i8> [[TMP2]]
//
svuint8x3_t test_svcreate3_u8(svuint8_t x0, svuint8_t x1, svuint8_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_u8,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_u16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> poison, <vscale x 8 x i16> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP0]], <vscale x 8 x i16> [[X1:%.*]], i64 8)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP1]], <vscale x 8 x i16> [[X2:%.*]], i64 16)
// CHECK-NEXT:    ret <vscale x 24 x i16> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_u16u12__SVUint16_tu12__SVUint16_tu12__SVUint16_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> poison, <vscale x 8 x i16> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP0]], <vscale x 8 x i16> [[X1:%.*]], i64 8)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x i16> @llvm.vector.insert.nxv24i16.nxv8i16(<vscale x 24 x i16> [[TMP1]], <vscale x 8 x i16> [[X2:%.*]], i64 16)
// CPP-CHECK-NEXT:    ret <vscale x 24 x i16> [[TMP2]]
//
svuint16x3_t test_svcreate3_u16(svuint16_t x0, svuint16_t x1, svuint16_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_u16,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_u32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> poison, <vscale x 4 x i32> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP0]], <vscale x 4 x i32> [[X1:%.*]], i64 4)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP1]], <vscale x 4 x i32> [[X2:%.*]], i64 8)
// CHECK-NEXT:    ret <vscale x 12 x i32> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_u32u12__SVUint32_tu12__SVUint32_tu12__SVUint32_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> poison, <vscale x 4 x i32> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP0]], <vscale x 4 x i32> [[X1:%.*]], i64 4)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x i32> @llvm.vector.insert.nxv12i32.nxv4i32(<vscale x 12 x i32> [[TMP1]], <vscale x 4 x i32> [[X2:%.*]], i64 8)
// CPP-CHECK-NEXT:    ret <vscale x 12 x i32> [[TMP2]]
//
svuint32x3_t test_svcreate3_u32(svuint32_t x0, svuint32_t x1, svuint32_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_u32,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_u64(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> poison, <vscale x 2 x i64> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP0]], <vscale x 2 x i64> [[X1:%.*]], i64 2)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP1]], <vscale x 2 x i64> [[X2:%.*]], i64 4)
// CHECK-NEXT:    ret <vscale x 6 x i64> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_u64u12__SVUint64_tu12__SVUint64_tu12__SVUint64_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> poison, <vscale x 2 x i64> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP0]], <vscale x 2 x i64> [[X1:%.*]], i64 2)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x i64> @llvm.vector.insert.nxv6i64.nxv2i64(<vscale x 6 x i64> [[TMP1]], <vscale x 2 x i64> [[X2:%.*]], i64 4)
// CPP-CHECK-NEXT:    ret <vscale x 6 x i64> [[TMP2]]
//
svuint64x3_t test_svcreate3_u64(svuint64_t x0, svuint64_t x1, svuint64_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_u64,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_f16(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> poison, <vscale x 8 x half> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> [[TMP0]], <vscale x 8 x half> [[X1:%.*]], i64 8)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> [[TMP1]], <vscale x 8 x half> [[X2:%.*]], i64 16)
// CHECK-NEXT:    ret <vscale x 24 x half> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_f16u13__SVFloat16_tu13__SVFloat16_tu13__SVFloat16_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> poison, <vscale x 8 x half> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> [[TMP0]], <vscale x 8 x half> [[X1:%.*]], i64 8)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 24 x half> @llvm.vector.insert.nxv24f16.nxv8f16(<vscale x 24 x half> [[TMP1]], <vscale x 8 x half> [[X2:%.*]], i64 16)
// CPP-CHECK-NEXT:    ret <vscale x 24 x half> [[TMP2]]
//
svfloat16x3_t test_svcreate3_f16(svfloat16_t x0, svfloat16_t x1, svfloat16_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_f16,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_f32(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> poison, <vscale x 4 x float> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> [[TMP0]], <vscale x 4 x float> [[X1:%.*]], i64 4)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> [[TMP1]], <vscale x 4 x float> [[X2:%.*]], i64 8)
// CHECK-NEXT:    ret <vscale x 12 x float> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_f32u13__SVFloat32_tu13__SVFloat32_tu13__SVFloat32_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> poison, <vscale x 4 x float> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> [[TMP0]], <vscale x 4 x float> [[X1:%.*]], i64 4)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 12 x float> @llvm.vector.insert.nxv12f32.nxv4f32(<vscale x 12 x float> [[TMP1]], <vscale x 4 x float> [[X2:%.*]], i64 8)
// CPP-CHECK-NEXT:    ret <vscale x 12 x float> [[TMP2]]
//
svfloat32x3_t test_svcreate3_f32(svfloat32_t x0, svfloat32_t x1, svfloat32_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_f32,,)(x0, x1, x2);
}

// CHECK-LABEL: @test_svcreate3_f64(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> poison, <vscale x 2 x double> [[X0:%.*]], i64 0)
// CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> [[TMP0]], <vscale x 2 x double> [[X1:%.*]], i64 2)
// CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> [[TMP1]], <vscale x 2 x double> [[X2:%.*]], i64 4)
// CHECK-NEXT:    ret <vscale x 6 x double> [[TMP2]]
//
// CPP-CHECK-LABEL: @_Z18test_svcreate3_f64u13__SVFloat64_tu13__SVFloat64_tu13__SVFloat64_t(
// CPP-CHECK-NEXT:  entry:
// CPP-CHECK-NEXT:    [[TMP0:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> poison, <vscale x 2 x double> [[X0:%.*]], i64 0)
// CPP-CHECK-NEXT:    [[TMP1:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> [[TMP0]], <vscale x 2 x double> [[X1:%.*]], i64 2)
// CPP-CHECK-NEXT:    [[TMP2:%.*]] = tail call <vscale x 6 x double> @llvm.vector.insert.nxv6f64.nxv2f64(<vscale x 6 x double> [[TMP1]], <vscale x 2 x double> [[X2:%.*]], i64 4)
// CPP-CHECK-NEXT:    ret <vscale x 6 x double> [[TMP2]]
//
svfloat64x3_t test_svcreate3_f64(svfloat64_t x0, svfloat64_t x1, svfloat64_t x2) ATTR
{
  return SVE_ACLE_FUNC(svcreate3,_f64,,)(x0, x1, x2);
}
