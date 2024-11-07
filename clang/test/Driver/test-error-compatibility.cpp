// REQUIRES: build_for_openeuler
// RUN:	%clang -fgcc-compatible -c %s 2>&1 | FileCheck -check-prefix=CHECK-error %s
// RUN:	not %clang -c %s 2>&1 | FileCheck -check-prefix=CHECK-error %s
// CHECK-error:	first parameter of 'main' (argument count) must be of type 'int'
int main(char* argv[])
{
}
