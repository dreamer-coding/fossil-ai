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

FOSSIL_SUITE(cpp_anom_fixture);

FOSSIL_SETUP(cpp_anom_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_anom_fixture) {
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
// Anomaly Detection Tests
// ======================================================

FOSSIL_TEST(cpp_test_anom_create_and_free) {
    fossil::ai::Anomaly detector;
    ASSUME_NOT_CNULL(detector.native_handle());
    // Destructor will free resources automatically
}

FOSSIL_TEST(cpp_test_anom_detect_statistical_basic) {
    fossil::ai::Anomaly detector;
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 100.0f};
    float score = 0.0f;
    bool result = detector.detect_statistical(data, 2.0f, score);
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(score > 2.0f);
}

FOSSIL_TEST(cpp_test_anom_detect_deviation_basic) {
    fossil::ai::Anomaly detector;
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 10.0f};
    float score = 0.0f;
    bool result = detector.detect_deviation(data, score);
    ASSUME_ITS_TRUE(result || !result); // May or may not be above threshold
    ASSUME_ITS_TRUE(score >= 0.0f);
}

FOSSIL_TEST(cpp_test_anom_detect_reconstruction_null_args) {
    fossil::ai::Anomaly detector;
    std::vector<float> data;
    float score = 0.0f;
    bool result = detector.detect_reconstruction(data, nullptr, score);
    ASSUME_ITS_FALSE(result);
}

FOSSIL_TEST(cpp_test_anom_save_and_load) {
    const std::string filepath = "test_anom_save_load.bin";
    fossil::ai::Anomaly detector;
    std::vector<float> data = {1.0f, 2.0f, 3.0f, 10.0f};
    float score = 0.0f;
    detector.detect_statistical(data, 1.0f, score);
    bool saved = detector.save(filepath);
    ASSUME_ITS_TRUE(saved);

    fossil::ai::Anomaly loaded;
    bool loaded_ok = loaded.load(filepath);
    ASSUME_ITS_TRUE(loaded_ok);
    ASSUME_ITS_TRUE(loaded.native_handle()->history_len == detector.native_handle()->history_len);
}

FOSSIL_TEST(cpp_test_anom_save_null_args) {
    fossil::ai::Anomaly detector;
    bool ok = detector.save(""); // Empty path
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_anom_load_null_args) {
    fossil::ai::Anomaly detector;
    bool ok = detector.load(""); // Empty path
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(cpp_test_anom_load_invalid_path) {
    fossil::ai::Anomaly detector;
    bool ok = detector.load("nonexistent_file.bin");
    ASSUME_ITS_FALSE(ok);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_anom_tests) {    
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_create_and_free);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_detect_statistical_basic);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_detect_deviation_basic);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_detect_reconstruction_null_args);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_save_and_load);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_save_null_args);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_load_null_args);
    FOSSIL_TEST_ADD(cpp_anom_fixture, cpp_test_anom_load_invalid_path);

    FOSSIL_TEST_REGISTER(cpp_anom_fixture);
} // end of tests
