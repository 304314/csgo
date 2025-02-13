// REQUIRES: build_for_openeuler
// RUN: %clang_cc1 -verify -fsyntax-only %s

__attribute__((optimize(0))) // expected-no-error
void f1() {}

int a = 0;
__attribute__((optimize(a))) // expected-error {{'optimize' attribute requires a string or an integer constant}}
void f2() {}

__attribute__((optimize(1))) // expected-warning {{invalid optimization level '1' specified; only 0 and 'O0' are supported; attribute ignored}}
void f3() {}

__attribute__((optimize(c))) // expected-error {{use of undeclared identifier 'c'}}
void f4() {}

__attribute__((optimize("O0"))) // expected-no-error
void f5() {}

__attribute__((optimize("O0", "O1"))) // expected-error {{'optimize' attribute takes one argument}}
void f6() {}

__attribute__((optimize("O1"))) // expected-warning {{invalid optimization level 'O1' specified; only 0 and 'O0' are supported; attribute ignored}}
void f7() {}
