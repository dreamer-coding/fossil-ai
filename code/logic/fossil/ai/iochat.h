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

/* ============================================================
   Chat Types
   ============================================================ */

typedef struct fossil_ai_chat_session fossil_ai_chat_session_t;
typedef struct fossil_ai_chat_message fossil_ai_chat_message_t;

/* Message roles */
typedef enum {
    FOSSIL_AI_CHAT_ROLE_SYSTEM,
    FOSSIL_AI_CHAT_ROLE_USER,
    FOSSIL_AI_CHAT_ROLE_ASSISTANT,
    FOSSIL_AI_CHAT_ROLE_TOOL
} fossil_ai_chat_role_t;

/* Chat message (immutable once added) */
struct fossil_ai_chat_message {
    fossil_ai_chat_role_t role;
    fossil_ai_jellyfish_blob_t content;
};

/* ============================================================
   Session Lifecycle
   ============================================================ */

/* Create a chat session bound to a model */
fossil_ai_chat_session_t*
fossil_ai_chat_session_create(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_id_t session_id
);

/* Destroy session (does not destroy core/model) */
void
fossil_ai_chat_session_destroy(
    fossil_ai_chat_session_t* session
);

/* ============================================================
   Message Handling
   ============================================================ */

/* Append a message to the session history */
int
fossil_ai_chat_add_message(
    fossil_ai_chat_session_t* session,
    fossil_ai_chat_role_t role,
    fossil_ai_jellyfish_blob_t content
);

/* Retrieve message count */
unsigned long
fossil_ai_chat_message_count(
    const fossil_ai_chat_session_t* session
);

/* Read-only access to a message */
const fossil_ai_chat_message_t*
fossil_ai_chat_get_message(
    const fossil_ai_chat_session_t* session,
    unsigned long index
);

/* ============================================================
   Context Materialization
   ============================================================ */

/*
 * Materialize chat history into an immutable Jellyfish context.
 * Caller owns returned context.
 */
fossil_ai_jellyfish_context_t*
fossil_ai_chat_materialize_context(
    fossil_ai_chat_session_t* session
);

/* ============================================================
   Chat Inference
   ============================================================ */

/*
 * Execute a chat turn.
 * - Adds user message
 * - Builds context
 * - Calls jellyfish_ask
 * - Appends assistant response
 */
int
fossil_ai_chat_turn(
    fossil_ai_chat_session_t* session,
    const char* user_input,
    fossil_ai_jellyfish_blob_t* assistant_output
);

/* ============================================================
   Summarization & Compression
   ============================================================ */

/*
 * Summarize the current chat history.
 * Useful for long-running sessions or archival.
 */
int
fossil_ai_chat_summarize(
    fossil_ai_chat_session_t* session,
    fossil_ai_jellyfish_blob_t* summary
);

/*
 * Replace history with a system message containing a summary.
 * This is a destructive operation.
 */
int
fossil_ai_chat_compact(
    fossil_ai_chat_session_t* session
);

/* ============================================================
   Integrity & Audit
   ============================================================ */

/* Hash of the materialized chat context */
fossil_ai_jellyfish_hash_t
fossil_ai_chat_context_hash(
    fossil_ai_chat_session_t* session
);

/* Audit the underlying model used in the session */
fossil_ai_jellyfish_audit_t*
fossil_ai_chat_audit_model(
    fossil_ai_chat_session_t* session
)

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
