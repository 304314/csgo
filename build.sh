#!/bin/bash

# Tools to use for bootstrapping.
C_COMPILER_PATH=gcc
CXX_COMPILER_PATH=g++

gcc_toolchain="$BISHENG_CROSS_TOOLCHAIN"
sysroot="$BISHENG_CROSS_SYSROOT"

# Initialize our own variables:
buildtype=RelWithDebInfo
backends="ARM;AArch64;X86"
enabled_projects="clang;lld;compiler-rt;openmp;clang-tools-extra"
embedded_toolchain="0"
split_dwarf=on
use_ccache="0"
do_install="0"
clean=0
unit_test=""
install="install"
install_toolchain_only="0"
verbose=""
dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
build_dir_name="build"
install_dir_name="install"
build_prefix="$dir/$build_dir_name"
install_prefix="$dir/$install_dir_name"
cross_compile_arch=""
host_arch="$(uname -m)"

# Use 8 threads for builds and tests by default. Use more threads if possible,
# but avoid overloading the system by using up to 50% of available cores.
threads=8
nproc=$(type -p nproc)
if [ -x "$nproc" -a -f /proc/loadavg ]; then
  loadavg=$(awk '{printf "%.0f\n", $1}' < /proc/loadavg)
  let threads="($($nproc) - $loadavg) / 2"
  if [ $threads -le 0 ]; then
    threads=1
  fi
fi

# Exit script on first error.
set -e

usage() {
  cat <<EOF
Usage: $0 [options]

Build the compiler under $build_prefix, then install under $install_prefix.

Options:
  -b type  Specify CMake build type (default: $buildtype).
  -c       Use ccache (default: $use_ccache).
  -e       Build for embedded cross tool chain.
  -h       Display this help message.
  -i       Install the build (default: $do_install).
  -I name  Specify install directory name (default: "$install_dir_name").
  -j N     Allow N jobs at once (default: $threads).
  -o       Enable LLVM_INSTALL_TOOLCHAIN_ONLY=ON.
  -r       Delete $install_prefix and perform a clean build (default: incremental).
  -s       Strip binaries and minimize file permissions when (re-)installing.
  -t       Enable unit tests for components that support them (make check-all).
  -v       Enable verbose build output (default: quiet).
  -X archs Build only the specified semi-colon-delimited list of backends (default: "$backends").
  -x arch  Build cross-compiling toolchain for the specified target.
EOF
}

# Process command-line options. Remember the options for passing to the
# containerized build script.
while getopts :b:cehiI:j:orstvX:x: optchr; do
  case "$optchr" in
    b)
      buildtype="$OPTARG"
      case "${buildtype,,}" in
        release)
          split_dwarf=off
          ;;
        debug|relwithdebinfo)
          ;;
        *)
          echo "$0: invalid build type '$buildtype'"
          exit 1
          ;;
      esac
      ;;
    c)
      use_ccache="1"
      ;;
    e)
      embedded_toolchain="1"
      ;;
    h)
      usage
      exit
      ;;
    i)
      do_install="1"
      ;;
    I)
      install_dir_name="$OPTARG"
      install_prefix="$dir/$install_dir_name"
      ;;
    j)
      threads="$OPTARG"
      ;;
    o)
      install_toolchain_only=1
      ;;
    r)
      clean=1
      ;;
    s)
      install="install/strip"
      ;;
    t)
      unit_test=check-all
      ;;
    v)
      verbose="VERBOSE=1"
      ;;
    X)
      backends="$OPTARG"
      ;;
    x)
      cross_compile_arch=$OPTARG
      case "${cross_compile_arch,,}" in
        aarch64)
          ;;
        riscv64)
          # RISCV backend is not built by default.
          backends+=";RISCV"
          ;;
        *)
          echo "$0: unsupported architecture for cross-compilation: $cross_compile_arch"
          exit 1
          ;;
      esac
      ;;    :)
      echo "$0: missing argument for option '-$OPTARG'"
      exit 1
      ;;
    ?)
      echo "$0: invalid option '-$OPTARG'"
      exit 1
      ;;
  esac
done

# Run a command as a background process, wait for its termination, and die
# if its exit code is non-zero. Running a command in the background and
# waiting allows the trap handler in this script to catch a signal and
# immediately stop all child processes. Note that sub-shells are not
# interruptible unless they also enable job control and wait for background
# commands. For this reason, sub-shells should be avoided.
run_or_die() {
  set -x
  "$@" &
  set +x
  wait $! || exit 1
}

COMMON_CFLAGS="-pthread"

generator="Unix Makefiles"

CMAKE_OPTIONS="-DCMAKE_INSTALL_PREFIX=$install_prefix \
               -DCMAKE_BUILD_TYPE=$buildtype \
               -DCMAKE_C_COMPILER=$C_COMPILER_PATH \
               -DCMAKE_CXX_COMPILER=$CXX_COMPILER_PATH \
               -DLLVM_TARGETS_TO_BUILD=$backends "

