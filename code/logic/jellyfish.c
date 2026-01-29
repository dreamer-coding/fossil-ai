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
#include "fossil/ai/jellyfish.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

/* ======================================================
 * Lookup Tables
 * ====================================================== */

static const uint32_t fnv_prime = 16777619U;
static const uint32_t fnv_offset_basis = 2166136261U;

/* ======================================================
 * Initialization / Cleanup
 * ====================================================== */

fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_create_model(const char* name) {
    if (!name) return NULL;

    fossil_ai_jellyfish_model_t* model = (fossil_ai_jellyfish_model_t*)calloc(1, sizeof(fossil_ai_jellyfish_model_t));
    if (!model) return NULL;

    strncpy(model->name, name, FOSSIL_AI_JELLYFISH_MODEL_NAME_LEN - 1);
    model->version = 1;
    model->memory_count = 0;
    model->trained = false;

    return model;
}

void fossil_ai_jellyfish_free_model(fossil_ai_jellyfish_model_t* model) {
    if (!model) return;
    free(model);
}

/* ======================================================
 * Model Persistence
 * ====================================================== */

bool fossil_ai_jellyfish_save_model(const fossil_ai_jellyfish_model_t* model, const char* path) {
    if (!model || !path) return false;
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    // Write model metadata
    fwrite(&model->version, sizeof(model->version), 1, f);
    fwrite(&model->memory_count, sizeof(model->memory_count), 1, f);
    fwrite(&model->trained, sizeof(model->trained), 1, f);
    fwrite(model->name, sizeof(model->name), 1, f);

    // Write memory
    fwrite(model->memory, sizeof(fossil_ai_jellyfish_memory_t), model->memory_count, f);

    fclose(f);
    return true;
}

fossil_ai_jellyfish_model_t* fossil_ai_jellyfish_load_model(const char* path) {
    if (!path) return NULL;
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;

    fossil_ai_jellyfish_model_t* model = (fossil_ai_jellyfish_model_t*)calloc(1, sizeof(fossil_ai_jellyfish_model_t));
    if (!model) { fclose(f); return NULL; }

    fread(&model->version, sizeof(model->version), 1, f);
    fread(&model->memory_count, sizeof(model->memory_count), 1, f);
    fread(&model->trained, sizeof(model->trained), 1, f);
    fread(model->name, sizeof(model->name), 1, f);

    fread(model->memory, sizeof(fossil_ai_jellyfish_memory_t), model->memory_count, f);
    fclose(f);
    return model;
}

/* ======================================================
 * Memory Management
 * ====================================================== */

