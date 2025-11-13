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
#ifndef FOSSIL_JELLYFISH_SEEK_H
#define FOSSIL_JELLYFISH_SEEK_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Constants and Limits
// ======================================================

#define FOSSIL_AI_SEEK_MAX_RESULTS     16
#define FOSSIL_AI_SEEK_DESC_SIZE       512
#define FOSSIL_AI_SEEK_SYMBOL_SIZE     128
#define FOSSIL_AI_SEEK_SECTION_SIZE     32
#define FOSSIL_AI_SEEK_PATH_SIZE       256

// ======================================================
// Seek Result Structures
// ======================================================

/**
 * @struct fossil_ai_seek_result
 * @brief Represents a single lookup result from the Seek subsystem.
 */
typedef struct fossil_ai_seek_result {
    char path[FOSSIL_AI_SEEK_PATH_SIZE];         ///< Source path (manpage, header, etc.)
    char section[FOSSIL_AI_SEEK_SECTION_SIZE];   ///< Optional manpage section identifier
    char symbol[FOSSIL_AI_SEEK_SYMBOL_SIZE];     ///< Command, function, or topic name
    char description[FOSSIL_AI_SEEK_DESC_SIZE];  ///< Short extracted summary or synopsis
    char *content;                               ///< Allocated buffer containing full text
    size_t content_len;                          ///< Length of the content buffer
    double relevance;                            ///< Confidence or similarity score (0.0–1.0)
} fossil_ai_seek_result_t;

/**
 * @struct fossil_ai_seek_results
 * @brief Container for multiple Seek results.
 */
typedef struct fossil_ai_seek_results {
    fossil_ai_seek_result_t results[FOSSIL_AI_SEEK_MAX_RESULTS];
    size_t count;                                ///< Number of valid results
} fossil_ai_seek_results_t;

// ======================================================
// Initialization and Configuration
// ======================================================

/**
 * @brief Initialize the Seek subsystem and prepare default search paths.
 *
 * This must be called once before using any Seek functions. It sets up internal
 * buffers and registers known documentation directories, such as man page roots
 * or project-specific documentation locations.
 */
void fossil_ai_seek_init(void);

/**
 * @brief Add an additional documentation root for Seek queries.
 * @param path Root directory containing man pages or project documentation.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_seek_add_path(ccstring path);

// ======================================================
// Search and Lookup Functions
// ======================================================

/**
 * @brief Search for a command or topic in system man pages.
 * @param query The command, topic, or keyword to search for.
 * @param out_results Result container (cleared and filled on success).
 * @return Number of matches found, or negative error code.
 */
int fossil_ai_seek_man(ccstring query, fossil_ai_seek_results_t *out_results);

/**
 * @brief Search for a function, type, or symbol in source headers or Markdown docs.
 * @param symbol The function or symbol name.
 * @param source_root Root directory to search (e.g., "src/" or "include/").
 * @param out_results Result container.
 * @return Number of matches found, or negative error code.
 */
int fossil_ai_seek_api(ccstring symbol, ccstring source_root, fossil_ai_seek_results_t *out_results);

/**
 * @brief Perform a generic keyword search through documentation directories.
 * @param query Keyword or phrase to match.
 * @param root_dir Root directory containing text documentation.
 * @param out_results Result container.
 * @return Number of matches found, or negative error code.
 */
int fossil_ai_seek_docs(ccstring query, ccstring root_dir, fossil_ai_seek_results_t *out_results);

/**
 * @brief Unified lookup combining manpage, API, and doc searches.
 * @param chain Optional Jellyfish chain for caching results.
 * @param query Lookup term or symbol.
 * @param best_result Output for best match (highest relevance).
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_seek_lookup(fossil_ai_jellyfish_chain_t *chain,
                          ccstring query,
                          fossil_ai_seek_result_t *best_result);

// ======================================================
// Result Processing and Integration
// ======================================================

/**
 * @brief Summarize a Seek result into a short text answer.
 * @param result Pointer to the Seek result.
 * @param out Output buffer for summary text.
 * @param size Size of output buffer.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_seek_summarize(const fossil_ai_seek_result_t *result, char *out, size_t size);

/**
 * @brief Record a Seek result into the Jellyfish chain as a factual commit.
 * @param chain Target Jellyfish chain.
 * @param result Seek result to record.
 * @param type Commit type (e.g., OBSERVE, VALIDATE).
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_seek_record(fossil_ai_jellyfish_chain_t *chain,
                          const fossil_ai_seek_result_t *result,
                          fossil_ai_jellyfish_commit_type_t type);

/**
 * @brief Compute a fuzzy similarity score between a query and a Seek result.
 * @param query Input query string.
 * @param result Candidate result.
 * @return Similarity score (0.0–1.0).
 */
double fossil_ai_seek_similarity(ccstring query, const fossil_ai_seek_result_t *result);

// ======================================================
// Cleanup
// ======================================================

/**
 * @brief Free dynamically allocated content in a Seek result.
 * @param result Result to free.
 */
void fossil_ai_seek_free(fossil_ai_seek_result_t *result);

/**
 * @brief Release all Seek-related resources and caches.
 */
void fossil_ai_seek_shutdown(void);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

    namespace ai {



    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */