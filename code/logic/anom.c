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
#include "fossil/ai/anom.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

// ======================================================
// Initialization / Cleanup
// ======================================================

fossil_ai_anom_t *fossil_ai_anom_create(void) {
    fossil_ai_anom_t *detector = calloc(1, sizeof(fossil_ai_anom_t));
    return detector;
}

void fossil_ai_anom_free(fossil_ai_anom_t *detector) {
    if (!detector) return;
    free(detector);
}

// ======================================================
// Statistical Anomaly Detection (z-score)
// ======================================================

bool fossil_ai_anom_detect_statistical(fossil_ai_anom_t *detector,
                                       const float *data,
                                       size_t len,
                                       float threshold,
                                       float *score) {
    if (!detector || !data || len == 0 || !score) return false;

    // Compute mean
    float mean = 0.0f;
    for (size_t i = 0; i < len; i++)
        mean += data[i];
    mean /= len;

    // Compute standard deviation
    float std = 0.0f;
    for (size_t i = 0; i < len; i++)
        std += (data[i] - mean) * (data[i] - mean);
    std = sqrtf(std / len);

    // Z-score of last element
    float z = (std > 0.0f) ? fabsf(data[len-1] - mean) / std : 0.0f;
    *score = z;

    fossil_ai_anom_record_t rec = { .score = z, .type = FOSSIL_AI_ANOM_STATISTICAL, .timestamp = (int64_t)time(NULL) };
    if (detector->history_len < FOSSIL_AI_ANOM_MAX_HISTORY)
        detector->history[detector->history_len++] = rec;

    return (z > threshold);
}

// ======================================================
// Deviation Detection (simple difference from previous mean)
// ======================================================

bool fossil_ai_anom_detect_deviation(fossil_ai_anom_t *detector,
                                     const float *data,
                                     size_t len,
                                     float *score) {
    if (!detector || !data || len == 0 || !score) return false;

    float prev_mean = 0.0f;
    if (detector->history_len > 0) {
        for (size_t i = 0; i < detector->history_len; i++)
            prev_mean += detector->history[i].score;
        prev_mean /= detector->history_len;
    }

    float deviation = fabsf(data[len-1] - prev_mean);
    *score = deviation;

    fossil_ai_anom_record_t rec = { .score = deviation, .type = FOSSIL_AI_ANOM_DEVIATION, .timestamp = (int64_t)time(NULL) };
    if (detector->history_len < FOSSIL_AI_ANOM_MAX_HISTORY)
        detector->history[detector->history_len++] = rec;

    return (deviation > FOSSIL_AI_ANOM_THRESHOLD);
}

// ======================================================
// Reconstruction-based Detection (Autoencoder)
// ======================================================

bool fossil_ai_anom_detect_reconstruction(fossil_ai_anom_t *detector,
                                          const float *data,
                                          size_t len,
                                          fossil_ai_jellyfish_model_t *autoencoder,
                                          float *score) {
    if (!detector || !data || len == 0 || !autoencoder || !score) return false;

    float reconstructed[FOSSIL_AI_JELLYFISH_EMBED_SIZE] = {0};
    if (!fossil_ai_jellyfish_infer(autoencoder, NULL, data, reconstructed)) return false;

    float mse = 0.0f;
    for (size_t i = 0; i < len; i++)
        mse += (data[i] - reconstructed[i]) * (data[i] - reconstructed[i]);
    mse /= len;

    *score = mse;

    fossil_ai_anom_record_t rec = { .score = mse, .type = FOSSIL_AI_ANOM_RECONSTRUCTION, .timestamp = (int64_t)time(NULL) };
    if (detector->history_len < FOSSIL_AI_ANOM_MAX_HISTORY)
        detector->history[detector->history_len++] = rec;

    return (mse > FOSSIL_AI_ANOM_THRESHOLD);
}

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_anom_save(fossil_ai_anom_t *detector, const char *filepath) {
    if (!detector || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    fwrite(&detector->history_len, sizeof(size_t), 1, f);
    fwrite(detector->history, sizeof(fossil_ai_anom_record_t), detector->history_len, f);
    fclose(f);
    return true;
}

fossil_ai_anom_t *fossil_ai_anom_load(const char *filepath) {
    if (!filepath) return NULL;
    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    fossil_ai_anom_t *detector = fossil_ai_anom_create();
    if (!detector) { fclose(f); return NULL; }

    fread(&detector->history_len, sizeof(size_t), 1, f);
    fread(detector->history, sizeof(fossil_ai_anom_record_t), detector->history_len, f);
    fclose(f);
    return detector;
}
