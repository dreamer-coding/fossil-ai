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
#ifndef FOSSIL_JELLYFISH_ANOM_H
#define FOSSIL_JELLYFISH_ANOM_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_ANOM_MAX_HISTORY 1024
#define FOSSIL_AI_ANOM_THRESHOLD   3.0f // z-score threshold

// ======================================================
// Anomaly Types
// ======================================================

typedef enum {
    FOSSIL_AI_ANOM_NONE = 0,
    FOSSIL_AI_ANOM_STATISTICAL,
    FOSSIL_AI_ANOM_RECONSTRUCTION,
    FOSSIL_AI_ANOM_DEVIATION
} fossil_ai_anom_type_t;

// ======================================================
// Anomaly History Record
// ======================================================

typedef struct {
    float score;
    fossil_ai_anom_type_t type;
    int64_t timestamp;
} fossil_ai_anom_record_t;

// ======================================================
// Anomaly Detector
// ======================================================

typedef struct {
    fossil_ai_anom_record_t history[FOSSIL_AI_ANOM_MAX_HISTORY];
    size_t history_len;
} fossil_ai_anom_t;

// ======================================================
// Initialization
// ======================================================

fossil_ai_anom_t *fossil_ai_anom_create(void);
void fossil_ai_anom_free(fossil_ai_anom_t *detector);

// ======================================================
// Detection Methods
// ======================================================

bool fossil_ai_anom_detect_statistical(fossil_ai_anom_t *detector,
                                       const float *data,
                                       size_t len,
                                       float threshold,
                                       float *score);

bool fossil_ai_anom_detect_deviation(fossil_ai_anom_t *detector,
                                     const float *data,
                                     size_t len,
                                     float *score);

bool fossil_ai_anom_detect_reconstruction(fossil_ai_anom_t *detector,
                                          const float *data,
                                          size_t len,
                                          fossil_ai_jellyfish_model_t *autoencoder,
                                          float *score);

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_anom_save(fossil_ai_anom_t *detector, const char *filepath);
fossil_ai_anom_t *fossil_ai_anom_load(const char *filepath);

#ifdef __cplusplus
}

#include <vector>
#include <memory>

namespace fossil {

    namespace ai {



    } // namespace ai

} // namespace fossil

#endif

#endif /* AI_FRAMEWORK_H */