# Warning: the -DLLVM_ENABLE_PROJECTS option is specified with cmake
# to avoid issues with nested quotation marks
llvm_use_ccache=""
if [ $use_ccache == "1" ]; then
  echo "Build using ccache"
  llvm_use_ccache="-DCMAKE_C_COMPILER_LAUNCHER=ccache \
                   -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "
fi

if [ $embedded_toolchain == "1" ]; then
  echo "Build for embedded cross tool chain"
  enabled_projects="clang;lld;compiler-rt;"
fi

# When set LLVM_INSTALL_TOOLCHAIN_ONLY to On it removes many of the LLVM development
# and testing tools as well as component libraries from the default install target.
if [ $install_toolchain_only == "1" ]; then
  echo "Only install toolchain"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON"
fi

# Build and install
if [ $clean -eq 1 -a -e "$install_prefix" ]; then
  rm -rf "$install_prefix"
fi
mkdir -p "$install_prefix/bin"

if [ $clean -eq 1 -a -e "$build_prefix" ]; then
  rm -rf "$build_prefix"
fi

mkdir -p "$build_prefix" && cd "$build_prefix"
run_or_die \
  cmake $CMAKE_OPTIONS \
      $llvm_use_ccache \
      -DCOMPILER_RT_BUILD_SANITIZERS=on \
      -DLLVM_ENABLE_PROJECTS=$enabled_projects \
      -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
      -DLLVM_USE_LINKER=gold \
      -DLLVM_LIT_ARGS="-sv -j$threads" \
      -DLLVM_USE_SPLIT_DWARF=$split_dwarf \
      -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib"\
      -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      $verbose \
      ../llvm

make -j$threads
if [ $do_install == "1" ]; then
  make -j$threads $verbose $install
fi

if [ -n "$unit_test" ]; then
  make -j$threads $verbose check-all
fi

build_openmp() {
  local cross_suffix="-cross"
  local stage_install_prefix="$cross_compile_install_prefix"
  local cross_cmake_flags="$cross_cmake_flags \
                           -DOPENMP_ENABLE_LIBOMPTARGET=off"
  if [ -d "$libelf_path" ]; then
    cross_cmake_flags="$cross_cmake_flags \
                       -DLIBOMPTARGET_DEP_LIBELF_INCLUDE_DIR=$libelf_path/include \
                       -DLIBOMPTARGET_DEP_LIBELF_LIBRARIES=$libelf_path/lib"
  fi
  if [ -d "$libffi_path" ]; then
    cross_cmake_flags="$cross_cmake_flags \
                       -DLIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIR=$libffi_path/include \
                       -DLIBOMPTARGET_DEP_LIBFFI_LIBRARIES=$libffi_path/lib64"
  fi

  echo $1
  if [ "$1" = "static" ]; then
    local static_suffix="-static"
    local static_cmake_flags="-DLIBOMP_ENABLE_SHARED=off \
                              -DLIBOMP_OMPT_SUPPORT=off \
                              -DOPENMP_ENABLE_LIBOMPTARGET=off \
                              -DOPENMP_ENABLE_OMPT_TOOLS=off"
  else
    local static_suffix=""
    local static_cmake_flags=""
  fi

  echo "$0: building openmp$static_suffix$cross_suffix"

  if [ $clean -eq 1 ]; then
    rm -rf "$build_prefix"/projects/openmp"$static_suffix$cross_suffix"
  fi
	
  mkdir -p "$build_prefix"/projects/openmp"$static_suffix$cross_suffix" && cd "$build_prefix"/projects/openmp"$static_suffix$cross_suffix"
  # Disable OMPT and libomptarget, to prevent overwriting shared libraries
  # that have already been built when building static archives.
  run_or_die \
    cmake -Wno-dev -G "$generator" \
          -DCMAKE_BUILD_TYPE=$buildtype \
          -DCMAKE_INSTALL_PREFIX="$stage_install_prefix" \
          -DCMAKE_INSTALL_MESSAGE=LAZY \
          $llvm_use_ccache \
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          -DLIBOMP_ENABLE_ASSERTIONS=off \
          $static_cmake_flags \
          -DOPENMP_LLVM_TOOLS_DIR="$(dirname $llvm_lit)" \
          -DOPENMP_LIT_ARGS="-sv -j$threads" \
          -DOPENMP_TEST_CXX_COMPILER="$stage_install_prefix/bin/clang++" \
          -DOPENMP_TEST_C_COMPILER="$stage_install_prefix/bin/clang" \
          $cross_cmake_flags \
          ../../../openmp

  run_or_die make -j$threads
  run_or_die make -j$threads $verbose $install
}

