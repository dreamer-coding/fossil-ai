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
#ifndef FOSSIL_JELLYFISH_ML_H
#define FOSSIL_JELLYFISH_ML_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// MLP Limits
// ======================================================

#define FOSSIL_AI_ML_MAX_LAYERS 8
#define FOSSIL_AI_ML_MAX_NEURONS 128

typedef enum {
    FOSSIL_AI_ML_ACT_NONE = 0,
    FOSSIL_AI_ML_ACT_RELU,
    FOSSIL_AI_ML_ACT_SIGMOID
} fossil_ai_ml_activation_t;

// ======================================================
// MLP Model
// ======================================================

typedef struct {
    size_t layer_count;
    size_t neurons[FOSSIL_AI_ML_MAX_LAYERS];
    fossil_ai_ml_activation_t activations[FOSSIL_AI_ML_MAX_LAYERS];
    float *weights[FOSSIL_AI_ML_MAX_LAYERS]; // flattened weight matrices
} fossil_ai_ml_mlp_t;

// ======================================================
// MLP Management
// ======================================================

fossil_ai_ml_mlp_t *
fossil_ai_ml_create_mlp(size_t layer_count,
                        const size_t *neurons,
                        const fossil_ai_ml_activation_t *activations);

void fossil_ai_ml_free_mlp(fossil_ai_ml_mlp_t *mlp);

bool fossil_ai_ml_train_mlp(fossil_ai_ml_mlp_t *mlp,
                            const float *inputs,
                            const float *targets,
                            size_t count,
                            float learning_rate,
                            size_t epochs);

bool fossil_ai_ml_predict_mlp(fossil_ai_ml_mlp_t *mlp,
                              const float *input,
                              float *output);

#ifdef __cplusplus
}

#include <vector>
#include <memory>

namespace fossil {

    namespace ai {
    
        class ML {
        public:
            ML() = default;
            ~ML() { free_mlp(); }
        
            // Create MLP
            bool create_mlp(size_t layer_count,
                            const std::vector<size_t> &neurons,
                            const std::vector<fossil_ai_ml_activation_t> &activations) {
                free_mlp(); // clean previous if any
                if (neurons.size() != layer_count || activations.size() != layer_count) return false;
                mlp_ = fossil_ai_ml_create_mlp(layer_count, neurons.data(), activations.data());
                return mlp_ != nullptr;
            }
        
            void free_mlp() {
                if (mlp_) {
                    fossil_ai_ml_free_mlp(mlp_);
                    mlp_ = nullptr;
                }
            }
        
            // Train MLP
            bool train_mlp(const std::vector<float> &inputs,
                           const std::vector<float> &targets,
                           size_t count,
                           float learning_rate,
                           size_t epochs) {
                if (!mlp_) return false;
                return fossil_ai_ml_train_mlp(mlp_, inputs.data(), targets.data(), count, learning_rate, epochs);
            }
        
            // Predict
            bool predict_mlp(const std::vector<float> &input, std::vector<float> &output) {
                if (!mlp_ || input.size() < mlp_->neurons[0]) return false;
                output.resize(mlp_->neurons[mlp_->layer_count - 1]);
                return fossil_ai_ml_predict_mlp(mlp_, input.data(), output.data());
            }
        
            fossil_ai_ml_mlp_t* get_c_mlp() { return mlp_; }
        
        private:
            fossil_ai_ml_mlp_t* mlp_ = nullptr;
        };
    
    } // namespace ai

} // namespace fossil

#endif

#endif /* AI_FRAMEWORK_H */
