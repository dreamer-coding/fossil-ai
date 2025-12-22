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
#include "fossil/ai/jellyfish.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// ======================================================
// Helpers
// ======================================================

// Simple L2 distance for attention
static float fossil_ai_jellyfish_distance(const float *a, const float *b, size_t len) {
    float sum = 0.0f;
    for (size_t i = 0; i < len; i++) {
        float d = a[i] - b[i];
        sum += d * d;
    }
    return sqrtf(sum);
}

// ======================================================
// Model / Context Management
// ======================================================

fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_create_model(const char *name, size_t input_size, size_t output_size) {
    fossil_ai_jellyfish_model_t *model = (fossil_ai_jellyfish_model_t *)calloc(1, sizeof(fossil_ai_jellyfish_model_t));
    if (!model) return NULL;
    strncpy(model->name, name, sizeof(model->name)-1);
    model->version = 1;
    model->input_size = input_size;
    model->output_size = output_size;
    model->internal_state = NULL;
    model->memory_len = 0;
    return model;
}

void fossil_ai_jellyfish_free_model(fossil_ai_jellyfish_model_t *model) {
    if (!model) return;
    free(model->internal_state);
    free(model);
}

fossil_ai_jellyfish_context_t *fossil_ai_jellyfish_create_context(const char *session_id) {
    fossil_ai_jellyfish_context_t *ctx = (fossil_ai_jellyfish_context_t *)calloc(1, sizeof(fossil_ai_jellyfish_context_t));
    if (!ctx) return NULL;
    strncpy(ctx->session_id, session_id, sizeof(ctx->session_id)-1);
    ctx->history_len = 0;
    ctx->history = NULL;
    ctx->timestamp = (int64_t)time(NULL);
    return ctx;
}

void fossil_ai_jellyfish_free_context(fossil_ai_jellyfish_context_t *ctx) {
    if (!ctx) return;
    free(ctx->history);
    free(ctx);
}

// ======================================================
// Training / Memory
// ======================================================

bool fossil_ai_jellyfish_train(fossil_ai_jellyfish_model_t *model,
                               const float *inputs,
                               const float *targets,
                               size_t count) {
    if (!model || !inputs || !targets) return false;
    // Placeholder: adapt weights or embeddings
    printf("[Jellyfish] Training %zu samples on model '%s'\n", count, model->name);
    return true;
}

// Add new memory with embedding
bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t *model,
                                    const float *input,
                                    const float *output,
                                    size_t embed_len) {
    if (!model || embed_len > FOSSIL_AI_JELLYFISH_EMBED_SIZE) return false;

    size_t idx = model->memory_len % FOSSIL_AI_JELLYFISH_MAX_MEMORY;
    for (size_t i = 0; i < embed_len; i++) {
        model->memory[idx].embedding[i] = input[i];
        model->memory[idx].output[i] = output[i];
    }
    model->memory[idx].timestamp = (int64_t)time(NULL);
    if (model->memory_len < FOSSIL_AI_JELLYFISH_MAX_MEMORY) model->memory_len++;
    return true;
}

// ======================================================
// Inference / Attention
// ======================================================

bool fossil_ai_jellyfish_infer(fossil_ai_jellyfish_model_t *model,
                               fossil_ai_jellyfish_context_t *ctx,
                               const float *input,
                               float *output) {
    if (!model || !input || !output) return false;

    // Attention: find closest memory vector
    size_t best_idx = 0;
    float best_dist = INFINITY;
    for (size_t i = 0; i < model->memory_len; i++) {
        float d = fossil_ai_jellyfish_distance(input, model->memory[i].embedding, FOSSIL_AI_JELLYFISH_EMBED_SIZE);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }

    // If memory exists, blend output
    if (model->memory_len > 0) {
        for (size_t i = 0; i < model->output_size; i++) {
            output[i] = (i < FOSSIL_AI_JELLYFISH_EMBED_SIZE) ? model->memory[best_idx].output[i] : 0.0f;
        }
    } else {
        for (size_t i = 0; i < model->output_size; i++) {
            output[i] = (i < model->input_size) ? input[i] : 0.0f;
        }
    }

    // Add to memory
    fossil_ai_jellyfish_add_memory(model, input, output, model->output_size);
    ctx->timestamp = (int64_t)time(NULL);
    return true;
}

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_jellyfish_save_model(const fossil_ai_jellyfish_model_t *model, const char *filepath) {
    if (!model || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    fwrite(model, sizeof(fossil_ai_jellyfish_model_t), 1, f);
    fclose(f);
    return true;
}

fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_load_model(const char *filepath) {
    if (!filepath) return NULL;
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;
    fossil_ai_jellyfish_model_t *model = (fossil_ai_jellyfish_model_t *)calloc(1, sizeof(fossil_ai_jellyfish_model_t));
    if (!model) { fclose(f); return NULL; }
    fread(model, sizeof(fossil_ai_jellyfish_model_t), 1, f);
    fclose(f);
    return model;
}
