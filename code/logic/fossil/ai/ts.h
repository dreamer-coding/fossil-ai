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
#ifndef FOSSIL_JELLYFISH_TS_H
#define FOSSIL_JELLYFISH_TS_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_TS_MAX_SERIES 1024
#define FOSSIL_AI_TS_MAX_POINTS 1024

// ======================================================
// Time Series Data Point
// ======================================================

typedef struct {
    float value;
    int64_t timestamp;
} fossil_ai_ts_point_t;

// ======================================================
// Time Series Series
// ======================================================

typedef struct {
    char name[64];
    fossil_ai_ts_point_t points[FOSSIL_AI_TS_MAX_POINTS];
    size_t length;
} fossil_ai_ts_series_t;

// ======================================================
// Time Series Manager
// ======================================================

typedef struct {
    fossil_ai_ts_series_t series[FOSSIL_AI_TS_MAX_SERIES];
    size_t series_len;
} fossil_ai_ts_t;

// ======================================================
// Initialization / Cleanup
// ======================================================

fossil_ai_ts_t *fossil_ai_ts_create(void);
void fossil_ai_ts_free(fossil_ai_ts_t *ts);

// ======================================================
// Series Management
// ======================================================

bool fossil_ai_ts_add_series(fossil_ai_ts_t *ts, const char *name);
bool fossil_ai_ts_add_point(fossil_ai_ts_t *ts, const char *series_name, float value, int64_t timestamp);

// ======================================================
// Forecasting
// ======================================================

bool fossil_ai_ts_forecast(fossil_ai_ts_t *ts,
                           const char *series_name,
                           size_t horizon,
                           float *forecast_output);

// ======================================================
// Trend Detection
// ======================================================

bool fossil_ai_ts_trend(fossil_ai_ts_t *ts,
                        const char *series_name,
                        float *slope,
                        float *intercept);

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_ts_save(fossil_ai_ts_t *ts, const char *filepath);
fossil_ai_ts_t *fossil_ai_ts_load(const char *filepath);

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
