#!/bin/bash

# Tools to use for bootstrapping.
C_COMPILER_PATH=gcc
CXX_COMPILER_PATH=g++

# Initialize our own variables:
enable_autotuner="1"
buildtype=RelWithDebInfo
backends="all"
build_for_openeuler="0"
enabled_projects="clang;lld;compiler-rt;openmp;clang-tools-extra"
embedded_toolchain="0"
split_dwarf=on
use_ccache="0"
enable_classic_flang="0"
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
host_arch="$(uname -m)"
# ACPO specific flags
enable_acpo="1"
acpo_aot=0
override_aot=1
acpo_aot_models="fi"
acpo_aot_cmake_flags=""
export OPENEULER_ACPO_DIR=""

if [ $acpo_aot -eq 1 ] && [ $enable_acpo -eq 0 ]; then
  echo "$0: acpo_aot need enable_acpo eq 1."
  exit 1
fi

if [ $acpo_aot -eq 1 ]; then
  if [ "$host_arch" = "aarch64" ]; then
    aot_arch=${host_arch}
    echo "$0: ACPO will be built for aarch64 architecture"
  elif [ "$host_arch" = "x86_64" ]; then
    aot_arch="x86"
    echo "$0: ACPO will be built for x86 architecture"
  else
    echo "$0: ${host_arch} is not yet a supported architecture for ACPO.
    Currently supporting aarch64 and x86 only.
    Please use '-a' to disable ACPO for building BiSheng compiler."
    exit 1
  fi
fi
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
  -a       Disable BiSheng-Autotuner.
  -A       Disable ACPO.
  -b type  Specify CMake build type (default: $buildtype).
  -c       Use ccache (default: $use_ccache).
  -e       Build for embedded cross tool chain.
  -E       Build for openEuler.
  -h       Display this help message.
  -i       Install the build (default: $do_install).
  -I name  Specify install directory name (default: "$install_dir_name").
  -d dir   Specify the build directory (default: "$build_dir_name").
  -j N     Allow N jobs at once (default: $threads).
  -o       Enable LLVM_INSTALL_TOOLCHAIN_ONLY=ON.
  -r       Delete $install_prefix and perform a clean build (default: incremental).
  -s       Strip binaries and minimize file permissions when (re-)installing.
  -t       Enable unit tests for components that support them (make check-all).
  -v       Enable verbose build output (default: quiet).
  -f       Enable classic flang.
  -X archs Build only the specified semi-colon-delimited list of backends (default: "$backends").
EOF
}

# Process command-line options. Remember the options for passing to the
# containerized build script.
while getopts :aAb:d:ceEhiI:j:orstvfX: optchr; do
  case "$optchr" in
    a)
      enable_autotuner="0"
      ;;
    A)
      enable_acpo="0"
      acpo_aot=0
      acpo_aot_models=""
      override_aot=0
      echo "$0: ACPO project is disabled!"
      ;;
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
    d)
      build_prefix="$OPTARG"
      ;;
    f)
      enable_classic_flang="1"
      ;;
    e)
      embedded_toolchain="1"
      ;;
    E)
      build_for_openeuler="1"
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
    :)
      echo "$0: missing argument for option '-$OPTARG'"
      exit 1
      ;;
    ?)
      echo "$0: invalid option '-$OPTARG'"
      exit 1
      ;;
  esac
done

CMAKE_OPTIONS="-DCMAKE_INSTALL_PREFIX=$install_prefix \
               -DCMAKE_BUILD_TYPE=$buildtype \
               -DCMAKE_C_COMPILER=$C_COMPILER_PATH \
               -DCMAKE_CXX_COMPILER=$CXX_COMPILER_PATH \
               -DLLVM_TARGETS_TO_BUILD=$backends "

gold=$(type -p ld.gold)
if [ -z "$gold" -o ! -x "$gold" ]; then
  echo "$0: no usable ld.gold"
  exit 1
fi

# If the invocation does not force a particular binutils installation, check
# that we are using an acceptable version.
if [ -n "$BINUTILS_INCDIR" ]; then
  llvm_binutils_incdir="-DLLVM_BINUTILS_INCDIR=$BINUTILS_INCDIR"
else
  incdir=$(realpath --canonicalize-existing $(dirname $gold)/../include)
  if [ -z "$incdir" -o ! -f "$incdir/plugin-api.h" ]; then
    echo "$0: plugin-api.h not found; required to build LLVMgold.so"
    exit 1
  fi
  llvm_binutils_incdir="-DLLVM_BINUTILS_INCDIR=$incdir"
fi

