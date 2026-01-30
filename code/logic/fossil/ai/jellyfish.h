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

#include <stddef.h>
#include <stdint.h>

// ----------------------------
// Basic Types
// ----------------------------
typedef struct fossil_ai_jellyfish_model fossil_ai_jellyfish_model_t;
typedef struct fossil_ai_jellyfish_context fossil_ai_jellyfish_context_t;
typedef struct fossil_ai_jellyfish_data fossil_ai_jellyfish_data_t;

// String ID type
typedef const char* fossil_ai_jellyfish_id_t;

// ----------------------------
// Initialization / Core
// ----------------------------

/**
 * Initialize the Jellyfish AI library.
 * Must be called before any other function.
 */
int fossil_ai_jellyfish_init();

/**
 * Shutdown the Jellyfish AI library and free resources.
 */
void fossil_ai_jellyfish_shutdown();

/**
 * Set up the cold core to enforce model integrity and protect against contamination.
 */
int fossil_ai_jellyfish_setup_cold_core(fossil_ai_jellyfish_model_t* model);

// ----------------------------
// Context Management
// ----------------------------
fossil_ai_jellyfish_context_t* fossil_ai_jellyfish_context_create();
void fossil_ai_jellyfish_context_destroy(fossil_ai_jellyfish_context_t* ctx);

/**
 * Add arbitrary context data to assist inference and training.
 */
int fossil_ai_jellyfish_context_add(fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_id_t key, const void* value, size_t size);

// ----------------------------
// Model Management
// ----------------------------
fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_model_load(fossil_ai_jellyfish_id_t model_path);
int fossil_ai_jellyfish_model_save(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_id_t model_path);
void fossil_ai_jellyfish_model_destroy(fossil_ai_jellyfish_model_t* model);

/**
 * Create a new model with optional multimodal capabilities.
 */
fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_model_create(fossil_ai_jellyfish_id_t model_name, int multimodal_enabled);

// ----------------------------
// Inference
// ----------------------------
/**
 * Run inference on a model given a context and input data.
 */
int fossil_ai_jellyfish_infer(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_data_t* input, fossil_ai_jellyfish_data_t* output);

/**
 * Auto-detect input type and route appropriately for multimodal inference.
 */
int fossil_ai_jellyfish_auto_infer(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_data_t* input, fossil_ai_jellyfish_data_t* output);

// ----------------------------
// Training / Retraining / Untraining
// ----------------------------
int fossil_ai_jellyfish_train(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_data_t* dataset);
int fossil_ai_jellyfish_retrain(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_data_t* dataset);
int fossil_ai_jellyfish_untrain(fossil_ai_jellyfish_model_t* model, fossil_ai_jellyfish_context_t* ctx, fossil_ai_jellyfish_data_t* dataset);

// ----------------------------
// Audit / Security
// ----------------------------
/**
 * Generate a cryptographic hash of a model to verify integrity.
 */
int fossil_ai_jellyfish_model_hash(fossil_ai_jellyfish_model_t* model, uint8_t* out_hash, size_t* hash_len);

/**
 * Audit a model for potential contamination or corruption.
 */
int fossil_ai_jellyfish_model_audit(fossil_ai_jellyfish_model_t* model);

// ----------------------------
// Data Management
// ----------------------------
fossil_ai_jellyfish_data_t* fossil_ai_jellyfish_data_create(size_t size);
void fossil_ai_jellyfish_data_destroy(fossil_ai_jellyfish_data_t* data);
int fossil_ai_jellyfish_data_set(fossil_ai_jellyfish_data_t* data, const void* buffer, size_t size);
int fossil_ai_jellyfish_data_get(fossil_ai_jellyfish_data_t* data, void* buffer, size_t size);

// ----------------------------
// Utility
// ----------------------------
/**
 * Auto-detect type of input data (text, image, audio, etc.).
 */
fossil_ai_jellyfish_id_t fossil_ai_jellyfish_data_autodetect(fossil_ai_jellyfish_data_t* data);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>

namespace fossil {

    namespace ai {

        class Jellyfish {
        public:
            // ----------------------------
            // Initialization / Core
            // ----------------------------
            Jellyfish() {
                fossil_ai_jellyfish_init();
            }
        
            ~Jellyfish() {
                fossil_ai_jellyfish_shutdown();
            }
        
            int setup_cold_core(fossil_ai_jellyfish_model_t* model) {
                return fossil_ai_jellyfish_setup_cold_core(model);
            }
        
