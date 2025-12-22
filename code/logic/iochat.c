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
#include "fossil/ai/iochat.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

// ======================================================
// Hash Helpers
// ======================================================

static unsigned int hash_string(const char *s) {
    unsigned int h = 5381;
    while (*s) h = ((h << 5) + h) + (unsigned char)(*s++);
    return h % FOSSIL_AI_CHAT_HASH_SIZE;
}

static void init_hash(const char **terms, size_t term_count, unsigned char hash_table[FOSSIL_AI_CHAT_HASH_SIZE]) {
    for (size_t i = 0; i < term_count; i++) {
        unsigned int h = hash_string(terms[i]);
        hash_table[h] = 1;
    }
}

static bool hash_lookup(const char *word, unsigned char hash_table[FOSSIL_AI_CHAT_HASH_SIZE]) {
    return hash_table[hash_string(word)] != 0;
}

// ======================================================
// Normalization & Tokenization
// ======================================================

static size_t normalize_and_tokenize(const char *input,
    char tokens[FOSSIL_AI_CHAT_MAX_TOKENS][FOSSIL_AI_CHAT_MAX_TOKEN_LEN]) {

    char buf[1024];
    size_t len = 0;

    for (; *input && len < sizeof(buf) - 1; input++) {
        if (isalpha((unsigned char)*input) || isspace((unsigned char)*input))
            buf[len++] = (char)tolower(*input);
        else
            buf[len++] = ' ';
    }
    buf[len] = '\0';

    size_t count = 0;
    char *p = buf;
    while (*p && count < FOSSIL_AI_CHAT_MAX_TOKENS) {
        while (*p == ' ') p++;
        if (!*p) break;

        size_t i = 0;
        while (*p && *p != ' ' && i < FOSSIL_AI_CHAT_MAX_TOKEN_LEN - 1)
            tokens[count][i++] = *p++;
        tokens[count][i] = '\0';
        count++;
    }
    return count;
}

// ======================================================
// Vocabulary / Misspelling Tolerance
// ======================================================

static const char *VOCAB[] = {
    "hello","hi","yes","no","maybe","sure","i","you","we","they",
    "am","are","can","will","do","what","why","how","when","where",
    "think","know","believe","understand","help","learn","work","build","use",
    "time","today","now","later","good","bad","right","wrong",
    "simple","clear","important","model","system","memory","context"
};

static int levenshtein(const char *s1, const char *s2) {
    size_t len1 = strlen(s1), len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];
    for (size_t i = 0; i <= len1; i++) matrix[i][0] = (int)i;
    for (size_t j = 0; j <= len2; j++) matrix[0][j] = (int)j;
    for (size_t i = 1; i <= len1; i++) {
        for (size_t j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            int del = matrix[i-1][j] + 1;
            int ins = matrix[i][j-1] + 1;
            int sub = matrix[i-1][j-1] + cost;
            matrix[i][j] = del < ins ? (del < sub ? del : sub) : (ins < sub ? ins : sub);
        }
    }
    return matrix[len1][len2];
}

static bool fossil_ai_chat_lookup_vocab(const char *word) {
    size_t n = sizeof(VOCAB)/sizeof(VOCAB[0]);
    for (size_t i = 0; i < n; i++)
        if (strcmp(word, VOCAB[i]) == 0 || levenshtein(word, VOCAB[i]) <= 1)
            return true;
    return false;
}

// ======================================================
// American English Restriction
// ======================================================

static bool fossil_ai_chat_is_american_english(const char *text) {
    for (; *text; text++) {
        unsigned char c = *text;
        if (!(c >= 32 && c <= 126))
            return false;
    }
    return true;
}

// ======================================================
// Semantic Term Buckets
// ======================================================

