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

/**
 * @brief Handles slash commands for the IoChat API session.
 *
 * This function parses and executes various slash commands entered by the user in the chat interface.
 * Slash commands are prefixed with '/' and provide administrative, informational, and session management capabilities.
 * The function processes the input command, interacts with the session state (represented by `fossil_ai_jellyfish_chain_t`),
 * and writes the result or response to the provided output buffer.
 *
 * Supported Slash Commands:
 * - `/summary` : Summarizes the current chat session. If no summary is available, an appropriate message is returned.
 * - `/turns`   : Displays the total number of turns/messages exchanged in the session.
 * - `/trust`   : Shows the current trust score of the session's chain, indicating reliability or confidence.
 * - `/end`     : Requests to end the current session. Returns a special code to signal session termination.
 * - `/help`    : Lists all available slash commands and their usage.
 * - `/export <filepath>` : Exports the session history to the specified file path. Reports success or failure.
 * - `/import <filepath>` : Imports context from the specified file path into the session. Reports success or failure.
 * - `/inject <message>`  : Injects a system message into the session, useful for administrative or debugging purposes.
 * - `/learn <input>|<output>` : Teaches the session a new response mapping. The input and output are separated by '|'.
 *
 * Command Handling:
 * - Each command is matched using `strncmp` for efficiency.
 * - Arguments are parsed and validated for commands that require them (e.g., `/export`, `/import`, `/inject`, `/learn`).
 * - Output is safely written to the buffer, ensuring null-termination and size constraints.
 * - Unknown commands return an informative error message.
 *
 * Return Values:
 * - `0` : Input is not a slash command.
 * - `1` : Command processed successfully (or with an error message).
 * - `2` : Special code indicating session end request (`/end` command).
 *
 * @param input  The user input string, expected to start with '/' for commands.
 * @param output Buffer to write the command response or result.
 * @param size   Size of the output buffer.
 * @param chain  Pointer to the session's jellyfish chain structure.
 * @return int   Status code indicating command handling result.
 *
 * @note
 * This API is designed to be extensible. New slash commands can be added following the existing pattern.
 * All commands are documented in the `/help` output for user reference.
 */

// *****************************************************************************
// Function prototypes
// *****************************************************************************

/**
 * @brief Starts a new conversation session.
 *
 * Initializes session context, sets up logging, and records system block.
 *
 * @param context_name Optional name for the context/session.
 * @param chain Pointer to Jellyfish chain.
 * @return 0 on success, non-zero on failure.
 */
int fossil_ai_iochat_start(const char *context_name, fossil_ai_jellyfish_chain_t *chain);

/**
 * @brief Processes a user input and generates a chatbot response.
 *
 * Handles slash commands, logs events, and learns new responses if appropriate.
 *
 * @param chain   Pointer to Jellyfish chain.
 * @param input   User input string.
 * @param output  Output buffer to receive response.
 * @param size    Size of output buffer.
 * @return        0 if response found, -1 if unknown, 1 if session end requested.
 */
int fossil_ai_iochat_respond(fossil_ai_jellyfish_chain_t *chain, const char *input, char *output, size_t size);

/**
 * @brief Ends the current conversation session and performs cleanup.
 *
 * Logs session end, verifies chain, records system block, and closes log file.
 *
 * @param chain Pointer to Jellyfish chain.
 * @return 0 on success.
 */
int fossil_ai_iochat_end(fossil_ai_jellyfish_chain_t *chain);

/**
 * @brief Injects a system message into the chain (e.g. "Hello", "System Ready").
 *
 * Adds an immutable system block and logs the event.
 *
 * @param chain  Jellyfish chain.
 * @param message System-level message.
 * @return 0 on success, -1 on error.
 */
int fossil_ai_iochat_inject_system_message(fossil_ai_jellyfish_chain_t *chain, const char *message);

/**
 * @brief Learns a new response based on user input and chatbot output.
 *
 * Adds a new input/output pair to the chain if no conflict exists.
 *
 * @param chain   Chain to add memory to.
 * @param input   Original user input.
 * @param output  Chatbot response to learn.
 * @return 0 on success, -1 on error.
 */
int fossil_ai_iochat_learn_response(fossil_ai_jellyfish_chain_t *chain, const char *input, const char *output);

/**
 * @brief Returns the number of conversational turns remembered.
 *
 * Counts valid user turns (excluding system blocks).
 *
 * @param chain Jellyfish chain.
 * @return Number of user-input/output pairs.
 */
int fossil_ai_iochat_turn_count(const fossil_ai_jellyfish_chain_t *chain);

/**
 * @brief Summarizes the session into a concise text form.
 *
 * Concatenates user turns into a summary paragraph.
 *
 * @param chain     Jellyfish chain to summarize.
 * @param summary   Output buffer to store summary.
 * @param size      Size of the output buffer.
 * @return 0 on success, -1 if summary couldn't be generated.
 */
int fossil_ai_iochat_summarize_session(const fossil_ai_jellyfish_chain_t *chain, char *summary, size_t size);

/**
 * @brief Filters the most recent N turns into a temporary sub-chain.
 *
 * Copies recent valid user turns into a new chain.
 *
 * @param chain     Original chat chain.
 * @param out_chain Output chain filled with most recent turns.
 * @param turn_count Number of recent user turns to include.
 * @return 0 on success, -1 on error.
 */
int fossil_ai_iochat_filter_recent(const fossil_ai_jellyfish_chain_t *chain, fossil_ai_jellyfish_chain_t *out_chain, int turn_count);

