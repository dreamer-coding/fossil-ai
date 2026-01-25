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
#include "fossil/ai/ml.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

// ======================================================
// Activation functions
// ======================================================

static float activate(float x, fossil_ai_ml_activation_t act) {
    switch (act) {
        case FOSSIL_AI_ML_ACT_RELU: return fmaxf(0.0f, x);
        case FOSSIL_AI_ML_ACT_SIGMOID: return 1.0f / (1.0f + expf(-x));
        default: return x;
    }
}

static float activate_derivative(float x, fossil_ai_ml_activation_t act) {
    switch (act) {
        case FOSSIL_AI_ML_ACT_RELU: return x > 0.0f ? 1.0f : 0.0f;
        case FOSSIL_AI_ML_ACT_SIGMOID: {
            float s = 1.0f / (1.0f + expf(-x));
            return s * (1.0f - s);
        }
        default: return 1.0f;
    }
}

// ======================================================
// MLP Creation
// ======================================================

fossil_ai_ml_mlp_t *
fossil_ai_ml_create_mlp(size_t layer_count,
                        const size_t *neurons,
                        const fossil_ai_ml_activation_t *activations) {
    if (layer_count == 0 || layer_count > FOSSIL_AI_ML_MAX_LAYERS) return NULL;

    fossil_ai_ml_mlp_t *mlp = calloc(1, sizeof(fossil_ai_ml_mlp_t));
    mlp->layer_count = layer_count;

    for (size_t i = 0; i < layer_count; i++) {
        mlp->neurons[i] = neurons[i];
        mlp->activations[i] = activations[i];

        if (i == 0) continue;
        size_t prev = neurons[i-1];
        size_t curr = neurons[i];
        mlp->weights[i] = calloc(prev * curr, sizeof(float));
        // Initialize weights with small random values
        for (size_t j = 0; j < prev * curr; j++)
            mlp->weights[i][j] = ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f;
    }
    return mlp;
}

void fossil_ai_ml_free_mlp(fossil_ai_ml_mlp_t *mlp) {
    if (!mlp) return;
    for (size_t i = 1; i < mlp->layer_count; i++)
        free(mlp->weights[i]);
    free(mlp);
}

// ======================================================
// Forward Pass
// ======================================================

static void mlp_forward(fossil_ai_ml_mlp_t *mlp,
                        const float *input,
                        float activations[FOSSIL_AI_ML_MAX_LAYERS][FOSSIL_AI_ML_MAX_NEURONS]) {
    memcpy(activations[0], input, sizeof(float) * mlp->neurons[0]);

    for (size_t l = 1; l < mlp->layer_count; l++) {
        size_t prev = mlp->neurons[l-1];
        size_t curr = mlp->neurons[l];
        for (size_t j = 0; j < curr; j++) {
            float sum = 0.0f;
            for (size_t i = 0; i < prev; i++)
                sum += mlp->weights[l][j * prev + i] * activations[l-1][i];
            activations[l][j] = activate(sum, mlp->activations[l]);
        }
    }
}

// ======================================================
// Backpropagation
// ======================================================

bool fossil_ai_ml_train_mlp(fossil_ai_ml_mlp_t *mlp,
                            const float *inputs,
                            const float *targets,
                            size_t count,
                            float learning_rate,
                            size_t epochs) {
    if (!mlp || !inputs || !targets) return false;

    float activations[FOSSIL_AI_ML_MAX_LAYERS][FOSSIL_AI_ML_MAX_NEURONS] = {0};
    float deltas[FOSSIL_AI_ML_MAX_LAYERS][FOSSIL_AI_ML_MAX_NEURONS] = {0};

    for (size_t epoch = 0; epoch < epochs; epoch++) {
        for (size_t n = 0; n < count; n++) {
            const float *x = inputs + n * mlp->neurons[0];
            const float *y = targets + n * mlp->neurons[mlp->layer_count-1];

            mlp_forward(mlp, x, activations);

            // Compute deltas for output layer
            size_t out_layer = mlp->layer_count - 1;
            for (size_t j = 0; j < mlp->neurons[out_layer]; j++) {
                float a = activations[out_layer][j];
                deltas[out_layer][j] = (a - y[j]) * activate_derivative(a, mlp->activations[out_layer]);
            }

            // Backpropagate
            for (size_t l = out_layer - 1; l > 0; l--) {
                for (size_t i = 0; i < mlp->neurons[l]; i++) {
                    float sum = 0.0f;
                    for (size_t j = 0; j < mlp->neurons[l+1]; j++)
                        sum += mlp->weights[l+1][j * mlp->neurons[l] + i] * deltas[l+1][j];
                    deltas[l][i] = sum * activate_derivative(activations[l][i], mlp->activations[l]);
                }
            }

            // Update weights
            for (size_t l = 1; l < mlp->layer_count; l++) {
                size_t prev = mlp->neurons[l-1];
                size_t curr = mlp->neurons[l];
                for (size_t j = 0; j < curr; j++)
                    for (size_t i = 0; i < prev; i++)
                        mlp->weights[l][j * prev + i] -= learning_rate * deltas[l][j] * activations[l-1][i];
            }
        }
    }
    return true;
}

// ======================================================
// Prediction
// ======================================================

bool fossil_ai_ml_predict_mlp(fossil_ai_ml_mlp_t *mlp,
                              const float *input,
                              float *output) {
    if (!mlp || !input || !output) return false;

    float activations[FOSSIL_AI_ML_MAX_LAYERS][FOSSIL_AI_ML_MAX_NEURONS] = {0};
    mlp_forward(mlp, input, activations);

    size_t out_layer = mlp->layer_count - 1;
    memcpy(output, activations[out_layer], sizeof(float) * mlp->neurons[out_layer]);
    return true;
}