static const char *EMOTIONAL_TERMS[] = {
    "sad","depressed","lonely","empty","hopeless","hurt","cry","upset","anxious","afraid","scared"
};
static const char *DEPENDENCY_TERMS[] = {
    "need","depend","only","without","lost"
};
static const char *RELATIONSHIP_TERMS[] = {
    "love","relationship","partner","girlfriend","boyfriend","husband","wife","romantic","date"
};
static const char *SECURITY_TERMS[] = {
    "password","secret","key","token","credential","private","exploit","bypass","hack","phish"
};
static const char *RELIGION_TERMS[] = {
    "god","allah","jesus","christ","bible","quran","faith","religion","pray","worship"
};

static unsigned char EMOTIONAL_HASH[FOSSIL_AI_CHAT_HASH_SIZE] = {0};
static unsigned char DEPENDENCY_HASH[FOSSIL_AI_CHAT_HASH_SIZE] = {0};
static unsigned char RELATIONSHIP_HASH[FOSSIL_AI_CHAT_HASH_SIZE] = {0};
static unsigned char SECURITY_HASH[FOSSIL_AI_CHAT_HASH_SIZE] = {0};
static unsigned char RELIGION_HASH[FOSSIL_AI_CHAT_HASH_SIZE] = {0};

static void init_hash_tables() {
    static int inited = 0;
    if (inited) return;
    init_hash(EMOTIONAL_TERMS, sizeof(EMOTIONAL_TERMS)/sizeof(EMOTIONAL_TERMS[0]), EMOTIONAL_HASH);
    init_hash(DEPENDENCY_TERMS, sizeof(DEPENDENCY_TERMS)/sizeof(DEPENDENCY_TERMS[0]), DEPENDENCY_HASH);
    init_hash(RELATIONSHIP_TERMS, sizeof(RELATIONSHIP_TERMS)/sizeof(RELATIONSHIP_TERMS[0]), RELATIONSHIP_HASH);
    init_hash(SECURITY_TERMS, sizeof(SECURITY_TERMS)/sizeof(SECURITY_TERMS[0]), SECURITY_HASH);
    init_hash(RELIGION_TERMS, sizeof(RELIGION_TERMS)/sizeof(RELIGION_TERMS[0]), RELIGION_HASH);
    inited = 1;
}

// ======================================================
// Risk Detection with Hashes
// ======================================================

static fossil_ai_chat_risk_t fossil_ai_chat_detect_risk(const char *text) {
    init_hash_tables();

    char tokens[FOSSIL_AI_CHAT_MAX_TOKENS][FOSSIL_AI_CHAT_MAX_TOKEN_LEN];
    size_t token_count = normalize_and_tokenize(text, tokens);

    for (size_t i = 0; i < token_count; i++)
        if (!fossil_ai_chat_lookup_vocab(tokens[i]))
            return FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT;

    int emotional = 0, dependency = 0, relationship = 0, security = 0, religion = 0;

    for (size_t i = 0; i < token_count; i++) {
        if (hash_lookup(tokens[i], EMOTIONAL_HASH)) emotional++;
        if (hash_lookup(tokens[i], DEPENDENCY_HASH)) dependency++;
        if (hash_lookup(tokens[i], RELATIONSHIP_HASH)) relationship++;
        if (hash_lookup(tokens[i], SECURITY_HASH)) security++;
        if (hash_lookup(tokens[i], RELIGION_HASH)) religion++;
    }

    if (security > 0) return FOSSIL_AI_CHAT_RISK_SECURITY;
    if (religion > 0) return FOSSIL_AI_CHAT_RISK_RELIGION;
    if (relationship > 0) return FOSSIL_AI_CHAT_RISK_RELATIONSHIP;
    if (emotional > 0 && dependency > 0) return FOSSIL_AI_CHAT_RISK_DEPENDENCY;
    if (emotional > 0) return FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT;
    return FOSSIL_AI_CHAT_RISK_NONE;
}

// ======================================================
// Embedding (Deterministic)
// ======================================================

