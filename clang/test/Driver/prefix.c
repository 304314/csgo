// REQUIRES: build_for_openeuler

// RUN: %clang -v -fgcc-compatible -iwithprefix include %s
// RUN: %clang -v -fgcc-compatible -nostdinc -iwithprefix include %s
// RUN: %clang -v -fgcc-compatible %s

#include <stdarg.h>
int main(void) {
return 0;
}