build_libunwind() {
  local stage_install_prefix="$cross_compile_install_prefix"
  local cross_cmake_flags="$cross_cmake_flags"

  echo "$0: building libunwind-cross"
  if [ $clean -eq 1 ]; then
    rm -rf "$build_prefix"/projects/libunwind-cross
  fi
  mkdir -p "$build_prefix"/projects/libunwind-cross && cd "$build_prefix"/projects/libunwind-cross
  run_or_die \
    cmake -Wno-dev -G "$generator" \
          -DCMAKE_BUILD_TYPE=$buildtype \
          -DCMAKE_INSTALL_PREFIX="$stage_install_prefix" \
          -DCMAKE_INSTALL_MESSAGE=LAZY \
          $llvm_use_ccache \
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          $cross_cmake_flags \
          ../../../libunwind

  run_or_die make -j$threads
  run_or_die make -j$threads $verbose $install
}

build_compiler_rt() {
  local cross_cmake_flags="$cross_cmake_flags"
  local stage_install_prefix="$($install_prefix/bin/clang --print-resource-dir)"

  echo "$0: building compiler-rt-cross"
  if [ $clean -eq 1 ]; then
    rm -rf "$build_prefix"/projects/compiler-rt-cross
  fi

  mkdir -p "$build_prefix"/projects/compiler-rt-cross && cd "$build_prefix"/projects/compiler-rt-cross
  run_or_die \
     cmake -Wno-dev -G "$generator" \
           -DCMAKE_BUILD_TYPE=$buildtype \
           -DCMAKE_INSTALL_PREFIX="$stage_install_prefix" \
           -DCMAKE_INSTALL_MESSAGE=LAZY \
           -DCMAKE_C_COMPILER=clang \
           -DCMAKE_CXX_COMPILER=clang++ \
           -DCOMPILER_RT_BUILD_BUILTINS=on \
           -DCOMPILER_RT_BUILD_LIBFUZZER=off \
           -DCOMPILER_RT_BUILD_PROFILE=on \
           -DCOMPILER_RT_BUILD_SANITIZERS=off \
           -DCOMPILER_RT_BUILD_XRAY=off \
           -DCOMPILER_RT_DEFAULT_TARGET_ONLY=ON \
           $cross_cmake_flags \
           ../../../compiler-rt

  run_or_die make -j$threads
  run_or_die make -j$threads $verbose $install
}

build_libcxx() {
  echo $install_libcxx
  local stage_install_prefix="$cross_compile_install_prefix"
  local cross_cmake_flags="$cross_cmake_flags"

  echo "$0: building libcxx-cross"
  if [ $clean -eq 1 -a -e "$build_prefix/projects/libcxx-cross" ]; then
    rm -rf "$build_prefix/projects/libcxx-cross"
  fi
  mkdir -p "$build_prefix/projects/libcxx-cross" && cd "$build_prefix/projects/libcxx-cross"
  run_or_die \
    cmake -Wno-dev -G "$generator" \
          -DLLVM_ENABLE_PROJECTS="libcxx;libcxxabi" \
	  -DCMAKE_BUILD_TYPE=$buildtype \
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          -DLLVM_TABLEGEN=$install_prefix/bin/llvm-tblgen \
          -DCMAKE_INSTALL_PREFIX="$stage_install_prefix" \
          -DCMAKE_INSTALL_MESSAGE=LAZY \
          -DLLVM_ENABLE_ASSERTIONS=off \
		  -DLLVM_ENABLE_ZSTD=off \
          -DLLVM_USE_SPLIT_DWARF=$split_dwarf \
          -DLLVM_LIT_ARGS="-sv -j$threads" \
          $cross_cmake_flags \
          ../../../llvm

  run_or_die make -j$threads $verbose
  run_or_die make -j$threads $verbose $install
}

path_orig="$PATH"
ld_library_path_orig="$LD_LIBRARY_PATH"
export PATH="$install_prefix/bin:$path_orig"
export LD_LIBRARY_PATH="$install_prefix/lib:$ld_library_path_orig"

if [ "$cross_compile_arch" != "" ]; then
  if [ "$host_arch" = "$cross_compile_arch" ]; then
    echo "$0: cannot cross-compile from $host_arch to $cross_compile_arch"
    exit 1
  fi

  host_triple="$(llvm-config --host-target)"
  cross_compile_triple="${host_triple/$host_arch/$cross_compile_arch}"
  cross_compile_install_prefix="$install_prefix/$cross_compile_triple"
  mkdir -p "$cross_compile_install_prefix"

  cross_cmake_flags="-DCMAKE_C_COMPILER_TARGET=$cross_compile_triple \
                     -DCMAKE_CXX_COMPILER_TARGET=$cross_compile_triple \
                     -DCMAKE_ASM_COMPILER_TARGET=$cross_compile_triple \
                     -DCMAKE_SYSROOT=$sysroot \
                     -DCMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN=$gcc_toolchain \
                     -DCMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN=$gcc_toolchain"

  build_openmp shared
  build_openmp static
  build_libunwind
  build_compiler_rt
  build_libcxx
fi

cd ..

echo "$0: SUCCESS"
