  我们提供了build.sh脚本来构建llvm-project。
  我们已经验证了它可以在openEuler系统成功构建。

  在构建编译器之前，确保已经安装了以下工具：
  gcc \
  g++ \
  libgcc \
  openssl-devel \
  cmake \
  python3 \
  python3-setuptools \
  python-wheel \
  texinfo \
  binutils \
  libstdc++-static \
  libatomic

 交叉编译时需要额外安装autoconf、automake、libelf、libffi。
 具体的编译选项可以在build.sh中查看，也可以通过“./build.sh -h”命令查看。
 例如：
   ./build.sh -r -b release -X X86 -i
 交叉编译:
   export BISHENG_LIBELF_PATH=/xxx/libelf/install
   export BISHENG_LIBFFI_PATH=/xxx/libffi/install
   export BISHENG_CROSS_TOOLCHAIN=/xxx/gcc-arm-aarch64-none-linux-gnu/
   export BISHENG_CROSS_SYSROOT=$BISHENG_CROSS_TOOLCHAIN/aarch64-none-linux-gnu/libc
   ./build.sh -r -b release -X "AArch64;X86" -x aarch64 -i

