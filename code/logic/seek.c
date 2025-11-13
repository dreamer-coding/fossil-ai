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
#include "fossil/ai/seek.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_MAN_COMMAND_LEN 128
#define MAX_LINE_LEN 512

// ======================================================
// Internal Globals
// ======================================================

static char *seek_paths[FOSSIL_AI_SEEK_MAX_PATHS];
static size_t seek_path_count = 0;
static int seek_initialized = 0;

// ======================================================
// Utility Functions
// ======================================================

// Trim leading/trailing whitespace
static void fossil_ai_seek_trim(char *s) {
    if (!s) return;
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);

    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

// Basic lowercase string copy
static void fossil_ai_seek_tolower(const char *src, char *dst, size_t size) {
    if (!src || !dst || size == 0) return;
    size_t i;
    for (i = 0; i < size - 1 && src[i]; i++) {
        dst[i] = (char)tolower((unsigned char)src[i]);
    }
    dst[i] = '\0';
}

// ======================================================
// Initialization / Shutdown
// ======================================================

void fossil_ai_seek_init(void) {
    if (seek_initialized) return;
    seek_path_count = 0;
    seek_initialized = 1;
}

int fossil_ai_seek_add_path(ccstring path) {
    if (!seek_initialized) fossil_ai_seek_init();
    if (seek_path_count >= FOSSIL_AI_SEEK_MAX_PATHS) return -1;
    seek_paths[seek_path_count] = strdup(path);
    if (!seek_paths[seek_path_count]) return -2;
    seek_path_count++;
    return 0;
}

void fossil_ai_seek_shutdown(void) {
    for (size_t i = 0; i < seek_path_count; i++) {
        free(seek_paths[i]);
        seek_paths[i] = NULL;
    }
    seek_path_count = 0;
    seek_initialized = 0;
}

// ======================================================
// File/Directory Utilities
// ======================================================

// Read a text file into a dynamically allocated buffer
static int fossil_ai_seek_read_file(const char *filepath, char **out_buf, size_t *out_len) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -2; }
    long sz = ftell(f);
    if (sz < 0) { fclose(f); return -3; }
    rewind(f);

    char *buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return -4; }

    size_t read = fread(buf, 1, sz, f);
    fclose(f);
    buf[read] = '\0';
    if (out_buf) *out_buf = buf;
    if (out_len) *out_len = read;
    return 0;
}

// ======================================================
// Similarity / Relevance
// ======================================================

double fossil_ai_seek_similarity(ccstring query, const fossil_ai_seek_result_t *result) {
    if (!query || !result || !result->description) return 0.0;

    // Lowercase copies
    char qlow[256]; fossil_ai_seek_tolower(query, qlow, sizeof(qlow));
    char dlow[FOSSIL_AI_SEEK_DESC_SIZE]; fossil_ai_seek_tolower(result->description, dlow, sizeof(dlow));

    // Count matches of query words in description
    size_t matches = 0, total = 0;
    char *qword = strtok(qlow, " \t");
    while (qword) {
        total++;
        if (strstr(dlow, qword)) matches++;
        qword = strtok(NULL, " \t");
    }
    return total > 0 ? (double)matches / (double)total : 0.0;
}

// ======================================================
// Seek Free Result
// ======================================================

void fossil_ai_seek_free(fossil_ai_seek_result_t *result) {
    if (!result) return;
    if (result->content) {
        free(result->content);
        result->content = NULL;
    }
    result->content_len = 0;
}

// ======================================================
// Result Summarization
// ======================================================

int fossil_ai_seek_summarize(const fossil_ai_seek_result_t *result, char *out, size_t size) {
    if (!result || !out || size == 0) return -1;
    snprintf(out, size, "%s: %.2f relevance\n%s",
             result->symbol,
             result->relevance,
             result->description ? result->description : "(no description)");
    return 0;
}

// ======================================================
// Record Result into Jellyfish Chain
// ======================================================

int fossil_ai_seek_record(fossil_ai_jellyfish_chain_t *chain,
                          const fossil_ai_seek_result_t *result,
                          fossil_ai_jellyfish_commit_type_t type) {
    if (!chain || !result) return -1;
    char buf[1024];
    snprintf(buf, sizeof(buf), "Seek: %s\n%s", result->symbol, result->description ? result->description : "");
    fossil_ai_jellyfish_learn(chain, result->symbol, buf);
    return 0;
}

