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
#ifndef FOSSIL_JELLYFISH_DATA_H
#define FOSSIL_JELLYFISH_DATA_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ======================================================
 * Constants / String IDs
 * ====================================================== */

#define FOSSIL_AI_DATA_ID_NONE "data.none"
#define FOSSIL_AI_DATA_ID_MINMAX "data.normalize.minmax"
#define FOSSIL_AI_DATA_ID_STANDARD "data.normalize.standard"
#define FOSSIL_AI_DATA_ID_L2 "data.normalize.l2"

#define FOSSIL_AI_DATA_ID_MEAN "data.stat.mean"
#define FOSSIL_AI_DATA_ID_VARIANCE "data.stat.variance"
#define FOSSIL_AI_DATA_ID_STDDEV "data.stat.stddev"

#define FOSSIL_AI_DATA_ID_SHUFFLE "data.transform.shuffle"
#define FOSSIL_AI_DATA_ID_BATCH "data.transform.batch"

/* ======================================================
    * Core Data Structures
    * ====================================================== */

typedef struct fossil_ai_data_tensor_t
{
    float *data;
    size_t rows;
    size_t cols;
    size_t stride;
} fossil_ai_data_tensor_t;

typedef struct fossil_ai_data_dataset_t
{
    fossil_ai_data_tensor_t inputs;
    fossil_ai_data_tensor_t targets;
    size_t sample_count;
} fossil_ai_data_dataset_t;

typedef struct fossil_ai_data_batch_t
{
    float *inputs;
    float *targets;
    size_t batch_size;
    size_t feature_size;
} fossil_ai_data_batch_t;

/* ======================================================
    * Tensor Utilities
    * ====================================================== */

bool fossil_ai_data_tensor_init(fossil_ai_data_tensor_t *tensor,
                                size_t rows,
                                size_t cols);

void fossil_ai_data_tensor_free(fossil_ai_data_tensor_t *tensor);

float fossil_ai_data_tensor_get(const fossil_ai_data_tensor_t *tensor,
                                size_t row,
                                size_t col);

void fossil_ai_data_tensor_set(fossil_ai_data_tensor_t *tensor,
                                size_t row,
                                size_t col,
                                float value);

/* ======================================================
    * Dataset Management
    * ====================================================== */

bool fossil_ai_data_dataset_init(fossil_ai_data_dataset_t *ds,
                                    size_t samples,
                                    size_t input_size,
                                    size_t target_size);

void fossil_ai_data_dataset_free(fossil_ai_data_dataset_t *ds);

/* ======================================================
    * Statistics (string-id driven)
    * ====================================================== */

bool fossil_ai_data_compute_stat(const char *stat_id,
                                    const float *data,
                                    size_t count,
                                    float *out_value);

/* ======================================================
    * Normalization / Transforms
    * ====================================================== */

bool fossil_ai_data_apply_transform(const char *transform_id,
                                    float *data,
                                    size_t count);

/* ======================================================
    * Batching
    * ====================================================== */

bool fossil_ai_data_create_batch(const fossil_ai_data_dataset_t *ds,
                                    size_t start_index,
                                    size_t batch_size,
                                    fossil_ai_data_batch_t *out_batch);

void fossil_ai_data_free_batch(fossil_ai_data_batch_t *batch);

/* ======================================================
    * Random / Shuffle
    * ====================================================== */

bool fossil_ai_data_shuffle(float *data,
                            size_t stride,
                            size_t count,
                            uint32_t seed);

#ifdef __cplusplus
}

#include <vector>
#include <memory>

namespace fossil
{

    namespace ai
    {

        class Data
        {
        public:
            static bool tensor_init(fossil_ai_data_tensor_t *tensor, size_t rows, size_t cols)
            {
                return fossil_ai_data_tensor_init(tensor, rows, cols);
            }

            static void tensor_free(fossil_ai_data_tensor_t *tensor)
            {
                fossil_ai_data_tensor_free(tensor);
            }

            static float tensor_get(const fossil_ai_data_tensor_t *tensor, size_t row, size_t col)
            {
                return fossil_ai_data_tensor_get(tensor, row, col);
            }

            static void tensor_set(fossil_ai_data_tensor_t *tensor, size_t row, size_t col, float value)
            {
                fossil_ai_data_tensor_set(tensor, row, col, value);
            }

            static bool dataset_init(fossil_ai_data_dataset_t *ds, size_t samples, size_t input_size, size_t target_size)
            {
                return fossil_ai_data_dataset_init(ds, samples, input_size, target_size);
            }

            static void dataset_free(fossil_ai_data_dataset_t *ds)
            {
                fossil_ai_data_dataset_free(ds);
            }

            static bool compute_stat(const char *stat_id, const float *data, size_t count, float *out_value)
            {
                return fossil_ai_data_compute_stat(stat_id, data, count, out_value);
            }

            static bool apply_transform(const char *transform_id, float *data, size_t count)
            {
                return fossil_ai_data_apply_transform(transform_id, data, count);
            }

            static bool create_batch(const fossil_ai_data_dataset_t *ds, size_t start_index, size_t batch_size, fossil_ai_data_batch_t *out_batch)
            {
                return fossil_ai_data_create_batch(ds, start_index, batch_size, out_batch);
            }

            static void free_batch(fossil_ai_data_batch_t *batch)
            {
                fossil_ai_data_free_batch(batch);
            }

            static bool shuffle(float *data, size_t stride, size_t count, uint32_t seed)
            {
                return fossil_ai_data_shuffle(data, stride, count, seed);
            }
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* AI_FRAMEWORK_H */
