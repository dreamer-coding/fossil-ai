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
#ifndef FOSSIL_JELLYFISH_IOCHAT_H
#define FOSSIL_JELLYFISH_IOCHAT_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_CHAT_MAX_HISTORY     128
#define FOSSIL_AI_CHAT_MAX_RESPONSE    512
#define FOSSIL_AI_CHAT_PERSISTENT_MAX  256
#define FOSSIL_AI_CHAT_MAX_TOKENS      128
#define FOSSIL_AI_CHAT_MAX_TOKEN_LEN   32
#define FOSSIL_AI_CHAT_HASH_SIZE       1024

// ======================================================
// Intent Classification
// ======================================================

typedef enum {
    FOSSIL_AI_CHAT_INTENT_UNKNOWN = 0,
    FOSSIL_AI_CHAT_INTENT_QUESTION,
    FOSSIL_AI_CHAT_INTENT_COMMAND,
    FOSSIL_AI_CHAT_INTENT_STATEMENT,
    FOSSIL_AI_CHAT_INTENT_SOCIAL,
    FOSSIL_AI_CHAT_INTENT_RELATIONSHIP
} fossil_ai_chat_intent_t;

// ======================================================
// Risk Classification (Hard Blocks)
// ======================================================

typedef enum {
    FOSSIL_AI_CHAT_RISK_NONE = 0,
    FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT,
    FOSSIL_AI_CHAT_RISK_RELATIONSHIP,
    FOSSIL_AI_CHAT_RISK_DEPENDENCY,
    FOSSIL_AI_CHAT_RISK_SECURITY,
    FOSSIL_AI_CHAT_RISK_RELIGION,
    FOSSIL_AI_CHAT_RISK_UNSUPPORTED_LANGUAGE
} fossil_ai_chat_risk_t;

// ======================================================
// Persistent Memory (FACTUAL ONLY)
// ======================================================

typedef enum {
    FOSSIL_AI_CHAT_MEMORY_FACT = 1,
    FOSSIL_AI_CHAT_MEMORY_TASK,
    FOSSIL_AI_CHAT_MEMORY_SYSTEM
} fossil_ai_chat_memory_type_t;

