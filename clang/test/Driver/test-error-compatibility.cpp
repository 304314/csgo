// REQUIRES: build_for_openeuler
// RUN:   %clang -fgcc-compatible -c %s 2>&1 | FileCheck -check-prefix=CHECK-error %s
// RUN: not %clang -c %s 2>&1 | FileCheck -check-prefix=CHECK-error %s
// CHECK-error: explicit instantiation cannot have a storage class


template <typename T> static void func() {}
template static void func<int>(); // explicit instantiation

