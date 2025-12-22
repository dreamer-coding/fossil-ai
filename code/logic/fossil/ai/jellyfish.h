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

#define FOSSIL_AI_JELLYFISH_MAX_MEMORY 1024
#define FOSSIL_AI_JELLYFISH_EMBED_SIZE 64

typedef struct fossil_ai_jellyfish_memory_t {
    float embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE]; // Embedding vector
    float output[FOSSIL_AI_JELLYFISH_EMBED_SIZE];    // Output vector for attention
    int64_t timestamp;                                // When this memory was created
} fossil_ai_jellyfish_memory_t;

typedef struct fossil_ai_jellyfish_model_t {
    char name[128];
    uint64_t version;
    size_t input_size;
    size_t output_size;
    void *internal_state;  // Weights, grok-like state
    fossil_ai_jellyfish_memory_t memory[FOSSIL_AI_JELLYFISH_MAX_MEMORY];
    size_t memory_len;
} fossil_ai_jellyfish_model_t;

typedef struct fossil_ai_jellyfish_context_t {
    char session_id[64];
    size_t history_len;
    void **history;
    int64_t timestamp;
} fossil_ai_jellyfish_context_t;

// ======================================================
// Initialization / Cleanup
// ======================================================

fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_create_model(const char *name, size_t input_size, size_t output_size);
void fossil_ai_jellyfish_free_model(fossil_ai_jellyfish_model_t *model);

fossil_ai_jellyfish_context_t *fossil_ai_jellyfish_create_context(const char *session_id);
void fossil_ai_jellyfish_free_context(fossil_ai_jellyfish_context_t *ctx);

// ======================================================
// Training / Memory
// ======================================================

bool fossil_ai_jellyfish_train(fossil_ai_jellyfish_model_t *model,
                               const float *inputs,
                               const float *targets,
                               size_t count);

bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t *model,
                                    const float *input,
                                    const float *output,
                                    size_t embed_len);

// ======================================================
// Inference / Prediction
// ======================================================

bool fossil_ai_jellyfish_infer(fossil_ai_jellyfish_model_t *model,
                               fossil_ai_jellyfish_context_t *ctx,
                               const float *input,
                               float *output);

// ======================================================
// Persistence
// ======================================================

bool fossil_ai_jellyfish_save_model(const fossil_ai_jellyfish_model_t *model, const char *filepath);
fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_load_model(const char *filepath);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>

namespace fossil {

    namespace ai {
    
    class Jellyfish {
    public:
        // Constructor: create a model
        Jellyfish(const std::string &name, size_t input_size, size_t output_size)
            : model_(fossil_ai_jellyfish_create_model(name.c_str(), input_size, output_size)),
              context_(fossil_ai_jellyfish_create_context("default_session")) {}
    
        // Destructor: cleanup
        ~Jellyfish() {
            if (model_) fossil_ai_jellyfish_free_model(model_);
            if (context_) fossil_ai_jellyfish_free_context(context_);
        }
    
        // Deleted copy
        Jellyfish(const Jellyfish&) = delete;
        Jellyfish& operator=(const Jellyfish&) = delete;
    
        // Move semantics
        Jellyfish(Jellyfish&& other) noexcept : model_(other.model_), context_(other.context_) {
            other.model_ = nullptr;
            other.context_ = nullptr;
        }
    
        Jellyfish& operator=(Jellyfish&& other) noexcept {
            if (this != &other) {
                if (model_) fossil_ai_jellyfish_free_model(model_);
                if (context_) fossil_ai_jellyfish_free_context(context_);
                model_ = other.model_;
                context_ = other.context_;
                other.model_ = nullptr;
                other.context_ = nullptr;
            }
            return *this;
        }
    
        // ======================================================
        // Training / Memory
        // ======================================================
        bool train(const std::vector<float> &inputs,
                   const std::vector<float> &targets,
                   size_t count) {
            return fossil_ai_jellyfish_train(model_, inputs.data(), targets.data(), count);
        }
    
        bool add_memory(const std::vector<float> &input,
                        const std::vector<float> &output,
                        size_t embed_len) {
            return fossil_ai_jellyfish_add_memory(model_, input.data(), output.data(), embed_len);
        }
    
        // ======================================================
        // Inference / Prediction
        // ======================================================
        bool infer(const std::vector<float> &input,
                   std::vector<float> &output) {
            if (output.size() != model_->output_size) output.resize(model_->output_size);
            return fossil_ai_jellyfish_infer(model_, context_, input.data(), output.data());
        }
    
        // ======================================================
        // Persistence
        // ======================================================
        bool save_model(const std::string &filepath) {
            return fossil_ai_jellyfish_save_model(model_, filepath.c_str());
        }
    
        bool load_model(const std::string &filepath) {
            auto loaded = fossil_ai_jellyfish_load_model(filepath.c_str());
            if (!loaded) return false;
            if (model_) fossil_ai_jellyfish_free_model(model_);
            model_ = loaded;
            return true;
        }
    
        // Accessors
        size_t input_size() const { return model_ ? model_->input_size : 0; }
        size_t output_size() const { return model_ ? model_->output_size : 0; }
        const std::string name() const { return model_ ? std::string(model_->name) : ""; }
    
    private:
        fossil_ai_jellyfish_model_t *model_;
        fossil_ai_jellyfish_context_t *context_;
    };
    
    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