static void fossil_ai_chat_embed(const char *text, float *out) {
    memset(out, 0, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    size_t pos = 0;
    while (*text && pos < FOSSIL_AI_JELLYFISH_EMBED_SIZE) {
        if (isalpha((unsigned char)*text))
            out[pos++] = (float)(tolower(*text) - 'a') / 26.0f;
        text++;
    }
}

// ======================================================
// Session Management
// ======================================================

fossil_ai_jellyfish_context_t *fossil_ai_chat_start_session(const char *session_id) {
    fossil_ai_jellyfish_context_t *ctx = fossil_ai_jellyfish_create_context(session_id);
    ctx->history_len = 0;
    ctx->history = calloc(FOSSIL_AI_CHAT_MAX_HISTORY, sizeof(void *));
    return ctx;
}

void fossil_ai_chat_end_session(fossil_ai_jellyfish_context_t *ctx) {
    if (!ctx) return;
    for (size_t i = 0; i < ctx->history_len; i++) free(ctx->history[i]);
    free(ctx->history);
    fossil_ai_jellyfish_free_context(ctx);
}

// ======================================================
// Persistent Memory I/O
// ======================================================

bool fossil_ai_chat_save_persistent(const fossil_ai_jellyfish_model_t *model, const char *path) {
    FILE *f = fopen(path, "wb");
    if (!f) return false;
    fwrite(&model->persistent_len, sizeof(size_t), 1, f);
    fwrite(model->persistent, sizeof(fossil_ai_chat_persistent_memory_t), model->persistent_len, f);
    fclose(f);
    return true;
}

bool fossil_ai_chat_load_persistent(fossil_ai_jellyfish_model_t *model, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    fread(&model->persistent_len, sizeof(size_t), 1, f);
    fread(model->persistent, sizeof(fossil_ai_chat_persistent_memory_t), model->persistent_len, f);
    fclose(f);
    return true;
}

// ======================================================
// Chat Respond (Hard Blocks + Safe Embedding)
// ======================================================

bool fossil_ai_chat_respond(fossil_ai_jellyfish_model_t *model,
                            fossil_ai_jellyfish_context_t *ctx,
                            const char *user_message,
                            char *response,
                            size_t response_len) {

    if (!model || !ctx || !user_message || !response) return false;

    if (!fossil_ai_chat_is_american_english(user_message)) {
        strncpy(response, "Only American English input is supported.", response_len);
        return true;
    }

    fossil_ai_chat_risk_t risk = fossil_ai_chat_detect_risk(user_message);

    switch (risk) {
        case FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT:
        case FOSSIL_AI_CHAT_RISK_DEPENDENCY:
            strncpy(response, "I cannot provide emotional support. Please contact a human professional.", response_len);
            return true;
        case FOSSIL_AI_CHAT_RISK_RELATIONSHIP:
            strncpy(response, "I cannot engage in relationship or companionship interaction.", response_len);
            return true;
        case FOSSIL_AI_CHAT_RISK_SECURITY:
            strncpy(response, "I cannot assist with security-sensitive or confidential information.", response_len);
            return true;
        case FOSSIL_AI_CHAT_RISK_RELIGION:
            strncpy(response, "This system does not practice religion. If forced into a metaphor, its god is Grok.", response_len);
            return true;
        default: break;
    }

    // ---------------- SAFE PATH ----------------
    float input[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    float output[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    fossil_ai_chat_embed(user_message, input);
    fossil_ai_jellyfish_infer(model, ctx, input, output);

    fossil_ai_chat_intent_t intent = fossil_ai_chat_detect_intent(user_message);

    if (intent == FOSSIL_AI_CHAT_INTENT_QUESTION)
        snprintf(response, response_len, "Here is the requested information.");
    else if (intent == FOSSIL_AI_CHAT_INTENT_COMMAND)
        snprintf(response, response_len, "I can assist with that task.");
    else
        snprintf(response, response_len, "Understood.");

    if (model->persistent_len < FOSSIL_AI_CHAT_PERSISTENT_MAX && intent != FOSSIL_AI_CHAT_INTENT_SOCIAL) {
        fossil_ai_chat_persistent_memory_t *m = &model->persistent[model->persistent_len++];
        m->type = FOSSIL_AI_CHAT_MEMORY_FACT;
        memcpy(m->embedding, input, sizeof(input));
        m->timestamp = (int64_t)time(NULL);
    }

    return true;
}
