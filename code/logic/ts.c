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
#include "fossil/ai/ts.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

// ======================================================
// Initialization / Cleanup
// ======================================================

fossil_ai_ts_t *fossil_ai_ts_create(void) {
    fossil_ai_ts_t *ts = calloc(1, sizeof(fossil_ai_ts_t));
    return ts;
}

void fossil_ai_ts_free(fossil_ai_ts_t *ts) {
    if (!ts) return;
    free(ts);
}

// ======================================================
// Series Management
// ======================================================

bool fossil_ai_ts_add_series(fossil_ai_ts_t *ts, const char *name) {
    if (!ts || !name || ts->series_len >= FOSSIL_AI_TS_MAX_SERIES) return false;
    strncpy(ts->series[ts->series_len].name, name, sizeof(ts->series[0].name)-1);
    ts->series[ts->series_len].length = 0;
    ts->series_len++;
    return true;
}

bool fossil_ai_ts_add_point(fossil_ai_ts_t *ts, const char *series_name, float value, int64_t timestamp) {
    if (!ts || !series_name) return false;
    for (size_t i = 0; i < ts->series_len; i++) {
        if (strcmp(ts->series[i].name, series_name) == 0) {
            if (ts->series[i].length >= FOSSIL_AI_TS_MAX_POINTS) return false;
            ts->series[i].points[ts->series[i].length].value = value;
            ts->series[i].points[ts->series[i].length].timestamp = timestamp;
            ts->series[i].length++;
            return true;
        }
    }
    return false;
}

// ======================================================
// Forecasting (simple linear regression per series)
// ======================================================

bool fossil_ai_ts_forecast(fossil_ai_ts_t *ts,
                           const char *series_name,
                           size_t horizon,
                           float *forecast_output) {
    if (!ts || !series_name || !forecast_output) return false;

    for (size_t i = 0; i < ts->series_len; i++) {
        if (strcmp(ts->series[i].name, series_name) == 0) {
            size_t n = ts->series[i].length;
            if (n < 2) return false;

            float slope, intercept;
            fossil_ai_ts_trend(ts, series_name, &slope, &intercept);

            int64_t last_timestamp = ts->series[i].points[n-1].timestamp;
            int64_t delta = (n > 1) ? ts->series[i].points[n-1].timestamp - ts->series[i].points[n-2].timestamp : 1;

            for (size_t h = 0; h < horizon; h++) {
                int64_t t = last_timestamp + (h+1)*delta;
                forecast_output[h] = intercept + slope * t;
            }
            return true;
        }
    }
    return false;
}

// ======================================================
// Trend Detection (linear regression slope)
// ======================================================

bool fossil_ai_ts_trend(fossil_ai_ts_t *ts,
                        const char *series_name,
                        float *slope,
                        float *intercept) {
    if (!ts || !series_name || !slope || !intercept) return false;

    for (size_t i = 0; i < ts->series_len; i++) {
        if (strcmp(ts->series[i].name, series_name) == 0) {
            size_t n = ts->series[i].length;
            if (n < 2) return false;

            float sum_x=0, sum_y=0, sum_xy=0, sum_x2=0;
            for (size_t j=0; j<n; j++) {
                float x = (float)ts->series[i].points[j].timestamp;
                float y = ts->series[i].points[j].value;
                sum_x += x;
                sum_y += y;
                sum_xy += x*y;
                sum_x2 += x*x;
            }

            float denom = n*sum_x2 - sum_x*sum_x;
            if (denom == 0) return false;

            *slope = (n*sum_xy - sum_x*sum_y) / denom;
            *intercept = (sum_y - (*slope)*sum_x) / n;
            return true;
        }
    }
    return false;
}

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_ts_save(fossil_ai_ts_t *ts, const char *filepath) {
    if (!ts || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    fwrite(&ts->series_len, sizeof(size_t), 1, f);
    fwrite(ts->series, sizeof(fossil_ai_ts_series_t), ts->series_len, f);

    fclose(f);
    return true;
}

fossil_ai_ts_t *fossil_ai_ts_load(const char *filepath) {
    if (!filepath) return NULL;

    FILE *f = fopen(filepath, "rb");
    if (!f) return NULL;

    fossil_ai_ts_t *ts = fossil_ai_ts_create();
    if (!ts) {
        fclose(f);
        return NULL;
    }

    /* Read series_len */
    if (fread(&ts->series_len, sizeof(size_t), 1, f) != 1) {
        fclose(f);
        fossil_ai_ts_free(ts);
        return NULL;
    }

    /* Validate series_len */
    if (ts->series_len > FOSSIL_AI_TS_MAX_SERIES) {
        fclose(f);
        fossil_ai_ts_free(ts);
        return NULL;
    }

    /* Read series data */
    if (ts->series_len > 0) {
        size_t read_count = fread(
            ts->series,
            sizeof(fossil_ai_ts_series_t),
            ts->series_len,
            f
        );

        if (read_count != ts->series_len) {
            fclose(f);
            fossil_ai_ts_free(ts);
            return NULL;
        }
    }

    fclose(f);
    return ts;
}
