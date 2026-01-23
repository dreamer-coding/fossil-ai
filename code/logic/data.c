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
#include "fossil/ai/data.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ======================================================
 * Tensor Utilities
 * ====================================================== */

bool fossil_ai_data_tensor_init(fossil_ai_data_tensor_t *tensor,
                                size_t rows,
                                size_t cols)
{
    if (!tensor || rows == 0 || cols == 0)
        return false;

    tensor->rows = rows;
    tensor->cols = cols;
    tensor->stride = cols;
    tensor->data = (float *)calloc(rows * cols, sizeof(float));

    return tensor->data != NULL;
}

void fossil_ai_data_tensor_free(fossil_ai_data_tensor_t *tensor)
{
    if (!tensor)
        return;

    free(tensor->data);
    memset(tensor, 0, sizeof(*tensor));
}

float fossil_ai_data_tensor_get(const fossil_ai_data_tensor_t *tensor,
                                size_t row,
                                size_t col)
{
    return tensor->data[row * tensor->stride + col];
}

void fossil_ai_data_tensor_set(fossil_ai_data_tensor_t *tensor,
                               size_t row,
                               size_t col,
                               float value)
{
    tensor->data[row * tensor->stride + col] = value;
}

/* ======================================================
 * Dataset Management
 * ====================================================== */

bool fossil_ai_data_dataset_init(fossil_ai_data_dataset_t *ds,
                                 size_t samples,
                                 size_t input_size,
                                 size_t target_size)
{
    if (!ds)
        return false;

    ds->sample_count = samples;

    if (!fossil_ai_data_tensor_init(&ds->inputs, samples, input_size))
        return false;

    if (!fossil_ai_data_tensor_init(&ds->targets, samples, target_size))
        return false;

    return true;
}

void fossil_ai_data_dataset_free(fossil_ai_data_dataset_t *ds)
{
    if (!ds)
        return;

    fossil_ai_data_tensor_free(&ds->inputs);
    fossil_ai_data_tensor_free(&ds->targets);
    memset(ds, 0, sizeof(*ds));
}

/* ======================================================
 * Statistics
 * ====================================================== */

bool fossil_ai_data_compute_stat(const char *stat_id,
                                 const float *data,
                                 size_t count,
                                 float *out_value)
{
    if (!stat_id || !data || count == 0 || !out_value)
        return false;

    if (strcmp(stat_id, FOSSIL_AI_DATA_ID_MEAN) == 0)
    {
        float sum = 0.0f;
        for (size_t i = 0; i < count; ++i)
            sum += data[i];
        *out_value = sum / (float)count;
        return true;
    }

    if (strcmp(stat_id, FOSSIL_AI_DATA_ID_VARIANCE) == 0 ||
        strcmp(stat_id, FOSSIL_AI_DATA_ID_STDDEV) == 0)
    {

        float mean = 0.0f;
        fossil_ai_data_compute_stat(FOSSIL_AI_DATA_ID_MEAN, data, count, &mean);

        float var = 0.0f;
        for (size_t i = 0; i < count; ++i)
        {
            float d = data[i] - mean;
            var += d * d;
        }

        var /= (float)count;

        if (strcmp(stat_id, FOSSIL_AI_DATA_ID_STDDEV) == 0)
            *out_value = sqrtf(var);
        else
            *out_value = var;

        return true;
    }

    return false;
}

/* ======================================================
 * Transforms / Normalization
 * ====================================================== */

bool fossil_ai_data_apply_transform(const char *transform_id,
                                    float *data,
                                    size_t count)
{
    if (!transform_id || !data || count == 0)
        return false;

    if (strcmp(transform_id, FOSSIL_AI_DATA_ID_MINMAX) == 0)
    {
        float min = data[0];
        float max = data[0];

        for (size_t i = 1; i < count; ++i)
        {
            if (data[i] < min)
                min = data[i];
            if (data[i] > max)
                max = data[i];
        }

        float range = max - min;
        if (range == 0.0f)
            return false;

        for (size_t i = 0; i < count; ++i)
            data[i] = (data[i] - min) / range;

        return true;
    }

    if (strcmp(transform_id, FOSSIL_AI_DATA_ID_STANDARD) == 0)
    {
        float mean = 0.0f;
        float std = 0.0f;

        fossil_ai_data_compute_stat(FOSSIL_AI_DATA_ID_MEAN, data, count, &mean);
        fossil_ai_data_compute_stat(FOSSIL_AI_DATA_ID_STDDEV, data, count, &std);

        if (std == 0.0f)
            return false;

        for (size_t i = 0; i < count; ++i)
            data[i] = (data[i] - mean) / std;

        return true;
    }

    if (strcmp(transform_id, FOSSIL_AI_DATA_ID_L2) == 0)
    {
        float sum_sq = 0.0f;
        for (size_t i = 0; i < count; ++i)
            sum_sq += data[i] * data[i];

        float norm = sqrtf(sum_sq);
        if (norm == 0.0f)
            return false;

        for (size_t i = 0; i < count; ++i)
            data[i] /= norm;

        return true;
    }

    return false;
}

/* ======================================================
 * Batching
 * ====================================================== */

bool fossil_ai_data_create_batch(const fossil_ai_data_dataset_t *ds,
                                 size_t start_index,
                                 size_t batch_size,
                                 fossil_ai_data_batch_t *out_batch)
{
    if (!ds || !out_batch || start_index >= ds->sample_count)
        return false;

    size_t end = start_index + batch_size;
    if (end > ds->sample_count)
        end = ds->sample_count;

    size_t actual = end - start_index;

    out_batch->batch_size = actual;
    out_batch->feature_size = ds->inputs.cols;

    out_batch->inputs = (float *)malloc(actual * ds->inputs.cols * sizeof(float));
    out_batch->targets = (float *)malloc(actual * ds->targets.cols * sizeof(float));

    if (!out_batch->inputs || !out_batch->targets)
        return false;

    memcpy(out_batch->inputs,
           &ds->inputs.data[start_index * ds->inputs.stride],
           actual * ds->inputs.cols * sizeof(float));

    memcpy(out_batch->targets,
           &ds->targets.data[start_index * ds->targets.stride],
           actual * ds->targets.cols * sizeof(float));

    return true;
}

void fossil_ai_data_free_batch(fossil_ai_data_batch_t *batch)
{
    if (!batch)
        return;

    free(batch->inputs);
    free(batch->targets);
    memset(batch, 0, sizeof(*batch));
}

/* ======================================================
 * Shuffle
 * ====================================================== */

bool fossil_ai_data_shuffle(float *data,
                            size_t stride,
                            size_t count,
                            uint32_t seed)
{
    if (!data || stride == 0 || count == 0)
        return false;

    srand(seed);

    for (size_t i = count - 1; i > 0; --i)
    {
        size_t j = rand() % (i + 1);

        for (size_t k = 0; k < stride; ++k)
        {
            float tmp = data[i * stride + k];
            data[i * stride + k] = data[j * stride + k];
            data[j * stride + k] = tmp;
        }
    }

    return true;
}
