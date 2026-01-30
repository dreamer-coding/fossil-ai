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

/* ======================================================
 * Constants
 * ====================================================== */

#define FOSSIL_AI_JELLYFISH_MAX_MEMORY 1024
#define FOSSIL_AI_JELLYFISH_EMBED_SIZE 64
#define FOSSIL_AI_JELLYFISH_MODEL_NAME_LEN 128

/* ======================================================
 * Types
 * ====================================================== */

/* Memory vector for embeddings and attention outputs */
typedef struct fossil_ai_jellyfish_memory_t {
    float embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE]; // Embedding vector
    float output[FOSSIL_AI_JELLYFISH_EMBED_SIZE];    // Output vector for attention
    int64_t timestamp;                                // Timestamp of memory
    char id[64];                                      // String ID
} fossil_ai_jellyfish_memory_t;

/* Persistent model structure */
typedef struct fossil_ai_jellyfish_model_t {
    char name[FOSSIL_AI_JELLYFISH_MODEL_NAME_LEN];
    uint64_t version;
    size_t memory_count;
    fossil_ai_jellyfish_memory_t memory[FOSSIL_AI_JELLYFISH_MAX_MEMORY];
    bool trained;                                    // Has model been trained
} fossil_ai_jellyfish_model_t;

/* Hardware and system info */
typedef struct fossil_ai_jellyfish_system_info_t {
    size_t ram_bytes;
    size_t cpu_cores;
    bool is_little_endian;
} fossil_ai_jellyfish_system_info_t;

/* ======================================================
 * Initialization / Cleanup
 * ====================================================== */

/* Create a new AI model instance */
fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_create_model(const char* name);

/* Free a model instance */
void fossil_ai_jellyfish_free_model(fossil_ai_jellyfish_model_t* model);

/* ======================================================
 * Model Persistence
 * ====================================================== */

/* Save model to file */
bool fossil_ai_jellyfish_save_model(const fossil_ai_jellyfish_model_t* model, const char* path);

/* Load model from file */
fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_load_model(const char* path);

/* ======================================================
 * Memory Management
 * ====================================================== */

/* Add memory vector to model */
bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t* model, const float* embedding, const float* output, const char* id, int64_t timestamp);

/* Retrieve memory by string ID */
fossil_ai_jellyfish_memory_t* fossil_ai_jellyfish_get_memory(const fossil_ai_jellyfish_model_t* model, const char* id);

/* ======================================================
 * Training / Inference
 * ====================================================== */

/* Train the model with current memory (incremental) */
bool fossil_ai_jellyfish_train_model(fossil_ai_jellyfish_model_t* model);

/* Predict output embedding from input embedding */
bool fossil_ai_jellyfish_predict(const fossil_ai_jellyfish_model_t* model, const float* input_embedding, float* output_embedding);

/* ======================================================
 * System / Hardware Awareness
 * ====================================================== */

/* Detect system endianness */
bool fossil_ai_jellyfish_is_little_endian(void);

/* Retrieve system info */
fossil_ai_jellyfish_system_info_t fossil_ai_jellyfish_get_system_info(void);

/* ======================================================
 * Utility / Hashing
 * ====================================================== */

/* Simple FNV-1a 32-bit hash for strings */
uint32_t fossil_ai_jellyfish_hash_string(const char* s);

/* ======================================================
 * Auto-Detection
 * ====================================================== */

/* Detect and report AI model capabilities */
void fossil_ai_jellyfish_detect_capabilities(fossil_ai_jellyfish_model_t* model);

void fossil_ai_jellyfish_audit(const fossil_ai_jellyfish_model_t* model);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>

namespace fossil {

    namespace ai {

        class Jellyfish {
        public:
            /* ======================================================
             * Construction / Destruction
             * ====================================================== */
        
            explicit Jellyfish(const std::string& name)
                : model_(fossil_ai_jellyfish_create_model(name.c_str())) {}
        
            ~Jellyfish() {
                if (model_) {
                    fossil_ai_jellyfish_free_model(model_);
                    model_ = nullptr;
                }
            }
        
            // Non-copyable (explicit ownership of C model)
            Jellyfish(const Jellyfish&) = delete;
            Jellyfish& operator=(const Jellyfish&) = delete;
        
            // Movable
            Jellyfish(Jellyfish&& other) noexcept : model_(other.model_) {
                other.model_ = nullptr;
            }
        
            Jellyfish& operator=(Jellyfish&& other) noexcept {
                if (this != &other) {
                    fossil_ai_jellyfish_free_model(model_);
                    model_ = other.model_;
                    other.model_ = nullptr;
                }
                return *this;
            }
        
            /* ======================================================
             * Model Persistence
             * ====================================================== */
        
            bool save_model(const std::string& path) const {
                return model_ && fossil_ai_jellyfish_save_model(model_, path.c_str());
            }
        
            static Jellyfish load_model(const std::string& path) {
                fossil_ai_jellyfish_model_t* m =
                    fossil_ai_jellyfish_load_model(path.c_str());
        
                Jellyfish jf("");
                jf.model_ = m;
                return jf;
            }
        
            /* ======================================================
             * Memory Management
             * ====================================================== */
        
            bool add_memory(const float* embedding,
                            const float* output,
                            const std::string& id,
                            int64_t timestamp) {
                return model_ &&
                       fossil_ai_jellyfish_add_memory(
                           model_, embedding, output, id.c_str(), timestamp);
            }
        
            fossil_ai_jellyfish_memory_t* get_memory(const std::string& id) const {
                return model_
                    ? fossil_ai_jellyfish_get_memory(model_, id.c_str())
                    : nullptr;
            }
        
            size_t memory_count() const {
                return model_ ? model_->memory_count : 0;
            }
        
            /* ======================================================
             * Training / Inference
             * ====================================================== */
        
            bool train_model() {
                return model_ && fossil_ai_jellyfish_train_model(model_);
            }
        
            bool predict(const float* input_embedding,
                         float* output_embedding) const {
                return model_ &&
                       fossil_ai_jellyfish_predict(
                           model_, input_embedding, output_embedding);
            }
        
            bool is_trained() const {
                return model_ && model_->trained;
            }
        
            /* ======================================================
             * System / Hardware Awareness
             * ====================================================== */
        
            static bool is_little_endian() {
                return fossil_ai_jellyfish_is_little_endian();
            }
        
            static fossil_ai_jellyfish_system_info_t get_system_info() {
                return fossil_ai_jellyfish_get_system_info();
            }
        
            /* ======================================================
             * Utility / Hashing
             * ====================================================== */
        
            static uint32_t hash_string(const std::string& s) {
                return fossil_ai_jellyfish_hash_string(s.c_str());
            }
        
            /* ======================================================
             * Introspection / Audit
             * ====================================================== */
        
            void detect_capabilities() const {
                if (model_) {
                    fossil_ai_jellyfish_detect_capabilities(model_);
                }
            }
        
            void audit() const {
                if (model_) {
                    fossil_ai_jellyfish_audit(model_);
                }
            }
        
            /* ======================================================
             * Low-level Access (escape hatch)
             * ====================================================== */
        
            fossil_ai_jellyfish_model_t* raw() { return model_; }
            const fossil_ai_jellyfish_model_t* raw() const { return model_; }
        
        private:
            fossil_ai_jellyfish_model_t* model_ = nullptr;
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
