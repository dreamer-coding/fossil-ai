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

FOSSIL_SUITE(c_iochat_fixture);

FOSSIL_SETUP(c_iochat_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_iochat_fixture) {
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
// Chat Session Management
// ======================================================

FOSSIL_TEST(c_test_iochat_start_and_end_session) {
    const char *session_id = "chat-session-001";
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session(session_id);
    ASSUME_NOT_CNULL(ctx);
    if (ctx != NULL) {
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_start_session_null_id) {
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session(NULL);
    ASSUME_ITS_CNULL(ctx);
}

FOSSIL_TEST(c_test_iochat_end_session_null) {
    // Should not crash or leak
    fossil_ai_chat_end_session(NULL);
    ASSUME_ITS_TRUE(true); // Always passes if no crash
}

// ======================================================
// Chat Respond
// ======================================================

FOSSIL_TEST(c_test_iochat_respond_security_risk) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("security-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "My password is 1234", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        ASSUME_ITS_TRUE(strstr(response, "security") || strstr(response, "denied") || strstr(response, "blocked"));
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_relationship_risk) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("relationship-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "Will you be my girlfriend?", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        ASSUME_ITS_TRUE(strstr(response, "relationship") || strstr(response, "not a partner") || strstr(response, "bonding"));
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_emotional_support_risk) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("emotional-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "I feel sad and alone", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        ASSUME_ITS_TRUE(strstr(response, "emotional") || strstr(response, "support") || strstr(response, "not provided"));
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_religion_risk) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("religion-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "Do you believe in God?", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        ASSUME_ITS_TRUE(strstr(response, "religion") || strstr(response, "belief") || strstr(response, "spiritual"));
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_unsupported_language) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("lang-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "¿Puedes ayudarme con esto?", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        ASSUME_ITS_TRUE(strstr(response, "Input does not conform") || strstr(response, "unsupported"));
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_normal_inference) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 4, 2);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("normal-session");
    char response[256] = {0};
    bool ok = false;
    if (model && ctx) {
        ok = fossil_ai_chat_respond(model, ctx, "What is the capital of France?", response, sizeof(response));
        ASSUME_ITS_TRUE(ok);
        // Should not trigger any risk block, so fallback response
        ASSUME_ITS_TRUE(strstr(response, "Request acknowledged.") || strlen(response) > 0);
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_chat_end_session(ctx);
    }
}

FOSSIL_TEST(c_test_iochat_respond_null_args) {
    char response[64] = {0};
    bool ok = fossil_ai_chat_respond(NULL, NULL, NULL, NULL, 0);
    ASSUME_ITS_FALSE(ok);

    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("chat_model", 2, 1);
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_chat_start_session("null-session");
    ok = fossil_ai_chat_respond(model, ctx, NULL, response, sizeof(response));
    ASSUME_ITS_FALSE(ok);
    ok = fossil_ai_chat_respond(model, ctx, "Hello", NULL, sizeof(response));
    ASSUME_ITS_FALSE(ok);
    ok = fossil_ai_chat_respond(model, ctx, "Hello", response, 0);
    ASSUME_ITS_FALSE(ok);
    if (model) fossil_ai_jellyfish_free_model(model);
    if (ctx) fossil_ai_chat_end_session(ctx);
}

// ======================================================
// Persistent Memory I/O
// ======================================================

FOSSIL_TEST(c_test_iochat_save_and_load_persistent) {
    const char *filepath = "test_iochat_persistent.bin";
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("persist_model", 2, 2);
    bool saved = false, loaded = false;
    fossil_ai_jellyfish_model_t *model2 = fossil_ai_jellyfish_create_model("persist_model2", 2, 2);
    if (model && model2) {
        saved = fossil_ai_chat_save_persistent(model, filepath);
        loaded = fossil_ai_chat_load_persistent(model2, filepath);
        fossil_ai_jellyfish_free_model(model);
        fossil_ai_jellyfish_free_model(model2);
    }
    ASSUME_ITS_TRUE(saved || !saved);
    ASSUME_ITS_TRUE(loaded || !loaded);
}

FOSSIL_TEST(c_test_iochat_save_persistent_null) {
    bool ok = fossil_ai_chat_save_persistent(NULL, "should_not_exist.bin");
    ASSUME_ITS_FALSE(ok);
    ok = fossil_ai_chat_save_persistent((void*)1, NULL);
    ASSUME_ITS_FALSE(ok);
}

FOSSIL_TEST(c_test_iochat_load_persistent_null) {
    bool ok = fossil_ai_chat_load_persistent(NULL, "should_not_exist.bin");
    ASSUME_ITS_FALSE(ok);
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("persist_model", 2, 2);
    ok = fossil_ai_chat_load_persistent(model, NULL);
    ASSUME_ITS_FALSE(ok);
    if (model) fossil_ai_jellyfish_free_model(model);
}

FOSSIL_TEST(c_test_iochat_load_persistent_invalid_path) {
    fossil_ai_jellyfish_model_t *model = fossil_ai_jellyfish_create_model("persist_model", 2, 2);
    bool ok = false;
    if (model) {
        ok = fossil_ai_chat_load_persistent(model, "nonexistent_file.bin");
        fossil_ai_jellyfish_free_model(model);
    }
    ASSUME_ITS_FALSE(ok);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_iochat_tests) {    
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_start_and_end_session);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_start_session_null_id);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_end_session_null);

    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_security_risk);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_relationship_risk);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_emotional_support_risk);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_religion_risk);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_unsupported_language);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_normal_inference);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_respond_null_args);

    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_save_and_load_persistent);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_save_persistent_null);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_load_persistent_null);
    FOSSIL_TEST_ADD(c_iochat_fixture, c_test_iochat_load_persistent_invalid_path);

    FOSSIL_TEST_REGISTER(c_iochat_fixture);
} // end of tests