/**
 * @brief Exports the current conversation history to a text file.
 *
 * Saves the chain to the specified file path.
 *
 * @param chain     Chain to serialize.
 * @param filepath  Destination path for output.
 * @return 0 on success, -1 on error.
 */
int fossil_ai_iochat_export_history(const fossil_ai_jellyfish_chain_t *chain, const char *filepath);

/**
 * @brief Imports a context file and loads it into the chain.
 *
 * Loads chain data from the specified file path.
 *
 * @param chain     Destination Jellyfish chain.
 * @param filepath  Source path of saved context.
 * @return 0 on success, -1 if parsing fails.
 */
int fossil_ai_iochat_import_context(fossil_ai_jellyfish_chain_t *chain, const char *filepath);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

    namespace ai {

        class IOChat {
        public:
            /**
             * @brief Starts a new conversation session.
             *
             * Initializes session context, sets up logging, and records system block.
             *
             * @param context_name Optional name for the context/session.
             * @param chain Pointer to Jellyfish chain.
             * @return 0 on success, non-zero on failure.
             */
            static int start(const char *context_name, fossil_ai_jellyfish_chain_t *chain) {
            return fossil_ai_iochat_start(context_name, chain);
            }

            /**
             * @brief Processes a user input and generates a chatbot response.
             *
             * Handles slash commands, logs events, and learns new responses if appropriate.
             *
             * @param chain   Pointer to Jellyfish chain.
             * @param input   User input string.
             * @param output  Output buffer to receive response.
             * @param size    Size of output buffer.
             * @return        0 if response found, -1 if unknown, 1 if session end requested.
             */
            static int respond(fossil_ai_jellyfish_chain_t *chain, const char *input, char *output, size_t size) {
            return fossil_ai_iochat_respond(chain, input, output, size);
            }

            /**
             * @brief Ends the current conversation session and performs cleanup.
             *
             * Logs session end, verifies chain, records system block, and closes log file.
             *
             * @param chain Pointer to Jellyfish chain.
             * @return 0 on success.
             */
            static int end(fossil_ai_jellyfish_chain_t *chain) {
            return fossil_ai_iochat_end(chain);
            }

            /**
             * @brief Injects a system message into the chain (e.g. "Hello", "System Ready").
             *
             * Adds an immutable system block and logs the event.
             *
             * @param chain  Jellyfish chain.
             * @param message System-level message.
             * @return 0 on success, -1 on error.
             */
            static int inject_system_message(fossil_ai_jellyfish_chain_t *chain, const char *message) {
            return fossil_ai_iochat_inject_system_message(chain, message);
            }

            /**
             * @brief Learns a new response based on user input and chatbot output.
             *
             * Adds a new input/output pair to the chain if no conflict exists.
             *
             * @param chain   Chain to add memory to.
             * @param input   Original user input.
             * @param output  Chatbot response to learn.
             * @return 0 on success, -1 on error.
             */
            static int learn_response(fossil_ai_jellyfish_chain_t *chain, const char *input, const char *output) {
            return fossil_ai_iochat_learn_response(chain, input, output);
            }

            /**
             * @brief Returns the number of conversational turns remembered.
             *
             * Counts valid user turns (excluding system blocks).
             *
             * @param chain Jellyfish chain.
             * @return Number of user-input/output pairs.
             */
            static int turn_count(const fossil_ai_jellyfish_chain_t *chain) {
            return fossil_ai_iochat_turn_count(chain);
            }

            /**
             * @brief Summarizes the session into a concise text form.
             *
             * Concatenates user turns into a summary paragraph.
             *
             * @param chain     Jellyfish chain to summarize.
             * @param summary   Output buffer to store summary.
             * @param size      Size of the output buffer.
             * @return 0 on success, -1 if summary couldn't be generated.
             */
            static int summarize_session(const fossil_ai_jellyfish_chain_t *chain, char *summary, size_t size) {
            return fossil_ai_iochat_summarize_session(chain, summary, size);
            }

            /**
             * @brief Filters the most recent N turns into a temporary sub-chain.
             *
             * Copies recent valid user turns into a new chain.
             *
             * @param chain     Original chat chain.
             * @param out_chain Output chain filled with most recent turns.
             * @param turn_count Number of recent user turns to include.
             * @return 0 on success, -1 on error.
             */
            static int filter_recent(const fossil_ai_jellyfish_chain_t *chain, fossil_ai_jellyfish_chain_t *out_chain, int turn_count) {
            return fossil_ai_iochat_filter_recent(chain, out_chain, turn_count);
            }

            /**
             * @brief Exports the current conversation history to a text file.
             *
             * Saves the chain to the specified file path.
             *
             * @param chain     Chain to serialize.
             * @param filepath  Destination path for output.
             * @return 0 on success, -1 on error.
             */
            static int export_history(const fossil_ai_jellyfish_chain_t *chain, const char *filepath) {
            return fossil_ai_iochat_export_history(chain, filepath);
            }

            /**
             * @brief Imports a context file and loads it into the chain.
             *
             * Loads chain data from the specified file path.
             *
             * @param chain     Destination Jellyfish chain.
             * @param filepath  Source path of saved context.
             * @return 0 on success, -1 if parsing fails.
             */
            static int import_context(fossil_ai_jellyfish_chain_t *chain, const char *filepath) {
            return fossil_ai_iochat_import_context(chain, filepath);
            }
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
