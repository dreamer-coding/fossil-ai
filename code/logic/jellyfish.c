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
#include <stdint.h>

#define FOSSIL_AI_JELLYFISH_MAGIC 0x4A454C59 // 'JELY'
#define FOSSIL_AI_JELLYFISH_VERSION 1


// ======================================================
// Helpers — Vector math
// ======================================================

static void fossil_ai_jellyfish_copy(float *dst, const float *src, size_t len) {
    for (size_t i = 0; i < len; i++) dst[i] = src[i];
}

static float fossil_ai_jellyfish_l2_distance(const float *a, const float *b, size_t len) {
    float sum = 0.0f;
    for (size_t i = 0; i < len; i++) {
        float d = a[i] - b[i];
        sum += d * d;
    }
    return sqrtf(sum);
}

static void fossil_ai_jellyfish_softmax(const float *scores, float *out, size_t len) {
    float max = scores[0];
    for (size_t i = 1; i < len; i++) if (scores[i] > max) max = scores[i];
    float sum = 0.0f;
    for (size_t i = 0; i < len; i++) {
        out[i] = expf(scores[i] - max);
        sum += out[i];
    }
    for (size_t i = 0; i < len; i++) out[i] /= sum;
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

    model->internal_state = calloc(input_size * output_size, sizeof(float)); // linear weights
    if (!model->internal_state) { free(model); return NULL; }

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
// Training
// ======================================================


// Fisher-Yates shuffle for epochs
static void shuffle_indices(size_t *indices, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        size_t tmp = indices[i];
        indices[i] = indices[j];
        indices[j] = tmp;
    }
}

bool fossil_ai_jellyfish_train(fossil_ai_jellyfish_model_t *model,
                               const float *inputs,
                               const float *targets,
                               size_t count) {
    if (!model || !inputs || !targets || count == 0) return false;

    float *weights = (float *)model->internal_state;
    size_t in_size = model->input_size;
    size_t out_size = model->output_size;
    float lr = 0.01f;
    float lambda = 0.001f; // L2 regularization

    float *y_hat = (float *)calloc(model->output_size, sizeof(float));
    if (!y_hat) return false;

    size_t *indices = (size_t *)malloc(count * sizeof(size_t));
    if (!indices) { free(y_hat); return false; }
    for (size_t n = 0; n < count; n++) indices[n] = n;

    for (size_t epoch = 0; epoch < 20; epoch++) {
        // Shuffle samples each epoch
        shuffle_indices(indices, count);

        // Learning rate decay
        float epoch_lr = lr / (1.0f + 0.05f * epoch);

        for (size_t idx = 0; idx < count; idx++) {
            size_t n = indices[idx];
            const float *x = inputs + n * in_size;
            const float *y = targets + n * out_size;

            // Forward pass
            for (size_t j = 0; j < model->output_size; j++) {
                y_hat[j] = 0.0f;
                for (size_t i = 0; i < model->input_size; i++) {
                    y_hat[j] += weights[j * model->input_size + i] * x[i];
                }
            }

            // Gradient descent with L2 regularization
            for (size_t j = 0; j < out_size; j++) {
                float error = y_hat[j] - y[j];
                for (size_t i = 0; i < in_size; i++) {
                    weights[j * in_size + i] -= epoch_lr * (error * x[i] + lambda * weights[j * in_size + i]);
                }
            }
        }
    }

    free(y_hat);
    free(indices);
    return true;
}

// ======================================================
// Memory — attention weighted retrieval
// ======================================================

bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t *model,
                                    const float *input,
                                    const float *output,
                                    size_t embed_len) {
    if (!model || !input || !output) return false;
    // Ensure embed_len does not exceed the embedding/output array size or model's output size
    size_t safe_len = embed_len;
    if (safe_len > FOSSIL_AI_JELLYFISH_EMBED_SIZE) safe_len = FOSSIL_AI_JELLYFISH_EMBED_SIZE;
    if (model->output_size < safe_len) safe_len = model->output_size;
    size_t idx = model->memory_len % FOSSIL_AI_JELLYFISH_MAX_MEMORY;
    fossil_ai_jellyfish_copy(model->memory[idx].embedding, input, safe_len);
    fossil_ai_jellyfish_copy(model->memory[idx].output, output, safe_len);
    model->memory[idx].timestamp = (int64_t)time(NULL);
    if (model->memory_len < FOSSIL_AI_JELLYFISH_MAX_MEMORY) model->memory_len++;
    return true;
}

// ======================================================
// Inference — linear model + attention
// ======================================================

