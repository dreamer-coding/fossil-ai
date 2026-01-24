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

/**
 * @brief Creates a new Jellyfish AI model instance.
 *
 * Allocates and initializes a new model structure with the specified name,
 * input size, and output size. The caller is responsible for freeing the
 * returned model using fossil_ai_jellyfish_free_model().
 *
 * @param name        The name to assign to the model (null-terminated string).
 * @param input_size  The number of input features for the model.
 * @param output_size The number of output features for the model.
 * @return Pointer to the newly created model, or NULL on failure.
 */
fossil_ai_jellyfish_model_t *fossil_ai_jellyfish_create_model(const char *name, size_t input_size, size_t output_size);

/**
 * @brief Frees a Jellyfish AI model instance.
 *
 * Releases all resources associated with the given model. After calling this
 * function, the model pointer must not be used.
 *
 * @param model Pointer to the model to free.
 */
void fossil_ai_jellyfish_free_model(fossil_ai_jellyfish_model_t *model);

/**
 * @brief Creates a new Jellyfish AI context for a session.
 *
 * Allocates and initializes a context structure for managing a session with
 * the specified session ID. The caller is responsible for freeing the context
 * using fossil_ai_jellyfish_free_context().
 *
 * @param session_id The session identifier (null-terminated string).
 * @return Pointer to the newly created context, or NULL on failure.
 */
fossil_ai_jellyfish_context_t *fossil_ai_jellyfish_create_context(const char *session_id);

/**
 * @brief Frees a Jellyfish AI context.
 *
 * Releases all resources associated with the given context. After calling this
 * function, the context pointer must not be used.
 *
 * @param ctx Pointer to the context to free.
 */
void fossil_ai_jellyfish_free_context(fossil_ai_jellyfish_context_t *ctx);

// ======================================================
// Training / Memory
// ======================================================

/**
 * @brief Trains the Jellyfish AI model using the provided input and target data.
 *
 * This function updates the internal state of the given Jellyfish AI model
 * by performing a training step with the specified input and target arrays.
 *
 * @param model   Pointer to the Jellyfish AI model to be trained.
 * @param inputs  Pointer to an array of input feature values.
 * @param targets Pointer to an array of target output values corresponding to the inputs.
 * @param count   Number of training samples (length of inputs and targets arrays).
 *
 * @return true if the training step was successful, false otherwise.
 */
bool fossil_ai_jellyfish_train(fossil_ai_jellyfish_model_t *model,
                               const float *inputs,
                               const float *targets,
                               size_t count);

/**
 * @brief Adds a memory entry to the jellyfish AI model.
 *
 * This function stores a new memory consisting of an input embedding and its corresponding output
 * in the specified jellyfish model. The memory can be used for tasks such as recall, learning,
 * or inference within the model's logic.
 *
 * @param model Pointer to the jellyfish AI model where the memory will be added.
 * @param input Pointer to the input embedding array. The array should have a length of embed_len.
 * @param output Pointer to the output embedding array. The array should have a length of embed_len.
 * @param embed_len The length of the input and output embedding arrays.
 *
 * @return true if the memory was successfully added; false otherwise (e.g., if the model is full or
 *         if invalid parameters are provided).
 *
 * @note The function does not take ownership of the input or output arrays; the caller is responsible
 *       for managing their memory.
 */
bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t *model,
                                    const float *input,
                                    const float *output,
                                    size_t embed_len);

/**
 * @brief Performs inference (prediction) using the Jellyfish AI model.
 *
 * This function takes an input tensor and produces an output tensor by running
 * the model's inference logic. The context parameter may be used for thread safety,
 * resource management, or additional runtime configuration.
 *
 * @param model Pointer to a valid fossil_ai_jellyfish_model_t structure representing the loaded model.
 * @param ctx Pointer to a fossil_ai_jellyfish_context_t structure for context management.
 * @param input Pointer to the input data (float array) to be processed by the model.
 * @param output Pointer to the output buffer (float array) where the prediction results will be stored.
 * @return true if inference was successful, false otherwise.
 */
bool fossil_ai_jellyfish_infer(fossil_ai_jellyfish_model_t *model,
                               fossil_ai_jellyfish_context_t *ctx,
                               const float *input,
                               float *output);

/**
 * @brief Saves the Jellyfish AI model to a file.
 *
 * Serializes the model's parameters and state to the specified file path.
 * This allows the model to be persisted and loaded later for inference or further training.
 *
 * @param model Pointer to the model to be saved.
 * @param filepath Path to the file where the model will be saved.
 * @return true if the model was saved successfully, false otherwise.
 */
bool fossil_ai_jellyfish_save_model(const fossil_ai_jellyfish_model_t *model, const char *filepath);