            // ----------------------------
            // Context Management
            // ----------------------------
            fossil_ai_jellyfish_context_t* context_create() {
                return fossil_ai_jellyfish_context_create();
            }

            void context_destroy(fossil_ai_jellyfish_context_t* ctx) {
                fossil_ai_jellyfish_context_destroy(ctx);
            }
        
            int context_add(fossil_ai_jellyfish_context_t* ctx, const std::string& key, const void* value, size_t size) {
                return fossil_ai_jellyfish_context_add(ctx, key.c_str(), value, size);
            }

            // ----------------------------
            // Model Management
            // ----------------------------
            fossil_ai_jellyfish_model_t* model_load(const std::string& path) {
                return fossil_ai_jellyfish_model_load(path.c_str());
            }
        
            int model_save(fossil_ai_jellyfish_model_t* model, const std::string& path) {
                return fossil_ai_jellyfish_model_save(model, path.c_str());
            }

            void model_destroy(fossil_ai_jellyfish_model_t* model) {
                fossil_ai_jellyfish_model_destroy(model);
            }

            fossil_ai_jellyfish_model_t* model_create(const std::string& name, bool multimodal_enabled) {
                return fossil_ai_jellyfish_model_create(name.c_str(), multimodal_enabled ? 1 : 0);
            }

            // ----------------------------
            // Inference
            // ----------------------------
            int infer(fossil_ai_jellyfish_model_t* model,
                      fossil_ai_jellyfish_context_t* ctx,
                      fossil_ai_jellyfish_data_t* input,
                      fossil_ai_jellyfish_data_t* output) {
                return fossil_ai_jellyfish_infer(model, ctx, input, output);
            }

            int auto_infer(fossil_ai_jellyfish_model_t* model,
                           fossil_ai_jellyfish_context_t* ctx,
                           fossil_ai_jellyfish_data_t* input,
                           fossil_ai_jellyfish_data_t* output) {
                return fossil_ai_jellyfish_auto_infer(model, ctx, input, output);
            }

            // ----------------------------
            // Training / Retraining / Untraining
            // ----------------------------
            int train(fossil_ai_jellyfish_model_t* model,
                      fossil_ai_jellyfish_context_t* ctx,
                      fossil_ai_jellyfish_data_t* dataset) {
                return fossil_ai_jellyfish_train(model, ctx, dataset);
            }

            int retrain(fossil_ai_jellyfish_model_t* model,
                        fossil_ai_jellyfish_context_t* ctx,
                        fossil_ai_jellyfish_data_t* dataset) {
                return fossil_ai_jellyfish_retrain(model, ctx, dataset);
            }

            int untrain(fossil_ai_jellyfish_model_t* model,
                        fossil_ai_jellyfish_context_t* ctx,
                        fossil_ai_jellyfish_data_t* dataset) {
                return fossil_ai_jellyfish_untrain(model, ctx, dataset);
            }

            // ----------------------------
            // Audit / Security
            // ----------------------------
            int model_hash(fossil_ai_jellyfish_model_t* model, std::vector<uint8_t>& out_hash) {
                size_t hash_len = 32; // default max length
                out_hash.resize(hash_len);
                return fossil_ai_jellyfish_model_hash(model, out_hash.data(), &hash_len);
            }
        
            int model_audit(fossil_ai_jellyfish_model_t* model) {
                return fossil_ai_jellyfish_model_audit(model);
            }

            // ----------------------------
            // Data Management
            // ----------------------------
            fossil_ai_jellyfish_data_t* data_create(size_t size) {
                return fossil_ai_jellyfish_data_create(size);
            }
        
            void data_destroy(fossil_ai_jellyfish_data_t* data) {
                fossil_ai_jellyfish_data_destroy(data);
            }

            int data_set(fossil_ai_jellyfish_data_t* data, const void* buffer, size_t size) {
                return fossil_ai_jellyfish_data_set(data, buffer, size);
            }

            int data_get(fossil_ai_jellyfish_data_t* data, void* buffer, size_t size) {
                return fossil_ai_jellyfish_data_get(data, buffer, size);
            }

            std::string data_autodetect(fossil_ai_jellyfish_data_t* data) {
                fossil_ai_jellyfish_id_t id = fossil_ai_jellyfish_data_autodetect(data);
                return std::string(id ? id : "");
            }
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
