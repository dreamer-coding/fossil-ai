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
#include "jellyfish.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* ============================================================
 * Internal Structures
 * ============================================================ */

struct fossil_ai_jellyfish_core_t {
    fossil_ai_jellyfish_backend_t *backend;
};

struct fossil_ai_jellyfish_backend_t {
    char *backend_id;
};

struct fossil_ai_jellyfish_hardware_t {
    bool has_cpu;
    bool has_gpu;
};

struct fossil_ai_jellyfish_context_t {
    char *context_id;
    int64_t timestamp_ns;

    /* simple kv store (flat, cold, dumb) */
    char **keys;
    char **values;
    size_t count;
};

struct fossil_ai_jellyfish_model_t {
    char *model_id;
    char *architecture_id;

    void  *parameters;
    size_t parameter_size;
};

struct fossil_ai_jellyfish_train_t {
    fossil_ai_jellyfish_model_t *model;
    char *trainer_id;
};

/* ============================================================
 * Versioning
 * ============================================================ */

const char *
fossil_ai_jellyfish_version_string(void) {
    return "jellyfish 0.1.0";
}

/* ============================================================
 * Status Strings
 * ============================================================ */

const char *
fossil_ai_jellyfish_status_string(fossil_ai_jellyfish_status_t status) {
    switch (status) {
        case FOSSIL_AI_JELLYFISH_OK:           return "OK";
        case FOSSIL_AI_JELLYFISH_ERR_GENERIC:  return "Generic error";
        case FOSSIL_AI_JELLYFISH_ERR_NOMEM:    return "Out of memory";
        case FOSSIL_AI_JELLYFISH_ERR_IO:       return "I/O error";
        case FOSSIL_AI_JELLYFISH_ERR_INVALID:  return "Invalid argument";
        case FOSSIL_AI_JELLYFISH_ERR_UNSUP:    return "Unsupported";
        default:                              return "Unknown error";
    }
}

/* ============================================================
 * Core Lifecycle
 * ============================================================ */

fossil_ai_jellyfish_core_t *
fossil_ai_jellyfish_core_create(void) {
    return calloc(1, sizeof(fossil_ai_jellyfish_core_t));
}

void
fossil_ai_jellyfish_core_destroy(fossil_ai_jellyfish_core_t *core) {
    free(core);
}

/* ============================================================
 * Hardware Awareness
 * ============================================================ */

fossil_ai_jellyfish_hardware_t *
fossil_ai_jellyfish_hardware_detect(void) {
    fossil_ai_jellyfish_hardware_t *hw = calloc(1, sizeof(*hw));
    if (!hw) return NULL;

    hw->has_cpu = true;
    hw->has_gpu = false; /* stub — GPU probing lives elsewhere */

    return hw;
}

void
fossil_ai_jellyfish_hardware_destroy(fossil_ai_jellyfish_hardware_t *hw) {
    free(hw);
}

bool
fossil_ai_jellyfish_hardware_has_cpu(const fossil_ai_jellyfish_hardware_t *hw) {
    return hw && hw->has_cpu;
}

bool
fossil_ai_jellyfish_hardware_has_gpu(const fossil_ai_jellyfish_hardware_t *hw) {
    return hw && hw->has_gpu;
}

const char *
fossil_ai_jellyfish_hardware_summary(const fossil_ai_jellyfish_hardware_t *hw) {
    if (!hw) return "no hardware";

    if (hw->has_cpu && hw->has_gpu) return "cpu+gpu";
    if (hw->has_cpu)               return "cpu";
    if (hw->has_gpu)               return "gpu";
    return "none";
}

/* ============================================================
 * Backend Management
 * ============================================================ */

fossil_ai_jellyfish_backend_t *
fossil_ai_jellyfish_backend_create(const char *backend_id) {
    if (!backend_id) return NULL;

    fossil_ai_jellyfish_backend_t *b = calloc(1, sizeof(*b));
    if (!b) return NULL;

    b->backend_id = strdup(backend_id);
    return b;
}

void
fossil_ai_jellyfish_backend_destroy(fossil_ai_jellyfish_backend_t *backend) {
    if (!backend) return;
    free(backend->backend_id);
    free(backend);
}

fossil_ai_jellyfish_status_t
fossil_ai_jellyfish_core_attach_backend(
    fossil_ai_jellyfish_core_t *core,
    fossil_ai_jellyfish_backend_t *backend
) {
    if (!core || !backend) return FOSSIL_AI_JELLYFISH_ERR_INVALID;
    core->backend = backend;
    return FOSSIL_AI_JELLYFISH_OK;
}

fossil_ai_jellyfish_backend_t *
fossil_ai_jellyfish_core_select_backend(
    fossil_ai_jellyfish_core_t *core,
    const char *backend_id
) {
    (void)backend_id;
    return core ? core->backend : NULL;
}

/* ============================================================
 * Context & Time
 * ============================================================ */

fossil_ai_jellyfish_context_t *
fossil_ai_jellyfish_context_create(const char *context_id) {
    fossil_ai_jellyfish_context_t *ctx = calloc(1, sizeof(*ctx));
    if (!ctx) return NULL;

    ctx->context_id = context_id ? strdup(context_id) : NULL;
    return ctx;
}

void
fossil_ai_jellyfish_context_destroy(fossil_ai_jellyfish_context_t *ctx) {
    if (!ctx) return;

    for (size_t i = 0; i < ctx->count; ++i) {
        free(ctx->keys[i]);
        free(ctx->values[i]);
    }
    free(ctx->keys);
    free(ctx->values);
    free(ctx->context_id);
    free(ctx);
}

