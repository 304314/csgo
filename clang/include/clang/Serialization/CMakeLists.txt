set(CLANG_SER_OPTIONS)
if(BUILD_FOR_OPENEULER)
  list(APPEND CLANG_SER_OPTIONS "-DBUILD_FOR_OPENEULER")
endif()

clang_tablegen(AttrPCHRead.inc -gen-clang-attr-pch-read
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SER_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrPCHRead)

clang_tablegen(AttrPCHWrite.inc -gen-clang-attr-pch-write
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SER_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrPCHWrite)