bool fossil_ai_jellyfish_infer(fossil_ai_jellyfish_model_t *model,
                               fossil_ai_jellyfish_context_t *ctx,
                               const float *input,
                               float *output) {
    if (!model || !input || !output) return false;
    size_t in_size = model->input_size;
    size_t out_size = model->output_size;
    float *weights = (float *)model->internal_state;

    // 1. Linear model
    for (size_t j = 0; j < out_size; j++) {
        output[j] = 0.0f;
        for (size_t i = 0; i < in_size; i++) {
            output[j] += weights[j * in_size + i] * input[i];
        }
    }

    // 2. Attention memory blending
    if (model->memory_len > 0) {
        size_t mem_vec_len = out_size < FOSSIL_AI_JELLYFISH_EMBED_SIZE ? out_size : FOSSIL_AI_JELLYFISH_EMBED_SIZE;
        float scores[FOSSIL_AI_JELLYFISH_MAX_MEMORY] = {0};
        for (size_t m = 0; m < model->memory_len; m++) {
            scores[m] = -fossil_ai_jellyfish_l2_distance(input, model->memory[m].embedding, mem_vec_len);
        }
        float attn[FOSSIL_AI_JELLYFISH_MAX_MEMORY];
        fossil_ai_jellyfish_softmax(scores, attn, model->memory_len);

        // blend memory
        for (size_t j = 0; j < out_size; j++) {
            float blend = 0.0f;
            if (j < mem_vec_len) {
                for (size_t m = 0; m < model->memory_len; m++) {
                    blend += model->memory[m].output[j] * attn[m];
                }
                output[j] = 0.5f * output[j] + 0.5f * blend; // blend linear + memory
            }
            // If j >= mem_vec_len, leave output[j] as is (from linear model)
        }
    }

    fossil_ai_jellyfish_add_memory(model, input, output, out_size);
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

    // Header
    uint32_t magic = FOSSIL_AI_JELLYFISH_MAGIC;
    uint32_t version = FOSSIL_AI_JELLYFISH_VERSION;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&version, sizeof(version), 1, f);

    // Core metadata
    fwrite(&model->input_size, sizeof(model->input_size), 1, f);
    fwrite(&model->output_size, sizeof(model->output_size), 1, f);
    fwrite(model->name, sizeof(model->name), 1, f);
    fwrite(&model->memory_len, sizeof(model->memory_len), 1, f);

    // Memory
    size_t mem_len = model->memory_len;
    if (mem_len > FOSSIL_AI_JELLYFISH_MAX_MEMORY) mem_len = FOSSIL_AI_JELLYFISH_MAX_MEMORY;
    fwrite(model->memory, sizeof(fossil_ai_jellyfish_memory_t), mem_len, f);

    // Internal state (weights)
    size_t weight_count = model->input_size * model->output_size;
    fwrite(model->internal_state, sizeof(float), weight_count, f);

    fclose(f);
    return true;
}

fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_load_model(const char *filepath) {
    if (!filepath) return NULL;
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    uint32_t magic = 0, version = 0;
    if (fread(&magic, sizeof(magic), 1, f) != 1) { fclose(f); return NULL; }
    if (fread(&version, sizeof(version), 1, f) != 1) { fclose(f); return NULL; }
    if (magic != FOSSIL_AI_JELLYFISH_MAGIC || version != FOSSIL_AI_JELLYFISH_VERSION) {
        fclose(f);
        return NULL; // invalid file
    }

    fossil_ai_jellyfish_model_t *model = (fossil_ai_jellyfish_model_t *)calloc(1, sizeof(fossil_ai_jellyfish_model_t));
    if (!model) { fclose(f); return NULL; }

    if (fread(&model->input_size, sizeof(model->input_size), 1, f) != 1) { free(model); fclose(f); return NULL; }
    if (fread(&model->output_size, sizeof(model->output_size), 1, f) != 1) { free(model); fclose(f); return NULL; }
    if (fread(model->name, sizeof(model->name), 1, f) != 1) { free(model); fclose(f); return NULL; }
    if (fread(&model->memory_len, sizeof(model->memory_len), 1, f) != 1) { free(model); fclose(f); return NULL; }

    // Memory
    size_t mem_len = model->memory_len;
    if (mem_len > FOSSIL_AI_JELLYFISH_MAX_MEMORY) mem_len = FOSSIL_AI_JELLYFISH_MAX_MEMORY;
    if (fread(model->memory, sizeof(fossil_ai_jellyfish_memory_t), mem_len, f) != mem_len) { free(model); fclose(f); return NULL; }
    model->memory_len = mem_len;

    // Internal state
    size_t weight_count = model->input_size * model->output_size;
    model->internal_state = calloc(weight_count, sizeof(float));
    if (!model->internal_state) { free(model); fclose(f); return NULL; }
    if (fread(model->internal_state, sizeof(float), weight_count, f) != weight_count) { free(model->internal_state); free(model); fclose(f); return NULL; }

    fclose(f);
    return model;
}
