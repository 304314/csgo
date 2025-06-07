set(CLANG_SEMA_OPTIONS)
if (BUILD_FOR_OPENEULER)
  list(APPEND CLANG_SEMA_OPTIONS "-DBUILD_FOR_OPENEULER")
endif()

clang_tablegen(AttrTemplateInstantiate.inc -gen-clang-attr-template-instantiate
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SEMA_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrTemplateInstantiate)

clang_tablegen(AttrParsedAttrList.inc -gen-clang-attr-parsed-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SEMA_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrParsedAttrList)

clang_tablegen(AttrParsedAttrKinds.inc -gen-clang-attr-parsed-attr-kinds
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SEMA_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrParsedAttrKinds)

clang_tablegen(AttrSpellingListIndex.inc -gen-clang-attr-spelling-index
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SEMA_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrSpellingListIndex)

clang_tablegen(AttrParsedAttrImpl.inc -gen-clang-attr-parsed-attr-impl
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  ${CLANG_SEMA_OPTIONS}
  SOURCE ../Basic/Attr.td
  TARGET ClangAttrParsedAttrImpl)