/**
 * @brief Loads a Jellyfish AI model from a file.
 *
 * Deserializes the model from the specified file path, reconstructing its parameters and state.
 * The returned pointer must be freed appropriately when no longer needed.
 *
 * @param filepath Path to the file from which the model will be loaded.
 * @return Pointer to the loaded fossil_ai_jellyfish_model_t structure, or NULL on failure.
 */
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
        // ---------------------------------------------------------------------
        // Constructor: create a model
        // ---------------------------------------------------------------------
        /**
         * @brief Constructs a Jellyfish model and context.
         *
         * Initializes a new Jellyfish AI model with the given name, input size, and output size.
         * Also creates a default session context for inference and training.
         *
         * @param name        The name to assign to the model.
         * @param input_size  The number of input features for the model.
         * @param output_size The number of output features for the model.
         */
        Jellyfish(const std::string &name, size_t input_size, size_t output_size)
            : model_(fossil_ai_jellyfish_create_model(name.c_str(), input_size, output_size)),
              context_(fossil_ai_jellyfish_create_context("default_session")) {}

        // ---------------------------------------------------------------------
        // Destructor: cleanup
        // ---------------------------------------------------------------------
        /**
         * @brief Destructor for Jellyfish.
         *
         * Releases all resources associated with the model and context.
         */
        ~Jellyfish() {
            if (model_) fossil_ai_jellyfish_free_model(model_);
            if (context_) fossil_ai_jellyfish_free_context(context_);
        }

        // ---------------------------------------------------------------------
        // Deleted copy
        // ---------------------------------------------------------------------
        /**
         * @brief Deleted copy constructor.
         */
        Jellyfish(const Jellyfish&) = delete;
        /**
         * @brief Deleted copy assignment operator.
         */
        Jellyfish& operator=(const Jellyfish&) = delete;

        // ---------------------------------------------------------------------
        // Move semantics
        // ---------------------------------------------------------------------
        /**
         * @brief Move constructor.
         *
         * Transfers ownership of the model and context from another Jellyfish instance.
         */
        Jellyfish(Jellyfish&& other) noexcept : model_(other.model_), context_(other.context_) {
            other.model_ = nullptr;
            other.context_ = nullptr;
        }

        /**
         * @brief Move assignment operator.
         *
         * Transfers ownership of the model and context from another Jellyfish instance.
         */
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

        /**
         * @brief Trains the Jellyfish model.
         *
         * Performs a training step using the provided input and target arrays.
         *
         * @param inputs   Input feature values.
         * @param targets  Target output values.
         * @param count    Number of training samples.
         * @return true if training was successful, false otherwise.
         */
        bool train(const std::vector<float> &inputs,
               const std::vector<float> &targets,
               size_t count) {
            return fossil_ai_jellyfish_train(model_, inputs.data(), targets.data(), count);
        }

        /**
         * @brief Adds a memory entry to the Jellyfish model.
         *
         * Stores an input embedding and its corresponding output in the model's memory.
         *
         * @param input      Input embedding array.
         * @param output     Output embedding array.
         * @param embed_len  Length of the embedding arrays.
         * @return true if memory was added, false otherwise.
         */
        bool add_memory(const std::vector<float> &input,
                const std::vector<float> &output,
                size_t embed_len) {
            return fossil_ai_jellyfish_add_memory(model_, input.data(), output.data(), embed_len);
        }

        // ======================================================
        // Inference / Prediction
        // ======================================================

        /**
         * @brief Performs inference using the Jellyfish model.
         *
         * Runs the model's inference logic on the input and stores results in output.
         *
         * @param input   Input data array.
         * @param output  Output buffer for prediction results.
         * @return true if inference was successful, false otherwise.
         */
        bool infer(const std::vector<float> &input,
               std::vector<float> &output) {
            if (output.size() != model_->output_size) output.resize(model_->output_size);
            return fossil_ai_jellyfish_infer(model_, context_, input.data(), output.data());
        }

        // ======================================================
        // Persistence
        // ======================================================

        /**
         * @brief Saves the Jellyfish model to a file.
         *
         * Serializes the model's parameters and state to the specified file path.
         *
         * @param filepath Path to the file where the model will be saved.
         * @return true if the model was saved successfully, false otherwise.
         */
        bool save_model(const std::string &filepath) {
            return fossil_ai_jellyfish_save_model(model_, filepath.c_str());
        }

        /**
         * @brief Loads a Jellyfish model from a file.
         *
         * Loads model parameters and state from the specified file path.
         *
         * @param filepath Path to the file from which the model will be loaded.
         * @return true if the model was loaded successfully, false otherwise.
         */
        bool load_model(const std::string &filepath) {
            auto loaded = fossil_ai_jellyfish_load_model(filepath.c_str());
            if (!loaded) return false;
            if (model_) fossil_ai_jellyfish_free_model(model_);
            model_ = loaded;
            return true;
        }

        // ======================================================
        // Accessors
        // ======================================================

        /**
         * @brief Gets the input size of the model.
         * @return Number of input features.
         */
        size_t input_size() const { return model_ ? model_->input_size : 0; }

        /**
         * @brief Gets the output size of the model.
         * @return Number of output features.
         */
        size_t output_size() const { return model_ ? model_->output_size : 0; }

        /**
         * @brief Gets the name of the model.
         * @return Model name as a string.
         */
        const std::string name() const { return model_ ? std::string(model_->name) : ""; }
    
    private:
        fossil_ai_jellyfish_model_t *model_;
        fossil_ai_jellyfish_context_t *context_;
    };
    
    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