void
fossil_ai_jellyfish_context_set_timestamp(
    fossil_ai_jellyfish_context_t *ctx,
    int64_t unix_time_ns
) {
    if (ctx) ctx->timestamp_ns = unix_time_ns;
}

void
fossil_ai_jellyfish_context_set_kv(
    fossil_ai_jellyfish_context_t *ctx,
    const char *key,
    const char *value
) {
    if (!ctx || !key || !value) return;

    ctx->keys   = realloc(ctx->keys,   sizeof(char*) * (ctx->count + 1));
    ctx->values = realloc(ctx->values, sizeof(char*) * (ctx->count + 1));

    ctx->keys[ctx->count]   = strdup(key);
    ctx->values[ctx->count] = strdup(value);
    ctx->count++;
}

const char *
fossil_ai_jellyfish_context_get_kv(
    const fossil_ai_jellyfish_context_t *ctx,
    const char *key
) {
    if (!ctx || !key) return NULL;

    for (size_t i = 0; i < ctx->count; ++i) {
        if (strcmp(ctx->keys[i], key) == 0)
            return ctx->values[i];
    }
    return NULL;
}

/* ============================================================
 * Model Management
 * ============================================================ */

fossil_ai_jellyfish_model_t *
fossil_ai_jellyfish_model_create(
    const char *model_id,
    const char *architecture_id
) {
    fossil_ai_jellyfish_model_t *m = calloc(1, sizeof(*m));
    if (!m) return NULL;

    m->model_id = strdup(model_id);
    m->architecture_id = strdup(architecture_id);
    return m;
}

void
fossil_ai_jellyfish_model_destroy(fossil_ai_jellyfish_model_t *model) {
    if (!model) return;
    free(model->model_id);
    free(model->architecture_id);
    free(model->parameters);
    free(model);
}

fossil_ai_jellyfish_status_t
fossil_ai_jellyfish_model_save(
    const fossil_ai_jellyfish_model_t *model,
    const char *path
) {
    if (!model || !path) return FOSSIL_AI_JELLYFISH_ERR_INVALID;

    FILE *f = fopen(path, "wb");
    if (!f) return FOSSIL_AI_JELLYFISH_ERR_IO;

    fwrite(&model->parameter_size, sizeof(size_t), 1, f);
    fwrite(model->parameters, 1, model->parameter_size, f);
    fclose(f);

    return FOSSIL_AI_JELLYFISH_OK;
}

fossil_ai_jellyfish_model_t *
fossil_ai_jellyfish_model_load(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fossil_ai_jellyfish_model_t *m = calloc(1, sizeof(*m));
    fread(&m->parameter_size, sizeof(size_t), 1, f);

    m->parameters = malloc(m->parameter_size);
    fread(m->parameters, 1, m->parameter_size, f);
    fclose(f);

    return m;
}

/* ============================================================
 * Inference (Stub)
 * ============================================================ */

fossil_ai_jellyfish_status_t
fossil_ai_jellyfish_model_run(
    fossil_ai_jellyfish_model_t *model,
    fossil_ai_jellyfish_backend_t *backend,
    fossil_ai_jellyfish_context_t *ctx,
    const void *input,
    size_t input_size,
    void *output,
    size_t output_size
) {
    (void)model;
    (void)backend;
    (void)ctx;

    /* cold stub: echo input to output */
    size_t n = input_size < output_size ? input_size : output_size;
    memcpy(output, input, n);

    return FOSSIL_AI_JELLYFISH_OK;
}

/* ============================================================
 * Training (Stub)
 * ============================================================ */

fossil_ai_jellyfish_train_t *
fossil_ai_jellyfish_train_create(
    fossil_ai_jellyfish_model_t *model,
    const char *trainer_id
) {
    fossil_ai_jellyfish_train_t *t = calloc(1, sizeof(*t));
    if (!t) return NULL;

    t->model = model;
    t->trainer_id = strdup(trainer_id);
    return t;
}

void
fossil_ai_jellyfish_train_destroy(fossil_ai_jellyfish_train_t *train) {
    if (!train) return;
    free(train->trainer_id);
    free(train);
}

fossil_ai_jellyfish_status_t
fossil_ai_jellyfish_train_step(
    fossil_ai_jellyfish_train_t *train,
    const void *input,
    size_t input_size,
    const void *expected,
    size_t expected_size
) {
    (void)train;
    (void)input;
    (void)input_size;
    (void)expected;
    (void)expected_size;

    return FOSSIL_AI_JELLYFISH_OK;
}

fossil_ai_jellyfish_status_t
fossil_ai_jellyfish_train_finalize(
    fossil_ai_jellyfish_train_t *train
) {
    (void)train;
    return FOSSIL_AI_JELLYFISH_OK;
}

/* ============================================================
 * Introspection
 * ============================================================ */

const char *
fossil_ai_jellyfish_model_id(
    const fossil_ai_jellyfish_model_t *model
) {
    return model ? model->model_id : NULL;
}

const char *
fossil_ai_jellyfish_model_architecture(
    const fossil_ai_jellyfish_model_t *model
) {
    return model ? model->architecture_id : NULL;
}

uint64_t
fossil_ai_jellyfish_model_parameter_count(
    const fossil_ai_jellyfish_model_t *model
) {
    return model ? (uint64_t)model->parameter_size : 0;
}
