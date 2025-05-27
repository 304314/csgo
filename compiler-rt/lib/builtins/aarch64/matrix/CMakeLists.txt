cmake_minimum_required(VERSION 3.20.0)

set(SME_MATRIX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/sme_matrix.c")
set(SME_MATRIX_DST "${LLVM_LIBRARY_DIR}/clang/${LLVM_VERSION_MAJOR}/lib/${LLVM_DEFAULT_TARGET_TRIPLE}")
add_library(sme_matrix SHARED ${SME_MATRIX_SRC})
set_target_properties(sme_matrix PROPERTIES
  COMPILE_FLAGS "-fPIC -march=armv9.1-a+sme"
  LINK_FLAGS "-march=armv9.1-a+sme"
  LIBRARY_OUTPUT_DIRECTORY ${SME_MATRIX_DST}
)
target_link_libraries(sme_matrix PRIVATE "${SME_MATRIX_DST}/libclang_rt.builtins.a")
