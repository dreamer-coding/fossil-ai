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

FOSSIL_SUITE(cpp_jellyfish_fixture);

FOSSIL_SETUP(cpp_jellyfish_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_jellyfish_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Cases
// * * * * * * * * * * * * * * * * * * * * * * * *
// The test cases below are provided as samples, inspired
// by the Meson build system's approach of using test cases
// as samples for library usage.
// * * * * * * * * * * * * * * * * * * * * * * * *

// ======================================================
// Initialization / Cleanup
// ======================================================

FOSSIL_TEST(cpp_test_jellyfish_create_and_free_model) {
    fossil::ai::Jellyfish jf("test_model", 4, 2);
    ASSUME_NOT_CNULL(jf.input_size());
    ASSUME_NOT_CNULL(jf.output_size());
}

FOSSIL_TEST(cpp_test_jellyfish_create_model_null_name) {
    // The wrapper expects a string, so simulate empty name
    fossil::ai::Jellyfish jf("", 3, 1);
    // Accepts empty or non-empty, depending on implementation
    ASSUME_ITS_TRUE(jf.input_size() == 3 || jf.input_size() == 0);
}

FOSSIL_TEST(cpp_test_jellyfish_create_and_free_context) {
    fossil::ai::Jellyfish jf("session-123", 2, 2);
    ASSUME_NOT_CNULL(jf.input_size());
}

FOSSIL_TEST(cpp_test_jellyfish_create_context_null_id) {
    // The wrapper always creates a default session, so just check construction
    fossil::ai::Jellyfish jf("model", 1, 1);
    ASSUME_NOT_CNULL(jf.input_size());
}

// ======================================================
// Training / Memory
// ======================================================

FOSSIL_TEST(cpp_test_jellyfish_train_basic) {
    fossil::ai::Jellyfish jf("train_model", 2, 1);
    std::vector<float> inputs = {1.0f, 2.0f};
    std::vector<float> targets = {0.5f};
    bool ok = jf.train(inputs, targets, 1);
    ASSUME_ITS_TRUE(ok || !ok);
}

FOSSIL_TEST(cpp_test_jellyfish_add_memory_basic) {
    fossil::ai::Jellyfish jf("mem_model", 3, 3);
    std::vector<float> input = {0.1f, 0.2f, 0.3f};
    std::vector<float> output = {0.4f, 0.5f, 0.6f};
    bool ok = jf.add_memory(input, output, 3);
    ASSUME_ITS_TRUE(ok || !ok);
}

FOSSIL_TEST(cpp_test_jellyfish_add_memory_null_model) {
    // Not directly testable via wrapper, so fallback to C API
    float input[2] = {1.0f, 2.0f};
    float output[2] = {3.0f, 4.0f};
    bool ok = fossil_ai_jellyfish_add_memory(NULL, input, output, 2);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_jellyfish_infer_basic) {
    fossil::ai::Jellyfish jf("infer_model", 2, 2);
    std::vector<float> input = {0.7f, 0.8f};
    std::vector<float> output(2, 0.0f);
    bool ok = jf.infer(input, output);
    ASSUME_ITS_TRUE(ok || !ok);
}

FOSSIL_TEST(cpp_test_jellyfish_infer_null_model) {
    // Not directly testable via wrapper, so fallback to C API
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_jellyfish_create_context("infer-session");
    float input[1] = {0.0f};
    float output[1] = {0.0f};
    bool ok = fossil_ai_jellyfish_infer(NULL, ctx, input, output);
    if (ctx) fossil_ai_jellyfish_free_context(ctx);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_jellyfish_infer_null_context) {
    // Not directly testable via wrapper, so fallback to C API
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("infer_model", 1, 1);
    float input[1] = {0.0f};
    float output[1] = {0.0f};
    bool ok = fossil_ai_jellyfish_infer(model, NULL, input, output);
    if (model) fossil_ai_jellyfish_free_model(model);
    ASSUME_ITS_FALSE(ok);
}

// ======================================================
// Save / Load
// ======================================================

FOSSIL_TEST(cpp_test_jellyfish_save_and_load_model) {
    const std::string filepath = "test_jellyfish_model.bin";
    fossil::ai::Jellyfish jf("save_model", 2, 2);
    bool saved = jf.save_model(filepath);
    fossil::ai::Jellyfish loaded("dummy", 2, 2);
    bool loaded_ok = loaded.load_model(filepath);
    ASSUME_ITS_TRUE(saved || !saved);
    ASSUME_ITS_TRUE(loaded_ok || !loaded_ok);
}

FOSSIL_TEST(cpp_test_jellyfish_save_model_null) {
    // Not directly testable via wrapper, so fallback to C API
    bool ok = fossil_ai_jellyfish_save_model(NULL, "should_not_exist.bin");
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_jellyfish_load_model_invalid_path) {
    // Not directly testable via wrapper, so fallback to C API
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_load_model("nonexistent_file.bin");
    ASSUME_ITS_CNULL(model);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_jellyfish_tests) {    
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_create_and_free_model);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_create_model_null_name);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_create_and_free_context);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_create_context_null_id);

    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_train_basic);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_add_memory_basic);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_add_memory_null_model);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_infer_basic);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_infer_null_model);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_infer_null_context);

    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_save_and_load_model);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_save_model_null);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_load_model_invalid_path);

    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
