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
#ifndef FOSSIL_JELLYFISH_IOCHAT_H
#define FOSSIL_JELLYFISH_IOCHAT_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ======================================================
 * Constants
 * ====================================================== */

#define FOSSIL_AI_CHAT_MAX_TOKENS 512
#define FOSSIL_AI_CHAT_MAX_TEXT   2048

/* ======================================================
 * Types
 * ====================================================== */

typedef enum {
    FOSSIL_AI_CHAT_ROLE_SYSTEM,
    FOSSIL_AI_CHAT_ROLE_USER,
    FOSSIL_AI_CHAT_ROLE_ASSISTANT
} fossil_ai_chat_role_t;

typedef struct fossil_ai_chat_message_t {
    fossil_ai_chat_role_t role;
    char text[FOSSIL_AI_CHAT_MAX_TEXT];
    int64_t timestamp;
} fossil_ai_chat_message_t;

typedef struct fossil_ai_chat_context_t {
    fossil_ai_jellyfish_model_t* model;   // cold core
    fossil_ai_chat_message_t* history;
    size_t history_count;
    size_t history_capacity;
} fossil_ai_chat_context_t;

/* ======================================================
 * Lifecycle
 * ====================================================== */

fossil_ai_chat_context_t* fossil_ai_chat_create(
    fossil_ai_jellyfish_model_t* model
);

void fossil_ai_chat_free(fossil_ai_chat_context_t* ctx);

/* ======================================================
 * Messaging
 * ====================================================== */

bool fossil_ai_chat_add_message(
    fossil_ai_chat_context_t* ctx,
    fossil_ai_chat_role_t role,
    const char* text,
    int64_t timestamp
);

/* ======================================================
 * NLP / Inference
 * ====================================================== */

/* Convert text → embedding (pluggable NLP backend) */
bool fossil_ai_chat_embed_text(
    const char* text,
    float* embedding_out
);

/* Generate assistant reply */
bool fossil_ai_chat_reply(
    fossil_ai_chat_context_t* ctx,
    char* response_out,
    size_t response_size
);

/* ======================================================
 * Auditing
 * ====================================================== */

void fossil_ai_chat_audit(const fossil_ai_chat_context_t* ctx);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace fossil {

    namespace ai {
    
        //
    
    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
