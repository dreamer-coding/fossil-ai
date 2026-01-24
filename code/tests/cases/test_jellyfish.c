/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include <fossil/pizza/framework.h>
#include "fossil/ai/framework.h"


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(c_jellyfish_fixture);

FOSSIL_SETUP(c_jellyfish_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_jellyfish_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE(c_test_jellyfish_model_create_and_free) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("test", 4, 2);
    ASSUME_ITS_TRUE(model != NULL);
    ASSUME_ITS_EQUAL_CSTR(model->name, "test");
    ASSUME_ITS_EQUAL_I32((int)model->input_size, 4);
    ASSUME_ITS_EQUAL_I32((int)model->output_size, 2);
    fossil_ai_jellyfish_free_model(model);
}

FOSSIL_TEST_CASE(c_test_jellyfish_context_create_and_free) {
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_jellyfish_create_context("sess42");
    ASSUME_ITS_TRUE(ctx != NULL);
    ASSUME_ITS_EQUAL_CSTR(ctx->session_id, "sess42");
    fossil_ai_jellyfish_free_context(ctx);
}

FOSSIL_TEST_CASE(c_test_jellyfish_add_memory_and_retrieve) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("memtest", 3, 3);
    float input[3] = {1.0f, 2.0f, 3.0f};
    float output[3] = {4.0f, 5.0f, 6.0f};
    bool ok = fossil_ai_jellyfish_add_memory(model, input, output, 3);
    ASSUME_ITS_TRUE(ok);
    ASSUME_ITS_EQUAL_I32((int)model->memory_len, 1);
    for (int i = 0; i < 3; ++i) {
        ASSUME_ITS_TRUE(fabsf(model->memory[0].embedding[i] - input[i]) < 0.0001f);
        ASSUME_ITS_TRUE(fabsf(model->memory[0].output[i] - output[i]) < 0.0001f);
    }
    fossil_ai_jellyfish_free_model(model);
}

FOSSIL_TEST_CASE(c_test_jellyfish_train_and_infer) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("train", 2, 1);
    float inputs[4] = {0.0f, 0.0f, 1.0f, 1.0f}; // 2 samples, 2 features each
    float targets[2] = {0.0f, 1.0f}; // 2 samples, 1 output each
    bool trained = fossil_ai_jellyfish_train(model, inputs, targets, 2);
    ASSUME_ITS_TRUE(trained);

    fossil_ai_jellyfish_context_t *ctx = fossil_ai_jellyfish_create_context("sess");
    float test_input[2] = {1.0f, 1.0f};
    float test_output[1] = {0.0f};
    bool inferred = fossil_ai_jellyfish_infer(model, ctx, test_input, test_output);
    ASSUME_ITS_TRUE(inferred);
    ASSUME_ITS_TRUE(test_output[0] > 0.5f); // Should be closer to 1.0

    fossil_ai_jellyfish_free_context(ctx);
    fossil_ai_jellyfish_free_model(model);
}

FOSSIL_TEST_CASE(c_test_jellyfish_save_and_load_model) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("persist", 2, 2);
    float input[2] = {0.5f, 0.5f};
    float output[2] = {1.0f, 0.0f};
    fossil_ai_jellyfish_add_memory(model, input, output, 2);

    const char *filepath = "test_jellyfish_model_save.bin";
    bool saved = fossil_ai_jellyfish_save_model(model, filepath);
    ASSUME_ITS_TRUE(saved);

    fossil_ai_jellyfish_model_t *loaded = fossil_ai_jellyfish_load_model(filepath);
    ASSUME_ITS_TRUE(loaded != NULL);
    ASSUME_ITS_EQUAL_CSTR(loaded->name, "persist");
    ASSUME_ITS_EQUAL_I32((int)loaded->input_size, 2);
    ASSUME_ITS_EQUAL_I32((int)loaded->output_size, 2);
    ASSUME_ITS_EQUAL_I32((int)loaded->memory_len, 1);
    for (int i = 0; i < 2; ++i) {
        ASSUME_ITS_TRUE(fabsf(loaded->memory[0].embedding[i] - input[i]) < 0.0001f);
        ASSUME_ITS_TRUE(fabsf(loaded->memory[0].output[i] - output[i]) < 0.0001f);
    }

    fossil_ai_jellyfish_free_model(model);
    fossil_ai_jellyfish_free_model(loaded);
    remove(filepath);
}

FOSSIL_TEST_CASE(c_test_jellyfish_infer_blends_with_memory) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("blend", 2, 2);
    float input[2] = {0.2f, 0.8f};
    float output[2] = {0.9f, 0.1f};
    fossil_ai_jellyfish_add_memory(model, input, output, 2);

    fossil_ai_jellyfish_context_t *ctx = fossil_ai_jellyfish_create_context("blendctx");
    float test_input[2] = {0.2f, 0.8f};
    float test_output[2] = {0.0f, 0.0f};
    bool ok = fossil_ai_jellyfish_infer(model, ctx, test_input, test_output);
    ASSUME_ITS_TRUE(ok);
    // Output should be influenced by memory, so close to {0.9, 0.1}
    ASSUME_ITS_TRUE(fabsf(test_output[0] - 0.9f) < 0.2f);
    ASSUME_ITS_TRUE(fabsf(test_output[1] - 0.1f) < 0.2f);

    fossil_ai_jellyfish_free_context(ctx);
    fossil_ai_jellyfish_free_model(model);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_jellyfish_tests) {    
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_model_create_and_free);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_context_create_and_free);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_add_memory_and_retrieve);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_train_and_infer);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_save_and_load_model);
    FOSSIL_TEST_ADD(c_jellyfish_fixture, c_test_jellyfish_infer_blends_with_memory);

    FOSSIL_TEST_REGISTER(c_jellyfish_fixture);
} // end of tests
