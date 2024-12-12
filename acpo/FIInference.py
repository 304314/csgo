#===- FIInference.py - ACPO Function Inlining Inference   ---------------===//
#
# Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Copyright (C) 2021-2023. Huawei Technologies Co., Ltd. All rights reserved.
#
#===----------------------------------------------------------------------===//
from MLInference import MLInference, ACPO_LOG
import numpy as np
import tensorflow as tf
import tensorflow_probability
import tf_agents


class FIInference(MLInference):

  def prepare_features(self):
    intdf = self.features[0:13]
    hardcodeddf = self.features[13:]
    input1 = np.array(intdf, dtype=np.int64)
    input2 = np.array(hardcodeddf[0], dtype=np.int64)
    input3 = np.array(hardcodeddf[1:3], dtype=np.float32)
    input4 = np.array(hardcodeddf[3], dtype=np.int32)

    input1 = input1.reshape(len(self.features) - 4, 1)
    input2 = input2.reshape(1, 1)
    input3 = input3.reshape(2, 1)
    input4 = input4.reshape(1, 1)

    return input1, input2, input3, input4

  def inference(self):
    """
        Run an inference pass with an already loaded model and having features ready.
        This is for function inlining only for other inferences please see inference().
        """
    ACPO_LOG("ACPO Model successfully loaded for FI.")

    input = self.prepare_features()
    norm_input = input[0]
    inlin_def = input[1]
    dis_rew = input[2]
    ste_typ = input[3]

    # Order of input is as defined in model-fi.acpo
    output = self.infer(
        # Flexible features:
        callee_basic_block_count=tf.constant(norm_input[0]),
        callsite_height=tf.constant(norm_input[1]),
        node_count=tf.constant(norm_input[2]),
        nr_ctant_params=tf.constant(norm_input[3]),
        cost_estimate=tf.constant(norm_input[4]),
        edge_count=tf.constant(norm_input[5]),
        caller_users=tf.constant(norm_input[6]),
        caller_conditionally_executed_blocks=tf.constant(norm_input[7]),
        caller_basic_block_count=tf.constant(norm_input[8]),
        callee_conditionally_executed_blocks=tf.constant(norm_input[9]),
        callee_users=tf.constant(norm_input[10]),
        block_freq=tf.constant(norm_input[11]),
        loop_level=tf.constant(norm_input[12]),
        # Hardcoded training:
        inlining_default=tf.constant(inlin_def),
        discount=tf.constant(dis_rew[0]),
        reward=tf.constant(dis_rew[1]),
        step_type=tf.constant(ste_typ))

    output = output.get(self.output_key)
    if (output is None):
      return {}
    output = output.numpy()
    # For now we assume that the order of outputs within the output class tuple is the same
    # as the order specified by the OutputList field in model.acpo.
    # A better way might be generating a dict of dicts or a list of dicts during load_model.
    # Then use index to get the output dict directly. The dict will look like this:
    # classes_dict = {0: {'LU-Count': 0, 'LU-Type': 3}, 1: {'LU-Count': 0, 'LU-Type': 2}, ...}
    output_dict = {}
    for i in range(len(self.output_names)):
      output_dict[self.output_names[i]] = output[i]
      ACPO_LOG("Prediction is FI-ShouldInline=" +
            str(output_dict.get("FI-ShouldInline")) + "\n")
    return output_dict
