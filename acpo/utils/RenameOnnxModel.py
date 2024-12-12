#===- RenameOnnxModel.py - Rename ONNX Models    ------------------------===//
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Copyright (C) 2021-2023. Huawei Technologies Co., Ltd. All rights reserved.
#
#===----------------------------------------------------------------------===//
import onnx

onnx_model = onnx.load('lu.onnx')

for i in range(len(onnx_model.graph.node)):
  for j in range(len(onnx_model.graph.node[i].input)):
    print('-'*60)
    print(onnx_model.graph.node[i].name)
    print(onnx_model.graph.node[i].input)
    for k in range(len(onnx_model.graph.node[i].input)):
      print(onnx_model.graph.node[i].input[k])
      if onnx_model.graph.node[i].input[k] == "input.1":
        onnx_model.graph.node[i].input[k] = "input_1"
        print(onnx_model.graph.node[i].input[k])

    print(onnx_model.graph.node[i].output)

  for j in range(len(onnx_model.graph.node[i].output)):
    print('-'*60)
    print(onnx_model.graph.node[i].name)
    print(onnx_model.graph.node[i].input)
    print(onnx_model.graph.node[i].output)

  onnx_model.graph.node[i].output[j] = onnx_model.graph.node[i].output[j].split(':')[0]

for i in range(len(onnx_model.graph.input)):
  print('-'*60)
  print(onnx_model.graph.input[i])
  onnx_model.graph.input[i].name = onnx_model.graph.input[i].name.replace(".", "_")

for i in range(len(onnx_model.graph.output)):
  print('-'*60)
  print(onnx_model.graph.output[i])
  onnx_model.graph.output[i].name = onnx_model.graph.output[i].name.replace(".", "_")

onnx.save(onnx_model, 'lu_mod.onnx')
