# ACPO AOT compile cmake file
set(ACPO_ABS_PATH "" CACHE PATH "Absolute Path to ACPO")
set(LLVM_ACPO_MODEL_NAMES "")
set(ACPO_MODEL_PATH "" CACHE PATH "Model Path in ACPO_ABS_PATH")
set(LLVM_ACPO_MODEL_SIGNATURES "")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)
# Test demo
#set(ACPO_ABS_PATH "ACPO_ABS_PATH")
#set(ACPO_MODEL_PATH PATH_TO_TF_MODEL)
#set(LLVM_ACPO_MODEL_NAMES MODEL_NAME)
#set(LLVM_ACPO_MODEL_SIGNATURES serving_default)

foreach (model_path IN LISTS ACPO_MODEL_PATH)
  set(LLVM_ACPO_MODEL_PATHS ${LLVM_ACPO_MODEL_PATHS};${ACPO_ABS_PATH}/${model_path})
endforeach()

function(acpo_tf_find_and_compile model tag_set signature_def_key fname cpp_class)
  set(prefix ${CMAKE_CURRENT_BINARY_DIR}/${fname})
  set(obj_file ${prefix}.o)
  set(hdr_file ${prefix}.h)
  string(REPLACE "lib" "include/llvm" new_hdr_file "${hdr_file}")
  message("Using model at " ${model})
  add_custom_command(OUTPUT ${obj_file} ${hdr_file}
    COMMAND ${TENSORFLOW_AOT_COMPILER} aot_compile_cpu
          --multithreading false
          --dir ${model}
          --tag_set ${tag_set}
          --signature_def_key ${signature_def_key}
          --output_prefix ${prefix}
          --cpp_class ${cpp_class}
          --target_triple ${LLVM_HOST_TRIPLE}
    COMMAND ${CMAKE_COMMAND} -E copy ${hdr_file} ${new_hdr_file}
  )

  # Aggregate the objects so that results of different tf_compile calls may be
  # grouped into one target.
  set(GENERATED_OBJS ${GENERATED_OBJS} ${obj_file} PARENT_SCOPE)
  set_source_files_properties(${obj_file} PROPERTIES
    GENERATED 1 EXTERNAL_OBJECT 1)

  set(GENERATED_HEADERS ${GENERATED_HEADERS} ${hdr_file} PARENT_SCOPE)
  set_source_files_properties(${hdr_file} PROPERTIES
    GENERATED 1)

endfunction()
