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
#ifndef FOSSIL_JELLYFISH_LANG_H
#define FOSSIL_JELLYFISH_LANG_H

#include "jellyfish.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ======================================================
// Limits
// ======================================================

#define FOSSIL_AI_NLP_MAX_TOKENS       512
#define FOSSIL_AI_NLP_MAX_TOKEN_LEN    64
#define FOSSIL_AI_NLP_MAX_NGRAM_ORDER  5
#define FOSSIL_AI_NLP_MAX_PARAGRAPHS   32

// ======================================================
// Tokenization / Normalization
// ======================================================

size_t fossil_ai_nlp_tokenize(const char *text,
                              char tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN]);

bool fossil_ai_nlp_is_american_english(const char *text);

// ======================================================
// N-gram frequency
// ======================================================

typedef struct {
    size_t count; // frequency
    char ngram[FOSSIL_AI_NLP_MAX_NGRAM_ORDER][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
} fossil_ai_nlp_ngram_t;

typedef struct {
    fossil_ai_nlp_ngram_t *ngrams;
    size_t ngram_count;
    size_t order;
} fossil_ai_nlp_ngram_table_t;

void fossil_ai_nlp_build_ngram_table(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                     size_t token_count,
                                     size_t order,
                                     fossil_ai_nlp_ngram_table_t *table);

void fossil_ai_nlp_free_ngram_table(fossil_ai_nlp_ngram_table_t *table);

// ======================================================
// Markov text generation (temperature controlled)
// ======================================================

bool fossil_ai_nlp_generate_markov(const fossil_ai_nlp_ngram_table_t *table,
                                   size_t length,
                                   char output[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                   size_t *out_count,
                                   float temperature);

// ======================================================
// Tone / Sentiment Analysis (per paragraph drift)
// ======================================================

typedef enum {
    FOSSIL_AI_NLP_TONE_NEUTRAL = 0,
    FOSSIL_AI_NLP_TONE_POSITIVE,
    FOSSIL_AI_NLP_TONE_NEGATIVE
} fossil_ai_nlp_tone_t;

fossil_ai_nlp_tone_t fossil_ai_nlp_detect_tone(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                               size_t token_count);

float fossil_ai_nlp_detect_tone_drift(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                      size_t token_count,
                                      size_t paragraphs);

// ======================================================
// Persistent Contextual Embeddings
// ======================================================

bool fossil_ai_nlp_embed(fossil_ai_jellyfish_model_t *model,
                         const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                         size_t token_count,
                         float *embedding);

#ifdef __cplusplus
}

#include <vector>
#include <string>
#include <array>
#include <cstring>

namespace fossil {

    namespace ai {

        class NLP {
        public:
            NLP() = default;
            ~NLP() = default;
        
            // ------------------------------------------------------
            // Tokenization / Normalization
            // ------------------------------------------------------
            size_t tokenize(const std::string &text,
                            std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &tokens) const {
                static char c_tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                size_t count = fossil_ai_nlp_tokenize(text.c_str(), c_tokens);
                tokens.clear();
                for (size_t i = 0; i < count; i++) {
                    std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN> arr{};
                    std::memcpy(arr.data(), c_tokens[i], FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                    tokens.push_back(arr);
                }
                return count;
            }
        
            bool is_american_english(const std::string &text) const {
                return fossil_ai_nlp_is_american_english(text.c_str());
            }
        
            // ------------------------------------------------------
            // N-gram Table
            // ------------------------------------------------------
            struct NGramTable {
                fossil_ai_nlp_ngram_table_t table;
        
                NGramTable() {
                    table.ngrams = nullptr;
                    table.ngram_count = 0;
                    table.order = 0;
                }
        
                ~NGramTable() {
                    fossil_ai_nlp_free_ngram_table(&table);
                }
        
                void build(const std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &tokens, size_t order) {
                    char c_tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                    size_t n = std::min(tokens.size(), static_cast<size_t>(FOSSIL_AI_NLP_MAX_TOKENS));
                    for (size_t i = 0; i < n; i++)
                        std::memcpy(c_tokens[i], tokens[i].data(), FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                    fossil_ai_nlp_build_ngram_table(c_tokens, n, order, &table);
                }
            };
        
            // ------------------------------------------------------
            // Markov generation
            // ------------------------------------------------------
            bool generate_markov(const NGramTable &ngram_table,
                                 size_t length,
                                 std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &output,
                                 float temperature = 1.0f) const {
                char c_output[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                size_t out_count = 0;
                bool ok = fossil_ai_nlp_generate_markov(&ngram_table.table, length, c_output, &out_count, temperature);
                output.clear();
                for (size_t i = 0; i < out_count; i++) {
                    std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN> arr{};
                    std::memcpy(arr.data(), c_output[i], FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                    output.push_back(arr);
                }
                return ok;
            }
        
            // ------------------------------------------------------
            // Tone analysis
            // ------------------------------------------------------
            fossil_ai_nlp_tone_t detect_tone(const std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &tokens) const {
                char c_tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                size_t n = std::min(tokens.size(), static_cast<size_t>(FOSSIL_AI_NLP_MAX_TOKENS));
                for (size_t i = 0; i < n; i++)
                    std::memcpy(c_tokens[i], tokens[i].data(), FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                return fossil_ai_nlp_detect_tone(c_tokens, n);
            }
        
            float detect_tone_drift(const std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &tokens,
                                    size_t paragraphs) const {
                char c_tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                size_t n = std::min(tokens.size(), static_cast<size_t>(FOSSIL_AI_NLP_MAX_TOKENS));
                for (size_t i = 0; i < n; i++)
                    std::memcpy(c_tokens[i], tokens[i].data(), FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                return fossil_ai_nlp_detect_tone_drift(c_tokens, n, paragraphs);
            }
        
            // ------------------------------------------------------
            // Persistent embeddings
            // ------------------------------------------------------
            bool embed(fossil_ai_jellyfish_model_t *model,
                       const std::vector<std::array<char, FOSSIL_AI_NLP_MAX_TOKEN_LEN>> &tokens,
                       float *embedding) const {
                char c_tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN];
                size_t n = std::min(tokens.size(), static_cast<size_t>(FOSSIL_AI_NLP_MAX_TOKENS));
                for (size_t i = 0; i < n; i++)
                    std::memcpy(c_tokens[i], tokens[i].data(), FOSSIL_AI_NLP_MAX_TOKEN_LEN);
                return fossil_ai_nlp_embed(model, c_tokens, n, embedding);
            }
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* AI_FRAMEWORK_H */
