// REQUIRES: build_for_openeuler
// RUN: %clang_cc1 -O2 -S -emit-llvm %s -o - | FileCheck %s --check-prefix=O2
// RUN: %clang_cc1 -O0 -S -emit-llvm %s -o - | FileCheck %s --check-prefix=O0

__attribute__((optimize("O0"))) void f1(void) {}
// O2: @f1{{.*}}[[ATTR_OPTNONE:#[0-9]+]]
// O0: @f1{{.*}}[[ATTR_OPTNONE:#[0-9]+]]

__attribute__((optimize(0))) void f2(void) {}
// O2: @f2{{.*}}[[ATTR_OPTNONE:#[0-9]+]]
// O0: @f2{{.*}}[[ATTR_OPTNONE:#[0-9]+]]

// O2: attributes [[ATTR_OPTNONE]] = { {{.*}}optnone{{.*}} }

// CHECK the O0 overrides the attribute
// O0: attributes [[ATTR_OPTNONE]] = { {{.*}}optnone{{.*}} }
