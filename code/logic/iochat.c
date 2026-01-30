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
#include "fossil/ai/iochat.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================
   Internal Session Structure
   ============================================================ */
struct fossil_ai_chat_session {
    fossil_ai_jellyfish_core_t* core;
    fossil_ai_jellyfish_model_t* model;
    fossil_ai_jellyfish_id_t session_id;
    fossil_ai_chat_message_t* messages;
    unsigned long message_count;
    unsigned long message_capacity;
};

/* ============================================================
   Session Lifecycle
   ============================================================ */

fossil_ai_chat_session_t*
fossil_ai_chat_session_create(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_id_t session_id
) {
    fossil_ai_chat_session_t* session = (fossil_ai_chat_session_t*)malloc(sizeof(fossil_ai_chat_session_t));
    if (!session) return NULL;
    session->core = core;
    session->model = model;
    session->session_id = session_id;
    session->message_count = 0;
    session->message_capacity = 8;
    session->messages = (fossil_ai_chat_message_t*)malloc(sizeof(fossil_ai_chat_message_t) * session->message_capacity);
    if (!session->messages) {
        free(session);
        return NULL;
    }
    return session;
}

void
fossil_ai_chat_session_destroy(fossil_ai_chat_session_t* session) {
    if (!session) return;
    for (unsigned long i = 0; i < session->message_count; ++i) {
        /* Free blob data if owned; here we assume caller owns content.data, so skip freeing */
    }
    free(session->messages);
    free(session);
}

/* ============================================================
   Message Handling
   ============================================================ */

int
fossil_ai_chat_add_message(
    fossil_ai_chat_session_t* session,
    fossil_ai_chat_role_t role,
    fossil_ai_jellyfish_blob_t content
) {
    if (session->message_count >= session->message_capacity) {
        unsigned long new_capacity = session->message_capacity * 2;
        fossil_ai_chat_message_t* tmp = (fossil_ai_chat_message_t*)realloc(session->messages, sizeof(fossil_ai_chat_message_t) * new_capacity);
        if (!tmp) return -1;
        session->messages = tmp;
        session->message_capacity = new_capacity;
    }
    session->messages[session->message_count].role = role;
    session->messages[session->message_count].content = content;
    session->message_count++;
    return 0;
}

unsigned long
fossil_ai_chat_message_count(const fossil_ai_chat_session_t* session) {
    return session->message_count;
}

const fossil_ai_chat_message_t*
fossil_ai_chat_get_message(const fossil_ai_chat_session_t* session, unsigned long index) {
    if (!session || index >= session->message_count) return NULL;
    return &session->messages[index];
}

/* ============================================================
   Context Materialization
   ============================================================ */

fossil_ai_jellyfish_context_t*
fossil_ai_chat_materialize_context(fossil_ai_chat_session_t* session) {
    if (!session) return NULL;
    fossil_ai_jellyfish_context_t* context = fossil_ai_jellyfish_context_create(session->core, session->session_id);
    if (!context) return NULL;

    for (unsigned long i = 0; i < session->message_count; ++i) {
        fossil_ai_jellyfish_context_add(context, session->messages[i].content);
    }
    return context;
}

/* ============================================================
   Chat Inference
   ============================================================ */

int
fossil_ai_chat_turn(
    fossil_ai_chat_session_t* session,
    const char* user_input,
    fossil_ai_jellyfish_blob_t* assistant_output
) {
    if (!session || !user_input || !assistant_output) return -1;

    fossil_ai_jellyfish_blob_t user_blob = { .data = user_input, .size = (unsigned long)strlen(user_input), .media_type = "text/plain" };
    if (fossil_ai_chat_add_message(session, FOSSIL_AI_CHAT_ROLE_USER, user_blob) != 0) return -1;

    fossil_ai_jellyfish_context_t* context = fossil_ai_chat_materialize_context(session);
    if (!context) return -1;

    fossil_ai_jellyfish_blob_t output = {0};
    int ret = fossil_ai_jellyfish_ask(session->core, session->model, context, user_input, &output);

    fossil_ai_jellyfish_context_destroy(context);

    if (ret != 0) return ret;

    fossil_ai_chat_add_message(session, FOSSIL_AI_CHAT_ROLE_ASSISTANT, output);
    if (assistant_output) *assistant_output = output;
    return 0;
}

/* ============================================================
   Summarization & Compression
   ============================================================ */

int
fossil_ai_chat_summarize(fossil_ai_chat_session_t* session, fossil_ai_jellyfish_blob_t* summary) {
    if (!session || !summary) return -1;
    fossil_ai_jellyfish_context_t* context = fossil_ai_chat_materialize_context(session);
    if (!context) return -1;

    int ret = fossil_ai_jellyfish_summary(session->core, session->model, context, summary);
    fossil_ai_jellyfish_context_destroy(context);
    return ret;
}

int
fossil_ai_chat_compact(fossil_ai_chat_session_t* session) {
    if (!session) return -1;
    fossil_ai_jellyfish_blob_t summary = {0};
    if (fossil_ai_chat_summarize(session, &summary) != 0) return -1;

    /* Clear all previous messages */
    free(session->messages);
    session->messages = (fossil_ai_chat_message_t*)malloc(sizeof(fossil_ai_chat_message_t) * 8);
    session->message_capacity = 8;
    session->message_count = 0;

    /* Add summary as system message */
    fossil_ai_chat_add_message(session, FOSSIL_AI_CHAT_ROLE_SYSTEM, summary);
    return 0;
}

/* ============================================================
   Integrity & Audit
   ============================================================ */

fossil_ai_jellyfish_hash_t
fossil_ai_chat_context_hash(fossil_ai_chat_session_t* session) {
    fossil_ai_jellyfish_context_t* context = fossil_ai_chat_materialize_context(session);
    if (!context) return (fossil_ai_jellyfish_hash_t){0};
    fossil_ai_jellyfish_hash_t hash = fossil_ai_jellyfish_context_hash(context);
    fossil_ai_jellyfish_context_destroy(context);
    return hash;
}

fossil_ai_jellyfish_audit_t*
fossil_ai_chat_audit_model(fossil_ai_chat_session_t* session) {
    if (!session) return NULL;
    return fossil_ai_jellyfish_audit(session->core, session->model->model_type);
}
