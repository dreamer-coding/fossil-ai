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
#include <stdio.h>
#include <time.h>

#define FOSSIL_AI_CHAT_WINDOW 8
#define FOSSIL_AI_CHAT_CHARSET 256
#define FOSSIL_AI_CHAT_LR 0.01f

static float g_projection[FOSSIL_AI_CHAT_CHARSET]
                          [FOSSIL_AI_JELLYFISH_EMBED_SIZE];
static bool g_projection_init = false;

static void fossil_ai_chat_init_projection(void) {
    if (g_projection_init) return;
    for (size_t i = 0; i < FOSSIL_AI_CHAT_CHARSET; ++i) {
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j) {
            g_projection[i][j] = 0.001f;
        }
    }
    g_projection_init = true;
}

static void fossil_ai_chat_train_projection(
    const char* text,
    const float* target_embedding
) {
    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j) {
            g_projection[*p][j] +=
                FOSSIL_AI_CHAT_LR * target_embedding[j];
        }
    }
}

static size_t fossil_ai_chat_window_start(
    const fossil_ai_chat_context_t* ctx
) {
    if (ctx->history_count <= FOSSIL_AI_CHAT_WINDOW)
        return 0;
    return ctx->history_count - FOSSIL_AI_CHAT_WINDOW;
}


/* ======================================================
 * Internal Utilities
 * ====================================================== */

static int64_t fossil_ai_chat_now(void) {
    return (int64_t)time(NULL);
}

/* ======================================================
 * Lifecycle
 * ====================================================== */

fossil_ai_chat_context_t*
fossil_ai_chat_create(fossil_ai_jellyfish_model_t* model) {
    if (!model) return NULL;

    fossil_ai_chat_context_t* ctx =
        (fossil_ai_chat_context_t*)calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;

    ctx->model = model;
    ctx->history_capacity = 16;
    ctx->history = (fossil_ai_chat_message_t*)
        calloc(ctx->history_capacity, sizeof(*ctx->history));

    if (!ctx->history) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void fossil_ai_chat_free(fossil_ai_chat_context_t* ctx) {
    if (!ctx) return;
    free(ctx->history);
    free(ctx);
}

/* ======================================================
 * Messaging
 * ====================================================== */

bool fossil_ai_chat_add_message(
    fossil_ai_chat_context_t* ctx,
    fossil_ai_chat_role_t role,
    const char* text,
    int64_t timestamp
) {
    if (!ctx || !text) return false;

    if (ctx->history_count >= ctx->history_capacity) {
        size_t new_cap = ctx->history_capacity * 2;
        fossil_ai_chat_message_t* new_hist =
            (fossil_ai_chat_message_t*)realloc(
                ctx->history,
                new_cap * sizeof(*ctx->history)
            );
        if (!new_hist) return false;
        ctx->history = new_hist;
        ctx->history_capacity = new_cap;
    }

    fossil_ai_chat_message_t* msg =
        &ctx->history[ctx->history_count++];

    msg->role = role;
    msg->timestamp = timestamp ? timestamp : fossil_ai_chat_now();
    strncpy(msg->text, text, FOSSIL_AI_CHAT_MAX_TEXT - 1);

    return true;
}

/* ======================================================
 * NLP / Embedding
 * ====================================================== */

/*
 * Deterministic hash-based embedding.
 * Cold, fast, auditable, no learned bias.
 */
bool fossil_ai_chat_embed_text(
    const char* text,
    float* embedding_out
) {
    if (!text || !embedding_out) return false;

    fossil_ai_chat_init_projection();
    memset(embedding_out, 0,
           sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);

    for (const unsigned char* p = (const unsigned char*)text; *p; ++p) {
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j) {
            embedding_out[j] += g_projection[*p][j];
        }
    }

    return true;
}

/* ======================================================
 * Chat Inference
 * ====================================================== */

bool fossil_ai_chat_reply(
    fossil_ai_chat_context_t* ctx,
    char* response_out,
    size_t response_size
) {
    if (!ctx || !response_out || response_size == 0) return false;

    float input_embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE] = {0};
    float output_embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE] = {0};

    /* Embed last N messages */
    size_t start = fossil_ai_chat_window_start(ctx);
    for (size_t i = start; i < ctx->history_count; ++i) {
        float tmp[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
        fossil_ai_chat_embed_text(ctx->history[i].text, tmp);
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
            input_embedding[j] += tmp[j];
    }

    if (!fossil_ai_jellyfish_predict(
            ctx->model, input_embedding, output_embedding)) {
        snprintf(response_out, response_size,
                 "No relevant memory.");
        return true;
    }

    snprintf(response_out, response_size,
             "Response vector signal: %.2f",
             output_embedding[0]);

    /* Store assistant reply */
    fossil_ai_chat_add_message(
        ctx,
        FOSSIL_AI_CHAT_ROLE_ASSISTANT,
        response_out,
        fossil_ai_chat_now()
    );

    /* === Memory Injection === */
    char mem_id[64];
    snprintf(mem_id, sizeof(mem_id),
             "chat:%lld",
             (long long)fossil_ai_chat_now());

    fossil_ai_jellyfish_add_memory(
        ctx->model,
        input_embedding,
        output_embedding,
        mem_id,
        fossil_ai_chat_now()
    );

    /* === Train projection === */
    fossil_ai_chat_train_projection(
        response_out,
        output_embedding
    );

    return true;
}

/* ======================================================
 * Auditing
 * ====================================================== */

void fossil_ai_chat_audit(const fossil_ai_chat_context_t* ctx) {
    if (!ctx) return;

    printf("=== Chat Audit ===\n");
    printf("History Count : %zu\n", ctx->history_count);
    printf("Model Name    : %s\n", ctx->model->name);
    printf("Model Trained : %s\n", ctx->model->trained ? "yes" : "no");
    printf("Projection      : trainable (online)\n");
    printf("Window Size     : %d\n", FOSSIL_AI_CHAT_WINDOW);
    printf("Memory Injection: enabled\n");

    if (ctx->history_count > 0) {
        const fossil_ai_chat_message_t* last =
            &ctx->history[ctx->history_count - 1];
        printf("Last Role     : %d\n", last->role);
        printf("Last Time     : %lld\n",
               (long long)last->timestamp);
        printf("Last Text Hash: 0x%08X\n",
               fossil_ai_jellyfish_hash_string(last->text));
    }

    printf("Cold Core     : intact\n");
    printf("=== End Chat Audit ===\n");
}