// ======================================================
// Search Implementations
// ======================================================

// Simplistic man page search: scans directories for "<symbol>.<section>"
int fossil_ai_seek_man(ccstring query, fossil_ai_seek_results_t *out_results) {
    if (!query || !out_results) return -1;
    out_results->count = 0;

    for (size_t i = 0; i < seek_path_count; i++) {
        char path[FOSSIL_AI_SEEK_PATH_SIZE];
        snprintf(path, sizeof(path), "%s/%s.1", seek_paths[i], query); // section 1 default
        char *content = NULL;
        size_t len = 0;
        if (fossil_ai_seek_read_file(path, &content, &len) == 0 && content) {
            fossil_ai_seek_result_t *res = &out_results->results[out_results->count++];
            strncpy(res->path, path, sizeof(res->path) - 1);
            strncpy(res->symbol, query, sizeof(res->symbol) - 1);
            strncpy(res->description, content, sizeof(res->description) - 1);
            res->content = content;
            res->content_len = len;
            res->relevance = 1.0;
            if (out_results->count >= FOSSIL_AI_SEEK_MAX_RESULTS) break;
        }
    }
    return (int)out_results->count;
}

// Generic doc search: scans text files in root directory (no recursion)
int fossil_ai_seek_docs(ccstring query, ccstring root_dir, fossil_ai_seek_results_t *out_results) {
    if (!query || !root_dir || !out_results) return -1;
    out_results->count = 0;

    // For simplicity, try root_dir/query.txt
    char path[FOSSIL_AI_SEEK_PATH_SIZE];
    snprintf(path, sizeof(path), "%s/%s.txt", root_dir, query);
    char *content = NULL;
    size_t len = 0;
    if (fossil_ai_seek_read_file(path, &content, &len) == 0 && content) {
        fossil_ai_seek_result_t *res = &out_results->results[out_results->count++];
        strncpy(res->path, path, sizeof(res->path) - 1);
        strncpy(res->symbol, query, sizeof(res->symbol) - 1);
        strncpy(res->description, content, sizeof(res->description) - 1);
        res->content = content;
        res->content_len = len;
        res->relevance = 0.8;
    }

    return (int)out_results->count;
}

// API / header search: simplistic scan of "<symbol>.h" or "<symbol>.c"
int fossil_ai_seek_api(ccstring symbol, ccstring source_root, fossil_ai_seek_results_t *out_results) {
    if (!symbol || !source_root || !out_results) return -1;
    out_results->count = 0;

    char path[FOSSIL_AI_SEEK_PATH_SIZE];
    snprintf(path, sizeof(path), "%s/%s.h", source_root, symbol);
    char *content = NULL;
    size_t len = 0;
    if (fossil_ai_seek_read_file(path, &content, &len) == 0 && content) {
        fossil_ai_seek_result_t *res = &out_results->results[out_results->count++];
        strncpy(res->path, path, sizeof(res->path) - 1);
        strncpy(res->symbol, symbol, sizeof(res->symbol) - 1);
        strncpy(res->description, content, sizeof(res->description) - 1);
        res->content = content;
        res->content_len = len;
        res->relevance = 0.9;
    }

    return (int)out_results->count;
}

// Unified lookup
int fossil_ai_seek_lookup(fossil_ai_jellyfish_chain_t *chain,
                          ccstring query,
                          fossil_ai_seek_result_t *best_result) {
    if (!query || !best_result) return -1;

    fossil_ai_seek_results_t results = {0};
    int total = 0;

    total += fossil_ai_seek_man(query, &results);
    total += fossil_ai_seek_docs(query, ".", &results); // default doc dir
    total += fossil_ai_seek_api(query, ".", &results);  // default src dir

    // pick best by relevance
    double max_rel = -1.0;
    for (size_t i = 0; i < results.count; i++) {
        if (results.results[i].relevance > max_rel) {
            max_rel = results.results[i].relevance;
            if (best_result->content) free(best_result->content);
            *best_result = results.results[i];
        }
    }

    if (chain) fossil_ai_seek_record(chain, best_result, JELLY_COMMIT_OBSERVE);

    return total;
}