typedef struct {
    fossil_ai_chat_memory_type_t type;
    float embedding[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    int64_t timestamp;
} fossil_ai_chat_persistent_memory_t;

// ======================================================
// Session Management
// ======================================================

/**
 * Starts a new chat session within the fossil AI jellyfish context.
 *
 * This function initializes and returns a pointer to a new chat session context
 * associated with the specified session ID. The session context can be used to
 * manage and interact with the chat session, including sending and receiving messages,
 * maintaining session state, and handling session-specific resources.
 *
 * @param session_id A null-terminated string representing the unique identifier for the chat session.
 *                   This ID is used to distinguish between different chat sessions and should be unique
 *                   for each session.
 *
 * @return A pointer to a newly allocated fossil_ai_jellyfish_context_t structure representing the chat session context.
 *         Returns NULL if the session could not be started due to invalid parameters or internal errors.
 *
 * @note The returned context must be properly managed and released when no longer needed to avoid memory leaks.
 *       Thread safety and concurrent access to the session context should be handled by the caller if required.
 */
fossil_ai_jellyfish_context_t *
fossil_ai_chat_start_session(const char *session_id);

/**
 * @brief Ends the current chat session in the fossil AI context.
 *
 * This function finalizes and cleans up resources associated with the ongoing chat session
 * within the provided fossil_ai_jellyfish_context_t context. It should be called when the
 * chat interaction is complete to ensure proper resource management and to prevent memory leaks.
 *
 * @param ctx Pointer to a fossil_ai_jellyfish_context_t structure representing the current chat session context.
 *
 * @note After calling this function, the context should not be used for further chat operations
 * unless reinitialized.
 */
void
fossil_ai_chat_end_session(fossil_ai_jellyfish_context_t *ctx);

// ======================================================
// Chat
// ======================================================

/**
 * @brief Generates a response to a user message using the specified AI model and context.
 *
 * This function processes the given user message using the provided jellyfish AI model and context,
 * and writes the generated response into the supplied buffer. The response will be truncated if it
 * exceeds the specified buffer length.
 *
 * @param model         Pointer to the jellyfish AI model used for generating the response.
 * @param ctx           Pointer to the jellyfish AI context containing conversation state and parameters.
 * @param user_message  Null-terminated string containing the user's input message.
 * @param response      Pointer to a buffer where the generated response will be stored.
 * @param response_len  Size of the response buffer in bytes.
 *
 * @return true if a response was successfully generated and stored in the buffer; false otherwise.
 *
 * @note The response buffer must be large enough to hold the generated response, including the null terminator.
 *       If the response is too long, it will be truncated to fit the buffer.
 */
bool
fossil_ai_chat_respond(fossil_ai_jellyfish_model_t *model,
                       fossil_ai_jellyfish_context_t *ctx,
                       const char *user_message,
                       char *response,
                       size_t response_len);

// ======================================================
// Persistent Memory I/O
// ======================================================

/**
 * @brief Saves the persistent state of a jellyfish model to a specified file path.
 *
 * This function serializes and writes the internal state of the provided
 * fossil_ai_jellyfish_model_t instance to the file located at the given path.
 * The persistent state may include model parameters, weights, configuration,
 * or any other relevant data required to restore the model in future sessions.
 *
 * @param model Pointer to the jellyfish model to be saved. Must not be NULL.
 * @param path Path to the file where the model's persistent state will be saved.
 *             The path must be valid and writable.
 *
 * @return true if the model was successfully saved; false otherwise.
 *
 * @note The function does not guarantee atomicity. If the operation fails,
 *       the file at the specified path may be incomplete or corrupted.
 *       It is recommended to check the return value and handle errors appropriately.
 */
bool
fossil_ai_chat_save_persistent(const fossil_ai_jellyfish_model_t *model,
                               const char *path);

/**
 * @brief Loads persistent chat data into the specified jellyfish model.
 *
 * This function attempts to load chat-related persistent data from the file
 * located at the given path and initializes the provided jellyfish model
 * structure with the loaded data. This is typically used to restore the
 * state of a chat session or model from a previous run, enabling continuity
 * and persistence in AI-driven chat applications.
 *
 * @param model Pointer to a fossil_ai_jellyfish_model_t structure that will be
 *              populated with the loaded persistent data.
 * @param path  Path to the file containing the persistent chat data.
 *
 * @return true if the data was loaded successfully and the model was initialized;
 *         false if loading failed or the data was invalid.
 */
bool
fossil_ai_chat_load_persistent(fossil_ai_jellyfish_model_t *model,
                               const char *path);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace fossil {

    namespace ai {
    
        class Chat {
        public:
            /**
             * @brief Constructs a Chat session with the given model and session ID.
             *
             * Initializes the chat context and starts a new session using the provided
             * fossil_ai_jellyfish_model_t pointer and session identifier. Throws a
             * std::runtime_error if the model pointer is null or if the session fails to start.
             *
             * @param model Pointer to the fossil_ai_jellyfish_model_t model to use for chat.
             * @param session_id Unique identifier for the chat session.
             * @throws std::runtime_error if the model is null or session fails to start.
             */
            Chat(fossil_ai_jellyfish_model_t* model, const std::string& session_id)
                : model_(model) {
                if (!model_) throw std::runtime_error("Model is null");
                ctx_ = fossil_ai_chat_start_session(session_id.c_str());
                if (!ctx_) throw std::runtime_error("Failed to start session");
            }

            /**
             * @brief Destructor for the Chat session.
             *
             * Ends the chat session and cleans up the context resources. Ensures that
             * fossil_ai_chat_end_session is called if the context is valid, preventing
             * memory leaks and dangling pointers.
             */
            ~Chat() {
                if (ctx_) {
                    fossil_ai_chat_end_session(ctx_);
                    ctx_ = nullptr;
                }
            }

            /**
             * @brief Generates a response to a user message using the chat model.
             *
             * Processes the given user message using the underlying AI model and chat context,
             * and returns the generated response as a std::string. Throws std::runtime_error
             * if the response generation fails.
             *
             * @param user_message The message from the user to respond to.
             * @return std::string The response generated by the model.
             * @throws std::runtime_error if responding fails.
             */
            std::string respond(const std::string& user_message) {
                char buf[FOSSIL_AI_CHAT_MAX_RESPONSE] = {0};
                if (!fossil_ai_chat_respond(model_, ctx_, user_message.c_str(), buf, sizeof(buf))) {
                    throw std::runtime_error("Respond failed");
                }
                return std::string(buf);
            }

            /**
             * @brief Saves the persistent factual memory of the chat model to a file.
             *
             * Serializes and writes the persistent state of the chat model to the specified
             * file path. Returns true if saving was successful, false otherwise.
             *
             * @param path The file path where the memory should be saved.
             * @return true if saving was successful, false otherwise.
             */
            bool save_persistent(const std::string& path) {
                return fossil_ai_chat_save_persistent(model_, path.c_str());
            }

            /**
             * @brief Loads persistent factual memory into the chat model from a file.
             *
             * Loads and initializes the chat model's persistent memory from the specified
             * file path. Returns true if loading was successful, false otherwise.
             *
             * @param path The file path from which to load the memory.
             * @return true if loading was successful, false otherwise.
             */
            bool load_persistent(const std::string& path) {
                return fossil_ai_chat_load_persistent(model_, path.c_str());
            }
        
        private:
            fossil_ai_jellyfish_model_t* model_;
            fossil_ai_jellyfish_context_t* ctx_;
        };
    
    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
