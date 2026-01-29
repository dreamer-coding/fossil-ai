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
#include <stdint.h>
#include <stdbool.h>

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
 * Training / Inference
 * ====================================================== */

bool fossil_ai_jellyfish_train_model(fossil_ai_jellyfish_model_t* model) {
    if (!model) return false;
    // Simple stub: mark trained if it has memory
    if (model->memory_count > 0) model->trained = true;
    return model->trained;
}

bool fossil_ai_jellyfish_predict(const fossil_ai_jellyfish_model_t* model, const float* input_embedding, float* output_embedding) {
    if (!model || !input_embedding || !output_embedding) return false;
    if (!model->trained || model->memory_count == 0) return false;

    // Stub: copy the first memory output as prediction
    memcpy(output_embedding, model->memory[0].output, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    return true;
}

/* ======================================================
 * System / Hardware Awareness
 * ====================================================== */

bool fossil_ai_jellyfish_is_little_endian(void) {
    uint16_t x = 1;
    return *((uint8_t*)&x) == 1;
}

fossil_ai_jellyfish_system_info_t fossil_ai_jellyfish_get_system_info(void) {
    fossil_ai_jellyfish_system_info_t info = {0};
#if defined(_WIN32)
    info.cpu_cores = (size_t)__builtin_popcount(~0); // placeholder
    info.ram_bytes = 0; // placeholder, could use GlobalMemoryStatusEx
#else
    info.cpu_cores = (size_t)sysconf(_SC_NPROCESSORS_ONLN);
    info.ram_bytes = (size_t)sysconf(_SC_PHYS_PAGES) * (size_t)sysconf(_SC_PAGESIZE);
#endif
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
    // Stub: print capabilities
    printf("Model: %s\n", model->name);
    printf("Memory count: %zu\n", model->memory_count);
    printf("Trained: %s\n", model->trained ? "yes" : "no");
    printf("Version: %llu\n", (unsigned long long)model->version);
}
