#include <stdint.h>
#include <stdlib.h>

__attribute__((visibility("hidden"))) extern void
__sme_add_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_float4(float *dst, float *lhs, float *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_float8(double *dst, double *lhs, double *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_int1(signed char *dst, signed char *lhs, signed char *rhs,
               unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_int2(signed short *dst, signed short *lhs, signed short *rhs,
               unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_int4(signed int *dst, signed int *lhs, signed int *rhs, unsigned row,
               unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_int8(signed long long *dst, signed long long *lhs,
               signed long long *rhs, unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_add_uint8(unsigned long long *dst, unsigned long long *lhs,
                unsigned long long *rhs, unsigned row, unsigned column);

__attribute__((visibility("hidden"))) extern void
__sme_matmul_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned lhs_row,
                    unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_float4(float *dst, float *lhs, float *rhs, unsigned lhs_row,
                    unsigned lhs_column, unsigned rhs_column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_matmul_float8(double *dst, double *lhs, double *rhs, unsigned lhs_row,
                    unsigned lhs_column, unsigned rhs_column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_matmul_int1(signed char *dst, signed char *lhs, signed char *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_int2(signed short *dst, signed short *lhs, signed short *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_int4(signed int *dst, signed int *lhs, signed int *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_int8(int64_t *dst, int64_t *lhs, int64_t *rhs, unsigned lhs_row,
                  unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
                   unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_uint2(unsigned short *dst, unsigned short *lhs,
                   unsigned short *rhs, unsigned lhs_row, unsigned lhs_column,
                   unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
                   unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);
__attribute__((visibility("hidden"))) extern void
__sme_matmul_uint8(uint64_t *dst, uint64_t *lhs, uint64_t *rhs,
                   unsigned lhs_row, unsigned lhs_column, unsigned rhs_column);

__attribute__((visibility("hidden"))) extern void
__sme_sub_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_float4(float *dst, float *lhs, float *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_float8(double *dst, double *lhs, double *rhs, unsigned row,
                 unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_int1(signed char *dst, signed char *lhs, signed char *rhs,
               unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_int2(signed short *dst, signed short *lhs, signed short *rhs,
               unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_int4(signed int *dst, signed int *lhs, signed int *rhs, unsigned row,
               unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_int8(signed long long *dst, signed long long *lhs,
               signed long long *rhs, unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
                unsigned row, unsigned column);
__attribute__((visibility("hidden"))) extern void
__sme_sub_uint8(unsigned long long *dst, unsigned long long *lhs,
                unsigned long long *rhs, unsigned row, unsigned column);

__attribute__((visibility("hidden"))) extern void
__sme_transpose_float2(__fp16 *dst, __fp16 *src, unsigned row,
                       unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_float4(float *dst, float *src, unsigned row,
                       unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_float8(double *dst, double *src, unsigned row,
                       unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_int1(signed char *dst, signed char *src, unsigned row,
                     unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_int2(short *dst, short *src, unsigned row,
                     unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_int4(int *dst, int *src, unsigned row,
                     unsigned column) __arm_streaming;
__attribute__((visibility("hidden"))) extern void
__sme_transpose_int8(long long *dst, long long *src, unsigned row,
                     unsigned column) __arm_streaming;

void add_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned row,
                unsigned column) {
  __sme_add_float2(dst, lhs, rhs, row, column);
}
void add_float4(float *dst, float *lhs, float *rhs, unsigned row,
                unsigned column) {
  __sme_add_float4(dst, lhs, rhs, row, column);
}
void add_float8(double *dst, double *lhs, double *rhs, unsigned row,
                unsigned column) {
  __sme_add_float8(dst, lhs, rhs, row, column);
}
void add_int1(signed char *dst, signed char *lhs, signed char *rhs,
              unsigned row, unsigned column) {
  __sme_add_int1(dst, lhs, rhs, row, column);
}
void add_int2(signed short *dst, signed short *lhs, signed short *rhs,
              unsigned row, unsigned column) {
  __sme_add_int2(dst, lhs, rhs, row, column);
}
void add_int4(signed int *dst, signed int *lhs, signed int *rhs, unsigned row,
              unsigned column) {
  __sme_add_int4(dst, lhs, rhs, row, column);
}
void add_int8(signed long long *dst, signed long long *lhs,
              signed long long *rhs, unsigned row, unsigned column) {
  __sme_add_int8(dst, lhs, rhs, row, column);
}
void add_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
               unsigned row, unsigned column) {
  __sme_add_uint1(dst, lhs, rhs, row, column);
}
void add_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
               unsigned row, unsigned column) {
  __sme_add_uint2(dst, lhs, rhs, row, column);
}
void add_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
               unsigned row, unsigned column) {
  __sme_add_uint4(dst, lhs, rhs, row, column);
}
void add_uint8(unsigned long long *dst, unsigned long long *lhs,
               unsigned long long *rhs, unsigned row, unsigned column) {
  __sme_add_uint8(dst, lhs, rhs, row, column);
}

void matmul_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned lhs_row,
                   unsigned lhs_column, unsigned rhs_column) {
  __fp16 *tmpdst = (__fp16 *)malloc(sizeof(__fp16) * lhs_row * rhs_column);
  __fp16 *tmplhs = (__fp16 *)malloc(sizeof(__fp16) * lhs_row * lhs_column);
  __fp16 *tmprhs = (__fp16 *)malloc(sizeof(__fp16) * lhs_column * rhs_column);
  __sme_transpose_float2(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_float2(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_float2(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_float2(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_float4(float *dst, float *lhs, float *rhs, unsigned lhs_row,
                   unsigned lhs_column, unsigned rhs_column) {
  float *tmpdst = (float *)malloc(sizeof(float) * lhs_row * rhs_column);
  float *tmplhs = (float *)malloc(sizeof(float) * lhs_row * lhs_column);
  float *tmprhs = (float *)malloc(sizeof(float) * lhs_column * rhs_column);
  __sme_transpose_float4(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_float4(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_float4(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_float4(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_float8(double *dst, double *lhs, double *rhs, unsigned lhs_row,
                   unsigned lhs_column, unsigned rhs_column) {
  double *tmpdst = (double *)malloc(sizeof(double) * lhs_row * rhs_column);
  double *tmplhs = (double *)malloc(sizeof(double) * lhs_row * lhs_column);
  double *tmprhs = (double *)malloc(sizeof(double) * lhs_column * rhs_column);
  __sme_transpose_float8(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_float8(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_float8(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_float8(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_int1(signed char *dst, signed char *lhs, signed char *rhs,
                 unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  signed char *tmpdst =
      (signed char *)malloc(sizeof(signed char) * lhs_row * rhs_column);
  signed char *tmplhs =
      (signed char *)malloc(sizeof(signed char) * lhs_row * lhs_column);
  signed char *tmprhs =
      (signed char *)malloc(sizeof(signed char) * lhs_column * rhs_column);
  __sme_transpose_int1(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int1(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_int1(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int1(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_int2(signed short *dst, signed short *lhs, signed short *rhs,
                 unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  signed short *tmpdst =
      (signed short *)malloc(sizeof(signed short) * lhs_row * rhs_column);
  signed short *tmplhs =
      (signed short *)malloc(sizeof(signed short) * lhs_row * lhs_column);
  signed short *tmprhs =
      (signed short *)malloc(sizeof(signed short) * lhs_column * rhs_column);
  __sme_transpose_int2(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int2(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_int2(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int2(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_int4(signed int *dst, signed int *lhs, signed int *rhs,
                 unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  signed int *tmpdst =
      (signed int *)malloc(sizeof(signed int) * lhs_row * rhs_column);
  signed int *tmplhs =
      (signed int *)malloc(sizeof(signed int) * lhs_row * lhs_column);
  signed int *tmprhs =
      (signed int *)malloc(sizeof(signed int) * lhs_column * rhs_column);
  __sme_transpose_int4(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int4(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_int4(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int4(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_int8(int64_t *dst, int64_t *lhs, int64_t *rhs, unsigned lhs_row,
                 unsigned lhs_column, unsigned rhs_column) {
  int64_t *tmpdst = (int64_t *)malloc(sizeof(int64_t) * lhs_row * rhs_column);
  int64_t *tmplhs = (int64_t *)malloc(sizeof(int64_t) * lhs_row * lhs_column);
  int64_t *tmprhs =
      (int64_t *)malloc(sizeof(int64_t) * lhs_column * rhs_column);
  __sme_transpose_int8(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int8(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_int8(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int8(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  unsigned char *tmpdst =
      (unsigned char *)malloc(sizeof(unsigned char) * lhs_row * rhs_column);
  unsigned char *tmplhs =
      (unsigned char *)malloc(sizeof(unsigned char) * lhs_row * lhs_column);
  unsigned char *tmprhs =
      (unsigned char *)malloc(sizeof(unsigned char) * lhs_column * rhs_column);
  __sme_transpose_int1(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int1(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_uint1(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int1(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  unsigned short *tmpdst =
      (unsigned short *)malloc(sizeof(unsigned short) * lhs_row * rhs_column);
  unsigned short *tmplhs =
      (unsigned short *)malloc(sizeof(unsigned short) * lhs_row * lhs_column);
  unsigned short *tmprhs = (unsigned short *)malloc(sizeof(unsigned short) *
                                                    lhs_column * rhs_column);
  __sme_transpose_int2(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int2(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_uint2(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int2(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
                  unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  unsigned int *tmpdst =
      (unsigned int *)malloc(sizeof(unsigned int) * lhs_row * rhs_column);
  unsigned int *tmplhs =
      (unsigned int *)malloc(sizeof(unsigned int) * lhs_row * lhs_column);
  unsigned int *tmprhs =
      (unsigned int *)malloc(sizeof(unsigned int) * lhs_column * rhs_column);
  __sme_transpose_int4(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int4(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_uint4(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int4(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}
void matmul_uint8(uint64_t *dst, uint64_t *lhs, uint64_t *rhs, unsigned lhs_row,
                  unsigned lhs_column, unsigned rhs_column) {
  uint64_t *tmpdst =
      (uint64_t *)malloc(sizeof(uint64_t) * lhs_row * rhs_column);
  uint64_t *tmplhs =
      (uint64_t *)malloc(sizeof(uint64_t) * lhs_row * lhs_column);
  uint64_t *tmprhs =
      (uint64_t *)malloc(sizeof(uint64_t) * lhs_column * rhs_column);
  __sme_transpose_int8(tmplhs, lhs, lhs_column, lhs_row);
  __sme_transpose_int8(tmprhs, rhs, rhs_column, lhs_column);
  __sme_matmul_uint8(tmpdst, tmplhs, tmprhs, lhs_row, lhs_column, rhs_column);
  __sme_transpose_int8(dst, tmpdst, rhs_column, lhs_row);
  free(tmpdst);
  free(tmplhs);
  free(tmprhs);
}

void sub_float2(__fp16 *dst, __fp16 *lhs, __fp16 *rhs, unsigned row,
                unsigned column) {
  __sme_sub_float2(dst, lhs, rhs, row, column);
}
void sub_float4(float *dst, float *lhs, float *rhs, unsigned row,
                unsigned column) {
  __sme_sub_float4(dst, lhs, rhs, row, column);
}
void sub_float8(double *dst, double *lhs, double *rhs, unsigned row,
                unsigned column) {
  __sme_sub_float8(dst, lhs, rhs, row, column);
}
void sub_int1(signed char *dst, signed char *lhs, signed char *rhs,
              unsigned row, unsigned column) {
  __sme_sub_int1(dst, lhs, rhs, row, column);
}
void sub_int2(signed short *dst, signed short *lhs, signed short *rhs,
              unsigned row, unsigned column) {
  __sme_sub_int2(dst, lhs, rhs, row, column);
}
void sub_int4(signed int *dst, signed int *lhs, signed int *rhs, unsigned row,
              unsigned column) {
  __sme_sub_int4(dst, lhs, rhs, row, column);
}
void sub_int8(signed long long *dst, signed long long *lhs,
              signed long long *rhs, unsigned row, unsigned column) {
  __sme_sub_int8(dst, lhs, rhs, row, column);
}
void sub_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
               unsigned row, unsigned column) {
  __sme_sub_uint1(dst, lhs, rhs, row, column);
}
void sub_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
               unsigned row, unsigned column) {
  __sme_sub_uint2(dst, lhs, rhs, row, column);
}
void sub_uint4(unsigned int *dst, unsigned int *lhs, unsigned int *rhs,
               unsigned row, unsigned column) {
  __sme_sub_uint4(dst, lhs, rhs, row, column);
}
void sub_uint8(unsigned long long *dst, unsigned long long *lhs,
               unsigned long long *rhs, unsigned row, unsigned column) {
  __sme_sub_uint8(dst, lhs, rhs, row, column);
}

void transpose_float2(__fp16 *dst, __fp16 *src, unsigned row, unsigned column) {
  __sme_transpose_float2(dst, src, column, row);
}
void transpose_float4(float *dst, float *src, unsigned row, unsigned column) {
  __sme_transpose_float4(dst, src, column, row);
}
void transpose_float8(double *dst, double *src, unsigned row, unsigned column) {
  __sme_transpose_float8(dst, src, column, row);
}
void transpose_int1(signed char *dst, signed char *src, unsigned row,
                    unsigned column) {
  __sme_transpose_int1(dst, src, column, row);
}
void transpose_int2(short *dst, short *src, unsigned row, unsigned column) {
  __sme_transpose_int2(dst, src, column, row);
}
void transpose_int4(int *dst, int *src, unsigned row, unsigned column) {
  __sme_transpose_int4(dst, src, column, row);
}
void transpose_int8(long long *dst, long long *src, unsigned row,
                    unsigned column) {
  __sme_transpose_int8(dst, src, column, row);
}
