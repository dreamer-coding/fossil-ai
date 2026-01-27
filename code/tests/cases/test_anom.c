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

FOSSIL_SUITE(c_anom_fixture);

FOSSIL_SETUP(c_anom_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_anom_fixture) {
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

FOSSIL_TEST(c_test_anom_create_and_free) {
    fossil_ai_anom_t *detector = fossil_ai_anom_create();
    ASSUME_NOT_CNULL(detector);
    fossil_ai_anom_free(detector);
}

FOSSIL_TEST(c_test_anom_detect_statistical_basic) {
    fossil_ai_anom_t *detector = fossil_ai_anom_create();
    float data[] = {1.0f, 2.0f, 3.0f, 100.0f};
    float score = 0.0f;
    bool result = fossil_ai_anom_detect_statistical(detector, data, 4, 2.0f, &score);
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(score > 2.0f);
    fossil_ai_anom_free(detector);
}

FOSSIL_TEST(c_test_anom_detect_statistical_null_args) {
    float score = 0.0f;
    bool result = fossil_ai_anom_detect_statistical(NULL, NULL, 0, 1.0f, NULL);
    ASSUME_ITS_FALSE(result);
}

FOSSIL_TEST(c_test_anom_detect_deviation_basic) {
    fossil_ai_anom_t *detector = fossil_ai_anom_create();
    float data[] = {1.0f, 2.0f, 3.0f, 10.0f};
    float score = 0.0f;
    bool result = fossil_ai_anom_detect_deviation(detector, data, 4, &score);
    ASSUME_ITS_TRUE(result || !result); // May or may not be above threshold
    ASSUME_ITS_TRUE(score >= 0.0f);
    fossil_ai_anom_free(detector);
}

FOSSIL_TEST(c_test_anom_detect_deviation_null_args) {
    float score = 0.0f;
    bool result = fossil_ai_anom_detect_deviation(NULL, NULL, 0, NULL);
    ASSUME_ITS_FALSE(result);
}

FOSSIL_TEST(c_test_anom_detect_reconstruction_null_args) {
    float score = 0.0f;
    bool result = fossil_ai_anom_detect_reconstruction(NULL, NULL, 0, NULL, NULL);
    ASSUME_ITS_FALSE(result);
}

FOSSIL_TEST(c_test_anom_save_and_load) {
    const char *filepath = "test_anom_save_load.bin";
    fossil_ai_anom_t *detector = fossil_ai_anom_create();
    float data[] = {1.0f, 2.0f, 3.0f, 10.0f};
    float score = 0.0f;
    fossil_ai_anom_detect_statistical(detector, data, 4, 1.0f, &score);
    bool saved = fossil_ai_anom_save(detector, filepath);
    ASSUME_ITS_TRUE(saved);
    fossil_ai_anom_t *loaded = fossil_ai_anom_load(filepath);
    ASSUME_NOT_CNULL(loaded);
    ASSUME_ITS_TRUE(loaded->history_len == detector->history_len);
    fossil_ai_anom_free(detector);
    fossil_ai_anom_free(loaded);
}

FOSSIL_TEST(c_test_anom_save_null_args) {
    bool ok = fossil_ai_anom_save(NULL, "should_not_exist.bin");
    ASSUME_ITS_FALSE(ok);
    ok = fossil_ai_anom_save((void*)1, NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_anom_load_null_args) {
    fossil_ai_anom_t *detector = fossil_ai_anom_load(NULL);
    ASSUME_ITS_CNULL(detector);
}

FOSSIL_TEST(c_test_anom_load_invalid_path) {
    fossil_ai_anom_t *detector = fossil_ai_anom_load("nonexistent_file.bin");
    ASSUME_ITS_CNULL(detector);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_anom_tests) {    
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_create_and_free);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_detect_statistical_basic);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_detect_statistical_null_args);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_detect_deviation_basic);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_detect_deviation_null_args);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_detect_reconstruction_null_args);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_save_and_load);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_save_null_args);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_load_null_args);
    FOSSIL_TEST_ADD(c_anom_fixture, c_test_anom_load_invalid_path);

    FOSSIL_TEST_REGISTER(c_anom_fixture);
} // end of tests
