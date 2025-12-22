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

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_CHAT_MAX_HISTORY     128
#define FOSSIL_AI_CHAT_MAX_RESPONSE    512
#define FOSSIL_AI_CHAT_PERSISTENT_MAX  256

// ======================================================
// Intent Classification
// ======================================================

typedef enum {
    FOSSIL_AI_CHAT_INTENT_UNKNOWN = 0,
    FOSSIL_AI_CHAT_INTENT_QUESTION,
    FOSSIL_AI_CHAT_INTENT_COMMAND,
    FOSSIL_AI_CHAT_INTENT_STATEMENT,
    FOSSIL_AI_CHAT_INTENT_SOCIAL,
    FOSSIL_AI_CHAT_INTENT_RELATIONSHIP   // forbidden
} fossil_ai_chat_intent_t;

// ======================================================
// Persistent Memory (SAFE ONLY)
// ======================================================

typedef enum {
    FOSSIL_AI_CHAT_MEMORY_FACT = 1,
    FOSSIL_AI_CHAT_MEMORY_TASK,
    FOSSIL_AI_CHAT_MEMORY_SYSTEM
} fossil_ai_chat_memory_type_t;

typedef struct {
    fossil_ai_chat_memory_type_t type;
    float embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    int64_t timestamp;
} fossil_ai_chat_persistent_memory_t;

// ======================================================
// Session Management
// ======================================================

fossil_ai_jellyfish_context_t *
fossil_ai_chat_start_session(const char *session_id);

void
fossil_ai_chat_end_session(fossil_ai_jellyfish_context_t *ctx);

// ======================================================
// Chat
// ======================================================

bool
fossil_ai_chat_respond(fossil_ai_jellyfish_model_t *model,
                       fossil_ai_jellyfish_context_t *ctx,
                       const char *user_message,
                       char *response,
                       size_t response_len);

// ======================================================
// Persistent Memory I/O
// ======================================================

bool
fossil_ai_chat_save_persistent(const fossil_ai_jellyfish_model_t *model,
                               const char *path);

bool
fossil_ai_chat_load_persistent(fossil_ai_jellyfish_model_t *model,
                               const char *path);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

    namespace ai {



    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