# TensorFlow environment variables must be set if AOT is requested.
# If the user didn't specify them, try to determine the proper values.
if [ $acpo_aot -eq 1 ]; then
  if [ -z "$TENSORFLOW_AOT_PATH" ]; then
    python_full_version=$(python3 -V 2>&1 | grep -o '3\.[0-9]\+\.[0-9]\+')
    python_version=$(python3 -V 2>&1 | grep -o '3\.[0-9]\+')
    echo "$0: $python_full_version"
    echo "$0: $python_version"
    echo "$0: $HOME"
    if [ -d "/usr/local/lib64/python${python_version}/site-packages/tensorflow" ]; then
      export TENSORFLOW_AOT_PATH="/usr/local/lib64/python${python_version}/site-packages/tensorflow"
    elif [ -d "$HOME/.local/lib/python${python_version}/site-packages/tensorflow" ]; then
      export TENSORFLOW_AOT_PATH="$HOME/.local/lib/python${python_version}/site-packages/tensorflow"
    elif [ -d "/opt/python-${python_full_version}/lib/python${python_version}/site-packages/tensorflow" ]; then
      export TENSORFLOW_AOT_PATH="/opt/python-${python_full_version}/lib/python${python_version}/site-packages/tensorflow"
    elif [ -d "/opt/buildtools/python-${python_full_version}/lib/python${python_version}/site-packages/tensorflow" ]; then
      export TENSORFLOW_AOT_PATH="/opt/buildtools/python-${python_full_version}/lib/python${python_version}/site-packages/tensorflow"
    else
      echo "$0: TENSORFLOW_AOT_PATH not set"
      exit 1
    fi
  fi
fi

if [ $acpo_aot -eq 1 ] && [ $enable_acpo -eq 1 ]; then
  echo "$0: ACPO AOT compilation enabled."

  if [ -z "$OPENEULER_ACPO_DIR" ]; then
    echo "$0: OPENEULER_ACPO_DIR not set"
    echo "$OPENEULER_ACPO_DIR"
    exit 1
  fi
  echo "$0: ACPO directory is found at $OPENEULER_ACPO_DIR"
  full_model_paths=""
  acpo_aot_model_names=""
  model_signatures=""
  model_override_headers=""
  override_dir=""

  if [ $override_aot -eq 1 ] && [ -d ${OPENEULER_ACPO_DIR}/overrides/ ]; then
    if [ -d "${OPENEULER_ACPO_DIR}/overrides/" ]; then
      override_dir="${OPENEULER_ACPO_DIR}/overrides/"
    else
      echo "$0: ACPO overrrides directory for pre-compiled models is not found!"
      exit 1
    fi
  fi

  for model in $acpo_aot_models; do
    acpo_file="${OPENEULER_ACPO_DIR}/model-${model}.acpo"
    if [ -z "${acpo_file}" ]; then
      echo "$0: Error loading model ${model}: Could not find ${acpo_file}"
      exit 1
    fi

    full_path="${OPENEULER_ACPO_DIR}/$(grep ModelDirectory ${acpo_file} | cut -d= -f2)"
    if [ -z "${full_path}" ]; then
      echo "$0: Error loading model ${model}: Could not find ModelDirectory in ${acpo_file}"
      exit 1
    fi

    model_name="$(grep ModelName ${acpo_file} | cut -d= -f2)"
    if [ -z "${model_name}" ]; then
      echo "$0: Error loading model ${model}: Could not find ModelName in ${acpo_file}"
      exit 1
    fi

    model_signature="$(grep Signature ${acpo_file} | cut -d= -f2)"
    if [ -z "${model_signature}" ]; then
      echo "$0: Error loading model ${model}: Could not find Signature in ${acpo_file}"
      exit 1
    fi

    full_model_paths="${full_model_paths}${full_path};"
    acpo_aot_model_names="${acpo_aot_model_names}${model_name};"
    model_signatures="${model_signatures}${model_signature};"
  done

  # Remove trailing characters
  full_model_paths=${full_model_paths%?}
  acpo_aot_model_names=${acpo_aot_model_names%?}
  model_signatures=${model_signatures%?}

  acpo_aot_cmake_flags="-DACPO_AOT=ON \
                        -DTENSORFLOW_AOT_PATH=${TENSORFLOW_AOT_PATH} \
                        -DLLVM_ACPO_MODEL_PATHS=${full_model_paths} \
                        -DLLVM_ACPO_MODEL_NAMES=${acpo_aot_model_names} \
                        -DLLVM_ACPO_MODEL_SIGNATURES=${model_signatures} \
                        -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=True \
                        -DLLVM_ACPO_OVERRIDE=${override_aot} \
                        -DLLVM_ACPO_OVERRIDE_PATH=${override_dir} \
                        -DLLVM_ACPO_OVERRIDE_ARCH=${aot_arch}"
fi

# Warning: the -DLLVM_ENABLE_PROJECTS option is specified with cmake
# to avoid issues with nested quotation marks
if [ $use_ccache == "1" ]; then
  echo "Build using ccache"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DCMAKE_C_COMPILER_LAUNCHER=ccache \
                -DCMAKE_CXX_COMPILER_LAUNCHER=ccache "
