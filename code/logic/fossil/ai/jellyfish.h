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
#ifndef FOSSIL_JELLYFISH_AI_H
#define FOSSIL_JELLYFISH_AI_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Versioning
 * ============================================================ */

#define FOSSIL_AI_JELLYFISH_VERSION_MAJOR 0
#define FOSSIL_AI_JELLYFISH_VERSION_MINOR 3
#define FOSSIL_AI_JELLYFISH_VERSION_PATCH 0

const char *fossil_ai_jellyfish_version_string(void);

/* ============================================================
 * Forward Declarations (Opaque Types)
 * ============================================================ */

typedef struct fossil_ai_jellyfish_core_t     fossil_ai_jellyfish_core_t;
typedef struct fossil_ai_jellyfish_model_t    fossil_ai_jellyfish_model_t;
typedef struct fossil_ai_jellyfish_context_t  fossil_ai_jellyfish_context_t;
typedef struct fossil_ai_jellyfish_backend_t  fossil_ai_jellyfish_backend_t;
typedef struct fossil_ai_jellyfish_hardware_t fossil_ai_jellyfish_hardware_t;
typedef struct fossil_ai_jellyfish_train_t    fossil_ai_jellyfish_train_t;

/* ============================================================
 * Error Handling
 * ============================================================ */

typedef int32_t fossil_ai_jellyfish_status_t;

#define FOSSIL_AI_JELLYFISH_OK            0
#define FOSSIL_AI_JELLYFISH_ERR_GENERIC  -1
#define FOSSIL_AI_JELLYFISH_ERR_NOMEM    -2
#define FOSSIL_AI_JELLYFISH_ERR_IO       -3
#define FOSSIL_AI_JELLYFISH_ERR_INVALID  -4
#define FOSSIL_AI_JELLYFISH_ERR_UNSUP    -5

const char *fossil_ai_jellyfish_status_string(fossil_ai_jellyfish_status_t status);

/* ============================================================
 * Core Lifecycle
 * ============================================================ */

fossil_ai_jellyfish_core_t *fossil_ai_jellyfish_core_create(void);

void fossil_ai_jellyfish_core_destroy(fossil_ai_jellyfish_core_t *core);

/* ============================================================
 * Hardware Awareness
 * ============================================================ */

fossil_ai_jellyfish_hardware_t *fossil_ai_jellyfish_hardware_detect(void);

void fossil_ai_jellyfish_hardware_destroy(fossil_ai_jellyfish_hardware_t *hw);

bool fossil_ai_jellyfish_hardware_has_cpu(const fossil_ai_jellyfish_hardware_t *hw);

bool fossil_ai_jellyfish_hardware_has_gpu(const fossil_ai_jellyfish_hardware_t *hw);

const char *fossil_ai_jellyfish_hardware_summary(const fossil_ai_jellyfish_hardware_t *hw);

/* ============================================================
 * Backend Management (CPU / GPU / Custom)
 * ============================================================ */

fossil_ai_jellyfish_backend_t *fossil_ai_jellyfish_backend_create(const char *backend_id);

void fossil_ai_jellyfish_backend_destroy(fossil_ai_jellyfish_backend_t *backend);

fossil_ai_jellyfish_status_t fossil_ai_jellyfish_core_attach_backend(
    fossil_ai_jellyfish_core_t *core,
    fossil_ai_jellyfish_backend_t *backend
);

fossil_ai_jellyfish_backend_t *fossil_ai_jellyfish_core_select_backend(
    fossil_ai_jellyfish_core_t *core,
    const char *backend_id /* NULL = auto */
);

/* ============================================================
 * Context & Time Awareness
 * ============================================================ */

fossil_ai_jellyfish_context_t *fossil_ai_jellyfish_context_create(const char *context_id);

void fossil_ai_jellyfish_context_destroy(fossil_ai_jellyfish_context_t *ctx);

void fossil_ai_jellyfish_context_set_timestamp(
    fossil_ai_jellyfish_context_t *ctx,
    int64_t unix_time_ns
);

void fossil_ai_jellyfish_context_set_kv(
    fossil_ai_jellyfish_context_t *ctx,
    const char *key,
    const char *value
);

const char *fossil_ai_jellyfish_context_get_kv(
    const fossil_ai_jellyfish_context_t *ctx,
    const char *key
);

/* ============================================================
 * Model Management (Persistent)
 * ============================================================ */

fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_model_create(
    const char *model_id,
    const char *architecture_id
);

void fossil_ai_jellyfish_model_destroy(fossil_ai_jellyfish_model_t *model);

fossil_ai_jellyfish_status_t fossil_ai_jellyfish_model_save(
    const fossil_ai_jellyfish_model_t *model,
    const char *path
);

fossil_ai_jellyfish_model_t *
fossil_ai_jellyfish_model_load(const char *path);

/* ============================================================
 * Inference
 * ============================================================ */

fossil_ai_jellyfish_status_t fossil_ai_jellyfish_model_run(
    fossil_ai_jellyfish_model_t *model,
    fossil_ai_jellyfish_backend_t *backend,
    fossil_ai_jellyfish_context_t *ctx,
    const void *input,
    size_t input_size,
    void *output,
    size_t output_size
);

/* ============================================================
 * Training
 * ============================================================ */

fossil_ai_jellyfish_train_t *fossil_ai_jellyfish_train_create(
    fossil_ai_jellyfish_model_t *model,
    const char *trainer_id
);

void fossil_ai_jellyfish_train_destroy(fossil_ai_jellyfish_train_t *train);

fossil_ai_jellyfish_status_t fossil_ai_jellyfish_train_step(
    fossil_ai_jellyfish_train_t *train,
    const void *input,
    size_t input_size,
    const void *expected,
    size_t expected_size
);

fossil_ai_jellyfish_status_t fossil_ai_jellyfish_train_finalize(
    fossil_ai_jellyfish_train_t *train
);

/* ============================================================
 * Introspection (Cold, Explicit)
 * ============================================================ */

const char *fossil_ai_jellyfish_model_id(
    const fossil_ai_jellyfish_model_t *model
);

const char *fossil_ai_jellyfish_model_architecture(
    const fossil_ai_jellyfish_model_t *model
);

uint64_t
fossil_ai_jellyfish_model_parameter_count(
    const fossil_ai_jellyfish_model_t *model
);


#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>

namespace fossil {

    namespace ai {



    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
