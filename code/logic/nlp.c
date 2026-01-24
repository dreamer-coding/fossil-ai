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
#include "fossil/ai/nlp.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

// ======================================================
// Tokenization / Normalization
// ======================================================

size_t fossil_ai_nlp_tokenize(const char *text,
                              char tokens[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN]) {
    char buf[2048];
    size_t len = 0;
    for (; *text && len < sizeof(buf) - 1; text++) {
        if (isalpha((unsigned char)*text) || isspace((unsigned char)*text))
            buf[len++] = (char)tolower(*text);
        else
            buf[len++] = ' ';
    }
    buf[len] = '\0';

    size_t count = 0;
    char *p = buf;
    while (*p && count < FOSSIL_AI_NLP_MAX_TOKENS) {
        while (*p == ' ') p++;
        if (!*p) break;

        size_t i = 0;
        while (*p && *p != ' ' && i < FOSSIL_AI_NLP_MAX_TOKEN_LEN - 1)
            tokens[count][i++] = *p++;
        tokens[count][i] = '\0';
        count++;
    }
    return count;
}

bool fossil_ai_nlp_is_american_english(const char *text) {
    for (; *text; text++)
        if (!(*text >= 32 && *text <= 126))
            return false;
    return true;
}

// ======================================================
// N-gram table with frequency
// ======================================================

void fossil_ai_nlp_build_ngram_table(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                     size_t token_count,
                                     size_t order,
                                     fossil_ai_nlp_ngram_table_t *table) {
    if (!table || order == 0 || token_count < order) return;

    table->order = order;
    table->ngram_count = 0;
    table->ngrams = calloc(token_count, sizeof(fossil_ai_nlp_ngram_t));

    for (size_t i = 0; i <= token_count - order; i++) {
        bool found = false;
        for (size_t j = 0; j < table->ngram_count; j++) {
            bool match = true;
            for (size_t k = 0; k < order; k++) {
                if (strcmp(tokens[i+k], table->ngrams[j].ngram[k]) != 0) {
                    match = false;
                    break;
                }
            }
            if (match) {
                table->ngrams[j].count++;
                found = true;
                break;
            }
        }
        if (!found) {
            fossil_ai_nlp_ngram_t *ng = &table->ngrams[table->ngram_count++];
            ng->count = 1;
            for (size_t k = 0; k < order; k++)
                strncpy(ng->ngram[k], tokens[i+k], FOSSIL_AI_NLP_MAX_TOKEN_LEN);
        }
    }
}

void fossil_ai_nlp_free_ngram_table(fossil_ai_nlp_ngram_table_t *table) {
    if (!table) return;
    free(table->ngrams);
    table->ngrams = NULL;
    table->ngram_count = 0;
}

// ======================================================
// Markov text generation with temperature
// ======================================================

bool fossil_ai_nlp_generate_markov(const fossil_ai_nlp_ngram_table_t *table,
                                   size_t length,
                                   char output[FOSSIL_AI_NLP_MAX_TOKENS][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                   size_t *out_count,
                                   float temperature) {
    if (!table || !output || length == 0 || !out_count) return false;
    if (table->ngram_count == 0) return false;

    srand((unsigned int)time(NULL));
    size_t out_idx = 0;

    // Seed with a random n-gram
    size_t idx = rand() % table->ngram_count;
    for (size_t i = 0; i < table->order && out_idx < length; i++)
        strncpy(output[out_idx++], table->ngrams[idx].ngram[i], FOSSIL_AI_NLP_MAX_TOKEN_LEN);

    while (out_idx < length) {
        // Temperature-weighted selection
        float total = 0.0f;
        for (size_t i = 0; i < table->ngram_count; i++)
            total += powf((float)table->ngrams[i].count, 1.0f / fmaxf(temperature, 0.01f));

        float r = ((float)rand() / RAND_MAX) * total;
        float sum = 0.0f;
        size_t selected = 0;
        for (size_t i = 0; i < table->ngram_count; i++) {
            sum += powf((float)table->ngrams[i].count, 1.0f / fmaxf(temperature, 0.01f));
            if (sum >= r) {
                selected = i;
                break;
            }
        }
        strncpy(output[out_idx++], table->ngrams[selected].ngram[table->order - 1],
                FOSSIL_AI_NLP_MAX_TOKEN_LEN);
    }

    *out_count = out_idx;
    return true;
}

// ======================================================
// Tone analysis and per-paragraph drift
// ======================================================

fossil_ai_nlp_tone_t fossil_ai_nlp_detect_tone(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                               size_t token_count) {
    static const char *POS[] = {"good","great","excellent","happy","success"};
    static const char *NEG[] = {"bad","sad","terrible","fail","wrong"};

    int pos = 0, neg = 0;
    for (size_t i = 0; i < token_count; i++) {
        for (size_t j = 0; j < sizeof(POS)/sizeof(POS[0]); j++)
            if (strcmp(tokens[i], POS[j]) == 0) pos++;
        for (size_t j = 0; j < sizeof(NEG)/sizeof(NEG[0]); j++)
            if (strcmp(tokens[i], NEG[j]) == 0) neg++;
    }
    if (pos > neg) return FOSSIL_AI_NLP_TONE_POSITIVE;
    if (neg > pos) return FOSSIL_AI_NLP_TONE_NEGATIVE;
    return FOSSIL_AI_NLP_TONE_NEUTRAL;
}

float fossil_ai_nlp_detect_tone_drift(const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                                      size_t token_count,
                                      size_t paragraphs) {
    if (paragraphs == 0 || token_count == 0) return 0.0f;

    size_t per_para = token_count / paragraphs;
    int last_score = 0;
    float total_drift = 0.0f;

    for (size_t p = 0; p < paragraphs; p++) {
        size_t start = p * per_para;
        size_t end = (p == paragraphs -1) ? token_count : start + per_para;
        fossil_ai_nlp_tone_t tone = fossil_ai_nlp_detect_tone(tokens + start, end - start);
        int score = (tone == FOSSIL_AI_NLP_TONE_POSITIVE) ? 1 :
                    (tone == FOSSIL_AI_NLP_TONE_NEGATIVE) ? -1 : 0;
        if (p > 0) total_drift += fabsf((float)(score - last_score));
        last_score = score;
    }
    return total_drift;
}

// ======================================================
// Contextual embeddings
// ======================================================

bool fossil_ai_nlp_embed(fossil_ai_jellyfish_model_t *model,
                         const char tokens[][FOSSIL_AI_NLP_MAX_TOKEN_LEN],
                         size_t token_count,
                         float *embedding) {
    if (!model || !embedding) return false;
    memset(embedding, 0, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);

    for (size_t t = 0; t < token_count && t < FOSSIL_AI_JELLYFISH_EMBED_SIZE; t++) {
        size_t len = strlen(tokens[t]);
        float sum = 0.0f;
        for (size_t i = 0; i < len; i++)
            sum += (float)(tokens[t][i]) / 128.0f;
        embedding[t] = sum / (float)fmaxf(len, 1);
    }

    // Store embedding in safe memory
    if (model->memory_len < FOSSIL_AI_JELLYFISH_MAX_MEMORY) {
        fossil_ai_jellyfish_memory_t *m = &model->memory[model->memory_len++];
        memcpy(m->embedding, embedding, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
        m->timestamp = (int64_t)time(NULL);
    }

    return true;
}
