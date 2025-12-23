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
#include "fossil/ai/cv.h"
#include "fossil/ai/ml.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ======================================================
// CNN Layer Creation
// ======================================================

fossil_ai_cv_cnn_t *fossil_ai_cv_create_cnn(size_t input_width, size_t input_height, size_t kernel_size, size_t pool_size) {
    fossil_ai_cv_cnn_t *cnn = calloc(1, sizeof(fossil_ai_cv_cnn_t));
    cnn->conv.input_width = input_width;
    cnn->conv.input_height = input_height;
    cnn->conv.kernel_size = kernel_size;
    cnn->conv.kernel = calloc(kernel_size * kernel_size, sizeof(float));
    for (size_t i = 0; i < kernel_size * kernel_size; i++)
        cnn->conv.kernel[i] = ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
    cnn->pool_size = pool_size;
    cnn->output_size = ((input_width - kernel_size + 1)/pool_size) * ((input_height - kernel_size + 1)/pool_size);
    if (cnn->output_size > FOSSIL_AI_CV_MAX_CNN_FEATURES)
        cnn->output_size = FOSSIL_AI_CV_MAX_CNN_FEATURES;
    return cnn;
}

void fossil_ai_cv_free_cnn(fossil_ai_cv_cnn_t *cnn) {
    if (!cnn) return;
    free(cnn->conv.kernel);
    free(cnn);
}

// ======================================================
// Convolution (single-channel grayscale)
// ======================================================

static void cnn_convolve(fossil_ai_cv_cnn_t *cnn, const fossil_ai_cv_image_t *img, float *feature_map) {
    size_t kw = cnn->conv.kernel_size;
    size_t kh = cnn->conv.kernel_size;
    size_t out_w = img->width - kw + 1;
    size_t out_h = img->height - kh + 1;

    for (size_t y = 0; y < out_h; y++) {
        for (size_t x = 0; x < out_w; x++) {
            float sum = 0.0f;
            for (size_t ky = 0; ky < kh; ky++)
                for (size_t kx = 0; kx < kw; kx++) {
                    size_t idx = (y+ky)*img->width + (x+kx);
                    sum += img->data[idx] * cnn->conv.kernel[ky*kw + kx];
                }
            feature_map[y*out_w + x] = sum;
        }
    }
}

// ======================================================
// Max Pooling
// ======================================================

static void cnn_pool(fossil_ai_cv_cnn_t *cnn, const float *feature_map,
                     size_t in_w, size_t in_h,
                     float *pooled) {
    size_t pool = cnn->pool_size;
    size_t out_w = in_w / pool;
    size_t out_h = in_h / pool;

    for (size_t y = 0; y < out_h; y++) {
        for (size_t x = 0; x < out_w; x++) {
            float max_val = -INFINITY;
            for (size_t py = 0; py < pool; py++)
                for (size_t px = 0; px < pool; px++) {
                    size_t idx = (y*pool+py)*in_w + (x*pool+px);
                    if (idx < in_w*in_h && feature_map[idx] > max_val)
                        max_val = feature_map[idx];
                }
            pooled[y*out_w + x] = max_val;
        }
    }
}

// ======================================================
// CNN Forward Pass
// ======================================================

bool fossil_ai_cv_cnn_forward(fossil_ai_cv_cnn_t *cnn,
                              const fossil_ai_cv_image_t *img,
                              float *output, size_t output_len) {
    if (!cnn || !img || !output) return false;

    fossil_ai_cv_image_t gray;
    fossil_ai_cv_grayscale(img, &gray);

    size_t fmap_w = gray.width - cnn->conv.kernel_size + 1;
    size_t fmap_h = gray.height - cnn->conv.kernel_size + 1;

    float *feature_map = calloc(fmap_w * fmap_h, sizeof(float));
    cnn_convolve(cnn, &gray, feature_map);

    cnn_pool(cnn, feature_map, fmap_w, fmap_h, output);

    free(feature_map);
    return true;
}

// ======================================================
// ML Integration Prediction
// ======================================================

bool fossil_ai_cv_cnn_predict(fossil_ai_jellyfish_model_t *model,
                              fossil_ai_cv_cnn_t *cnn,
                              const fossil_ai_cv_image_t *img,
                              float *output) {
    if (!cnn || !img || !output || !model) return false;

    float features[FOSSIL_AI_CV_MAX_CNN_FEATURES] = {0};
    fossil_ai_cv_cnn_forward(cnn, img, features, sizeof(features)/sizeof(float));

    return fossil_ai_ml_predict(model, features, output);
}

// ======================================================
// CNN + MLP Backprop Training
// ======================================================

bool fossil_ai_cv_train_cnn(fossil_ai_cv_cnn_t *cnn,
                            fossil_ai_jellyfish_model_t *mlp_model,
                            const fossil_ai_cv_image_t *images,
                            const float *targets,
                            size_t count,
                            float learning_rate,
                            size_t epochs) {
    if (!cnn || !mlp_model || !images || !targets) return false;

    size_t fmap_w = cnn->conv.input_width - cnn->conv.kernel_size + 1;
    size_t fmap_h = cnn->conv.input_height - cnn->conv.kernel_size + 1;
    size_t pooled_w = fmap_w / cnn->pool_size;
    size_t pooled_h = fmap_h / cnn->pool_size;
    size_t feature_len = pooled_w * pooled_h;

    float *feature_map = calloc(fmap_w * fmap_h, sizeof(float));
    float *pooled = calloc(feature_len, sizeof(float));

    for (size_t epoch = 0; epoch < epochs; epoch++) {
        for (size_t n = 0; n < count; n++) {
            fossil_ai_cv_image_t gray;
            fossil_ai_cv_grayscale(&images[n], &gray);

            // Forward pass
            cnn_convolve(cnn, &gray, feature_map);
            cnn_pool(cnn, feature_map, fmap_w, fmap_h, pooled);

            // Forward through MLP
            float mlp_out[FOSSIL_AI_JELLYFISH_EMBED_SIZE] = {0};
            fossil_ai_ml_predict(mlp_model, pooled, mlp_out);

            // Compute output error (simple MSE)
            float error[FOSSIL_AI_JELLYFISH_EMBED_SIZE] = {0};
            for (size_t i = 0; i < FOSSIL_AI_JELLYFISH_EMBED_SIZE; i++)
                error[i] = mlp_out[i] - targets[n * FOSSIL_AI_JELLYFISH_EMBED_SIZE + i];

            // Backpropagate to MLP
            fossil_ai_ml_train_mlp(mlp_model, pooled, &targets[n * FOSSIL_AI_JELLYFISH_EMBED_SIZE], 1, learning_rate, 1);

            // -------------------------------------------------
            // CNN kernel update (simple gradient approximation)
            // For each kernel weight, compute sum of gradients from pooled output
            for (size_t ky = 0; ky < cnn->conv.kernel_size; ky++) {
                for (size_t kx = 0; kx < cnn->conv.kernel_size; kx++) {
                    float grad = 0.0f;
                    for (size_t y = 0; y < fmap_h; y += cnn->pool_size) {
                        for (size_t x = 0; x < fmap_w; x += cnn->pool_size) {
                            size_t idx = y * fmap_w + x;
                            // Approximate influence of weight
                            grad += feature_map[idx] * error[idx % FOSSIL_AI_JELLYFISH_EMBED_SIZE];
                        }
                    }
                    cnn->conv.kernel[ky*cnn->conv.kernel_size + kx] -= learning_rate * grad;
                }
            }
        }
    }

    free(feature_map);
    free(pooled);
    return true;
}
