#!/bin/bash

PROJECT_DIR=$(cd "$(dirname "$0")" && pwd)
PARENT_DIR=$(dirname "$PROJECT_DIR")
BUILD_DIR="$PARENT_DIR/build"
INSTALL_DIR="$PARENT_DIR/install"

mkdir -p "$BUILD_DIR"
mkdir -p "$INSTALL_DIR"

cmake -S "$PROJECT_DIR/llvm" -B "$BUILD_DIR" -G "Unix Makefiles" \
  -DLLVM_ENABLE_PROJECTS="clang;lld" \
  -DLLVM_TARGETS_TO_BUILD="RISCV" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
  -DCMAKE_BUILD_TYPE=release

cmake --build "$BUILD_DIR" --target install -j16