bool fossil_ai_jellyfish_add_memory(fossil_ai_jellyfish_model_t* model, const float* embedding, const float* output, const char* id, int64_t timestamp) {
    if (!model || !embedding || !output || !id) return false;
    if (model->memory_count >= FOSSIL_AI_JELLYFISH_MAX_MEMORY) return false;

    fossil_ai_jellyfish_memory_t* mem = &model->memory[model->memory_count++];
    memcpy(mem->embedding, embedding, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    memcpy(mem->output, output, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    mem->timestamp = timestamp;
    strncpy(mem->id, id, sizeof(mem->id) - 1);
    return true;
}

fossil_ai_jellyfish_memory_t* fossil_ai_jellyfish_get_memory(const fossil_ai_jellyfish_model_t* model, const char* id) {
    if (!model || !id) return NULL;

    for (size_t i = 0; i < model->memory_count; ++i) {
        if (strncmp(model->memory[i].id, id, sizeof(model->memory[i].id)) == 0)
            return (fossil_ai_jellyfish_memory_t*)&model->memory[i];
    }
    return NULL;
}

/* ======================================================
 * Memory / Embedding Utilities
 * ====================================================== */

/* Normalize embedding in-place and return magnitude */
static float fossil_ai_jellyfish_normalize_embedding(float* vec) {
    float mag = 0.0f;
    for (size_t i = 0; i < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++i) {
        mag += vec[i] * vec[i];
    }
    if (mag > 0.0f) {
        mag = sqrtf(mag);
        for (size_t i = 0; i < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++i) {
            vec[i] /= mag;
        }
    }
    return mag;
}

/* ======================================================
 * Training / Inference (Weighted k-NN)
 * ====================================================== */

/* Precompute normalized embeddings for all memory vectors */
static void fossil_ai_jellyfish_precompute_norms(fossil_ai_jellyfish_model_t* model) {
    for (size_t i = 0; i < model->memory_count; ++i) {
        float mag = 0.0f;
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
            mag += model->memory[i].embedding[j] * model->memory[i].embedding[j];
        mag = sqrtf(mag);
        if (mag > 0.0f) {
            for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
                model->memory[i].embedding[j] /= mag;
        }
    }
}

/* Train the model: normalize embeddings */
bool fossil_ai_jellyfish_train_model(fossil_ai_jellyfish_model_t* model) {
    if (!model) return false;
    if (model->memory_count == 0) {
        model->trained = false;
        return false;
    }

    fossil_ai_jellyfish_precompute_norms(model);
    model->trained = true;
    return true;
}

/* Predict output embedding using weighted k-nearest neighbors */
bool fossil_ai_jellyfish_predict(const fossil_ai_jellyfish_model_t* model, const float* input_embedding, float* output_embedding) {
    if (!model || !input_embedding || !output_embedding) return false;
    if (!model->trained || model->memory_count == 0) return false;

    // Normalize input embedding
    float norm_input[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    memcpy(norm_input, input_embedding, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    fossil_ai_jellyfish_normalize_embedding(norm_input);

    // Default k = min(3, memory_count)
    size_t k = model->memory_count < 3 ? model->memory_count : 3;

    // Arrays to track top k similarities
    size_t best_indices[3] = {0};
    float best_scores[3] = {-FLT_MAX, -FLT_MAX, -FLT_MAX};

    // Compute cosine similarity
    for (size_t i = 0; i < model->memory_count; ++i) {
        float dot = 0.0f;
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
            dot += norm_input[j] * model->memory[i].embedding[j];

        // Check if this is in top k
        for (size_t t = 0; t < k; ++t) {
            if (dot > best_scores[t]) {
                // Shift lower scores down
                for (size_t s = k - 1; s > t; --s) {
                    best_scores[s] = best_scores[s - 1];
                    best_indices[s] = best_indices[s - 1];
                }
                best_scores[t] = dot;
                best_indices[t] = i;
                break;
            }
        }
    }

    // Weighted sum of top k outputs
    memset(output_embedding, 0, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    float total_weight = 0.0f;
    for (size_t t = 0; t < k; ++t) {
        float weight = best_scores[t] > 0.0f ? best_scores[t] : 0.0f;
        total_weight += weight;
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
            output_embedding[j] += model->memory[best_indices[t]].output[j] * weight;
    }

    // Normalize output by total weight
    if (total_weight > 0.0f) {
        for (size_t j = 0; j < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++j)
            output_embedding[j] /= total_weight;
    }

    return true;
}

/* ======================================================
 * Endianness
 * ====================================================== */

bool fossil_ai_jellyfish_is_little_endian(void) {
    uint16_t x = 1;
    return *((uint8_t*)&x) == 1;
}

/* ======================================================
 * System / Hardware Awareness
 * ====================================================== */

fossil_ai_jellyfish_system_info_t fossil_ai_jellyfish_get_system_info(void) {
    fossil_ai_jellyfish_system_info_t info = {0};

    /* CPU cores */
#if defined(_WIN32)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    info.cpu_cores = (size_t)sysinfo.dwNumberOfProcessors;
#else
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    info.cpu_cores = (nprocs > 0) ? (size_t)nprocs : 1;
#endif

    /* RAM size */
#if defined(_WIN32)
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        info.ram_bytes = (size_t)memStatus.ullTotalPhys;
    } else {
        info.ram_bytes = 0;
    }
#else
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0) {
        info.ram_bytes = (size_t)pages * (size_t)page_size;
    } else {
        info.ram_bytes = 0;
    }
#endif

    /* Endianness */
    info.is_little_endian = fossil_ai_jellyfish_is_little_endian();

    return info;
}

/* ======================================================
 * Utility / Hashing
 * ====================================================== */

uint32_t fossil_ai_jellyfish_hash_string(const char* s) {
    uint32_t h = fnv_offset_basis;
    for (; *s; ++s) {
        h ^= (uint8_t)(*s);
        h *= fnv_prime;
    }
    return h;
}

/* ======================================================
 * Auto-Detection
 * ====================================================== */

void fossil_ai_jellyfish_detect_capabilities(fossil_ai_jellyfish_model_t* model) {
    if (!model) return;

    printf("=== Jellyfish AI Model Capabilities ===\n");

    // Model metadata
    printf("Model Name       : %s\n", model->name);
    printf("Model Version    : %llu\n", (unsigned long long)model->version);
    printf("Trained          : %s\n", model->trained ? "yes" : "no");

    // Memory statistics
    printf("Memory Count     : %zu / %d\n", model->memory_count, FOSSIL_AI_JELLYFISH_MAX_MEMORY);
    if (model->memory_count > 0) {
        size_t total_embeddings = model->memory_count * FOSSIL_AI_JELLYFISH_EMBED_SIZE;
        printf("Total Embeddings : %zu floats\n", total_embeddings);
    }

    // System / hardware info
    fossil_ai_jellyfish_system_info_t sysinfo = fossil_ai_jellyfish_get_system_info();
    printf("CPU Cores        : %zu\n", sysinfo.cpu_cores);
    printf("RAM              : %zu bytes\n", sysinfo.ram_bytes);
    printf("Endianness       : %s\n", sysinfo.is_little_endian ? "Little" : "Big");

    // Capabilities
    printf("Supports k-NN Prediction   : yes\n");
    printf("Supports Weighted Outputs  : yes\n");
    printf("Persistent Storage Ready   : yes\n");

    printf("=== End of Capabilities ===\n");
}

/* ======================================================
 * Utility: compute embedding magnitude
 * ====================================================== */
static float fossil_ai_jellyfish_embedding_magnitude(const float* embedding) {
    float mag = 0.0f;
    for (size_t i = 0; i < FOSSIL_AI_JELLYFISH_EMBED_SIZE; ++i)
        mag += embedding[i] * embedding[i];
    return sqrtf(mag);
}

/* ======================================================
 * Audit function
 * ====================================================== */
void fossil_ai_jellyfish_audit(const fossil_ai_jellyfish_model_t* model) {
    if (!model) return;

    printf("=== Jellyfish AI Model Audit ===\n");

    // Model metadata
    printf("Model Name       : %s\n", model->name);
    printf("Model Version    : %llu\n", (unsigned long long)model->version);
    printf("Trained          : %s\n", model->trained ? "yes" : "no");
    printf("Memory Count     : %zu / %d\n", model->memory_count, FOSSIL_AI_JELLYFISH_MAX_MEMORY);

    // Memory overview
    printf("---- Memory Vectors ----\n");
    for (size_t i = 0; i < model->memory_count; ++i) {
        const fossil_ai_jellyfish_memory_t* mem = &model->memory[i];
        float embed_mag = fossil_ai_jellyfish_embedding_magnitude(mem->embedding);
        printf("[%zu] ID: %s | Timestamp: %lld | Embedding Norm: %.4f\n",
               i, mem->id, (long long)mem->timestamp, embed_mag);
    }

    // Training audit
    printf("---- Training Audit ----\n");
    if (model->trained) {
        printf("Model is trained.\n");
        printf("Memory vectors ready for k-NN prediction: %zu\n", model->memory_count);
    } else {
        printf("Model not trained yet.\n");
    }

    // System / hardware info
    printf("---- Hardware Info ----\n");
    fossil_ai_jellyfish_system_info_t sysinfo = fossil_ai_jellyfish_get_system_info();
    printf("CPU Cores        : %zu\n", sysinfo.cpu_cores);
    printf("RAM              : %zu bytes\n", sysinfo.ram_bytes);
    printf("Endianness       : %s\n", sysinfo.is_little_endian ? "Little" : "Big");

    printf("=== End of Audit ===\n");
}