fi

if [ $enable_classic_flang == "1" ]; then
  echo "Enable classic flang"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DLLVM_ENABLE_CLASSIC_FLANG=on"
fi

if [ $embedded_toolchain == "1" ]; then
  echo "Build for embedded cross tool chain"
  enabled_projects="clang;lld;compiler-rt;"
  CMAKE_OPTIONS="$CMAKE_OPTIONS \
                -DLLVM_BUILD_FOR_EMBEDDED=ON"
fi

# When set LLVM_INSTALL_TOOLCHAIN_ONLY to On it removes many of the LLVM development
# and testing tools as well as component libraries from the default install target.
if [ $install_toolchain_only == "1" ]; then
  echo "Only install toolchain"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DLLVM_INSTALL_TOOLCHAIN_ONLY=ON"
fi

if [ $build_for_openeuler == "1" ]; then
  echo "Build for openEuler"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DBUILD_FOR_OPENEULER=ON"
fi

if [ $enable_autotuner == "1" ]; then
  echo "enable BiSheng-Autotuner"
  CMAKE_OPTIONS="$CMAKE_OPTIONS -DLLVM_ENABLE_AUTOTUNER=ON"
fi

if [ $enable_acpo == "1" ]; then
  echo "enable ACPO"
  export CFLAGS="-Wp,-DENABLE_ACPO ${CFLAGS}"
  export CXXFLAGS="-Wp,-DENABLE_ACPO ${CXXFLAGS}"
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
cmake $CMAKE_OPTIONS \
      -DCOMPILER_RT_BUILD_SANITIZERS=on \
      -DLLVM_ENABLE_PROJECTS=$enabled_projects \
      -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
      -DLLVM_USE_LINKER=gold \
      -DLLVM_LIT_ARGS="-sv -j$threads" \
      -DLLVM_USE_SPLIT_DWARF=$split_dwarf \
      -DCMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      -DCMAKE_EXE_LINKER_FLAGS_DEBUG="-Wl,--gdb-index -Wl,--compress-debug-sections=zlib" \
      -DBUILD_SHARED_LIBS=OFF \
      -DLLVM_ENABLE_LIBCXX=OFF \
      -DLLVM_ENABLE_ZLIB=ON \
      -DLLVM_BUILD_RUNTIME=ON \
      -DLLVM_INCLUDE_TOOLS=ON \
      -DLLVM_BUILD_TOOLS=ON \
      -DLLVM_INCLUDE_TESTS=ON \
      -DLLVM_BUILD_TESTS=ON \
      -DLLVM_INCLUDE_EXAMPLES=ON \
      -DLLVM_BUILD_EXAMPLES=OFF \
      -DCLANG_DEFAULT_PIE_ON_LINUX=ON \
      -DCLANG_ENABLE_ARCMT=ON \
      -DCLANG_ENABLE_STATIC_ANALYZER=ON \
      -DCLANG_PLUGIN_SUPPORT=ON \
      -DLLVM_DYLIB_COMPONENTS="all" \
      -DLLVM_ENABLE_PER_TARGET_RUNTIME_DIR=ON \
      -DCMAKE_SKIP_RPATH=ON \
      -DLLVM_ENABLE_FFI=ON \
      -DLLVM_ENABLE_RTTI=ON \
      -DLLVM_USE_PERF=ON \
      -DLLVM_INSTALL_GTEST=ON \
      -DLLVM_INCLUDE_UTILS=ON \
      -DLLVM_INSTALL_UTILS=ON \
      -DLLVM_INCLUDE_BENCHMARKS=OFF \
      -DENABLE_LINKER_BUILD_ID=ON \
      -DLLVM_ENABLE_EH=ON \
      -DCLANG_DEFAULT_UNWINDLIB=libgcc \
      -DLIBCXX_STATICALLY_LINK_ABI_IN_STATIC_LIBRARY=ON \
      -DLIBCXX_ENABLE_ABI_LINKER_SCRIPT=ON \
      -DLIBOMP_INSTALL_ALIASES=OFF \
      $llvm_binutils_incdir \
      $verbose \
      $acpo_aot_cmake_flags \
      ../llvm

make -j$threads
if [ $do_install == "1" ]; then
  make -j$threads $verbose $install
fi

if [ -n "$unit_test" ]; then
  make -j$threads $verbose check-all
fi

cd ..

# When building official deliverables, minimize file permissions under the
# installation directory.
if [ "$install" = "install/strip" ]; then
  find $install_prefix/bin/ -type f -exec strip {} \;
  find $install_prefix -type f -exec chmod a-w {} \;
fi

# In openEuler embedded building system, it need wrap llvm-readelf
# to replace binutils-readelf.
if [ -e "$install_prefix/bin/llvm-readobj" ]; then
  ln -sf llvm-readobj $install_prefix/bin/llvm-readelf
fi

echo "$0: SUCCESS"
