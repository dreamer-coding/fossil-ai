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

FOSSIL_TEST_SUITE(cpp_jellyfish_fixture);

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

FOSSIL_TEST_CASE(cpp_test_jellyfish_model_create_and_free) {
    fossil::ai::Jellyfish jf("test", 4, 2);
    ASSUME_ITS_EQUAL_CSTR(jf.name().c_str(), "test");
    ASSUME_ITS_EQUAL_I32((int)jf.input_size(), 4);
    ASSUME_ITS_EQUAL_I32((int)jf.output_size(), 2);
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_context_create_and_free) {
    fossil::ai::Jellyfish jf("sess42", 2, 2);
    // Context is created internally; just check model name
    ASSUME_ITS_EQUAL_CSTR(jf.name().c_str(), "sess42");
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_add_memory_and_retrieve) {
    fossil::ai::Jellyfish jf("memtest", 3, 3);
    std::vector<float> input = {1.0f, 2.0f, 3.0f};
    std::vector<float> output = {4.0f, 5.0f, 6.0f};
    bool ok = jf.add_memory(input, output, 3);
    ASSUME_ITS_TRUE(ok);
    // Direct access to memory not available in wrapper, so just check add_memory returns true
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_train_and_infer) {
    fossil::ai::Jellyfish jf("train", 2, 1);
    std::vector<float> inputs = {0.0f, 0.0f, 1.0f, 1.0f}; // 2 samples, 2 features each
    std::vector<float> targets = {0.0f, 1.0f}; // 2 samples, 1 output each
    bool trained = jf.train(inputs, targets, 2);
    ASSUME_ITS_TRUE(trained);

    std::vector<float> test_input = {1.0f, 1.0f};
    std::vector<float> test_output(1, 0.0f);
    bool inferred = jf.infer(test_input, test_output);
    ASSUME_ITS_TRUE(inferred);
    ASSUME_ITS_TRUE(test_output[0] > 0.5f); // Should be closer to 1.0
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_save_and_load_model) {
    fossil::ai::Jellyfish jf("persist", 2, 2);
    std::vector<float> input = {0.5f, 0.5f};
    std::vector<float> output = {1.0f, 0.0f};
    jf.add_memory(input, output, 2);

    const std::string filepath = "test_jellyfish_model_save.bin";
    bool saved = jf.save_model(filepath);
    ASSUME_ITS_TRUE(saved);

    fossil::ai::Jellyfish loaded("dummy", 2, 2);
    bool loaded_ok = loaded.load_model(filepath);
    ASSUME_ITS_TRUE(loaded_ok);
    ASSUME_ITS_EQUAL_CSTR(loaded.name().c_str(), "persist");
    ASSUME_ITS_EQUAL_I32((int)loaded.input_size(), 2);
    ASSUME_ITS_EQUAL_I32((int)loaded.output_size(), 2);

    remove(filepath.c_str());
}

FOSSIL_TEST_CASE(cpp_test_jellyfish_infer_blends_with_memory) {
    fossil::ai::Jellyfish jf("blend", 2, 2);
    std::vector<float> input = {0.2f, 0.8f};
    std::vector<float> output = {0.9f, 0.1f};
    jf.add_memory(input, output, 2);

    std::vector<float> test_input = {0.2f, 0.8f};
    std::vector<float> test_output(2, 0.0f);
    bool ok = jf.infer(test_input, test_output);
    ASSUME_ITS_TRUE(ok);
    // Output should be influenced by memory, so close to {0.9, 0.1}
    ASSUME_ITS_TRUE(fabsf(test_output[0] - 0.9f) < 0.2f);
    ASSUME_ITS_TRUE(fabsf(test_output[1] - 0.1f) < 0.2f);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_jellyfish_tests) {    
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_model_create_and_free);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_context_create_and_free);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_add_memory_and_retrieve);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_train_and_infer);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_save_and_load_model);
    FOSSIL_TEST_ADD(cpp_jellyfish_fixture, cpp_test_jellyfish_infer_blends_with_memory);

    FOSSIL_TEST_REGISTER(cpp_jellyfish_fixture);
} // end of tests
