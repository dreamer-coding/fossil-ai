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
#include "fossil/ai/module.h"
#ifndef _WIN32
#include <dirent.h>
#endif

/* ---------------------------------------------------------------------------
 * Jellyfish Chain API Usage Examples
 * --------------------------------------------------------------------------- */

/* Example: Learn new input-output pair in module memory */
void fossil_ai_module_learn(fossil_ai_module_t *module, const char *input, const char *output) {
    if (!module || !input || !output) return;
    fossil_ai_jellyfish_learn(&module->persistent_mem, input, output);
}

/* Example: Reason over module memory */
const char *fossil_ai_module_reason(fossil_ai_module_t *module, const char *input) {
    if (!module || !input) return "Unknown";
    return fossil_ai_jellyfish_reason(&module->persistent_mem, input);
}

/* Example: Prune module memory by confidence */
int fossil_ai_module_prune(fossil_ai_module_t *module, float min_confidence) {
    if (!module) return -1;
    return fossil_ai_jellyfish_prune(&module->persistent_mem, min_confidence);
}

/* Example: Audit module memory */
int fossil_ai_module_audit(const fossil_ai_module_t *module) {
    if (!module) return -1;
    return fossil_ai_jellyfish_audit(&module->persistent_mem);
}

/* Example: Dump module memory for diagnostics */
void fossil_ai_module_dump(const fossil_ai_module_t *module) {
    if (!module) return;
    fossil_ai_jellyfish_dump(&module->persistent_mem);
}

/* Example: Get best memory block for module */
const fossil_ai_jellyfish_block_t *fossil_ai_module_best_memory(const fossil_ai_module_t *module) {
    if (!module) return NULL;
    return fossil_ai_jellyfish_best_memory(&module->persistent_mem);
}

/* Example: Chain trust score */
float fossil_ai_module_trust_score(const fossil_ai_module_t *module) {
    if (!module) return 0.0f;
    return fossil_ai_jellyfish_chain_trust_score(&module->persistent_mem);
}

/* ---------------------------------------------------------------------------
 * Existing Module Registry and Utility Functions
 * --------------------------------------------------------------------------- */
static inline void fossil_ai_module_cap_add(fossil_ai_module_t *m, uint64_t cap) {
    m->capabilities |= cap;
}

static inline void fossil_ai_module_cap_remove(fossil_ai_module_t *m, uint64_t cap) {
    m->capabilities &= ~cap;
}

static inline int fossil_ai_module_cap_check(const fossil_ai_module_t *m, uint64_t cap) {
    return (m->capabilities & cap) != 0;
}

/* ---------------------------------------------------------------------------
 * Utility: safe string copy
 * --------------------------------------------------------------------------- */
static void fossil_ai_safe_strcpy(char *dst, const char *src, size_t max) {
    if (!dst || !src || max == 0) return;
    strncpy(dst, src, max - 1);
    dst[max - 1] = '\0';
}

/* ---------------------------------------------------------------------------
 * Registry Initialization
 * --------------------------------------------------------------------------- */
void fossil_ai_module_registry_init(fossil_ai_module_registry_t *registry) {
    if (!registry) return;

    registry->count = 0;
    registry->tick_count = 0;
    registry->global_log_fn = NULL;
    for (size_t i = 0; i < FOSSIL_AI_MODULE_MAX_MODULES; i++) {
        fossil_ai_module_t *mod = &registry->modules[i];
        mod->name[0] = '\0';
        mod->version[0] = '\0';
        mod->active = 0;
        mod->execute = NULL;
        mod->on_load = NULL;
        mod->on_unload = NULL;
        mod->on_tick = NULL;
        mod->ask_fn = NULL;
        mod->chat_fn = NULL;
        mod->summary_fn = NULL;
        mod->persistent_mem = (fossil_ai_jellyfish_chain_t){0};
        mod->config_count = 0;
        mod->capabilities = 0;
        mod->scratch[0] = '\0';
        mod->log_fn = NULL;
        mod->sandbox = NULL;
        mod->sandbox_size = 0;
        for (size_t k = 0; k < 32; k++) {
            mod->config_keys[k][0] = '\0';
            mod->config_values[k][0] = '\0';
        }
        fossil_ai_jellyfish_init(&mod->persistent_mem);
    }
}

/* ---------------------------------------------------------------------------
 * Register Module
 * --------------------------------------------------------------------------- */
int fossil_ai_module_register(
    fossil_ai_module_registry_t *registry,
    const char *name,
    fossil_ai_module_fn execute
) {
    if (!registry || !name || !execute) return -1;

    if (registry->count >= FOSSIL_AI_MODULE_MAX_MODULES)
        return -1;

    fossil_ai_module_t *mod = &registry->modules[registry->count];

    fossil_ai_safe_strcpy(mod->name, name, FOSSIL_AI_MODULE_MAX_NAME);
    mod->version[0] = '\0';
    mod->execute = execute;
    mod->active = 1;
    mod->on_load = NULL;
    mod->on_unload = NULL;
    mod->on_tick = NULL;
    mod->ask_fn = NULL;
    mod->chat_fn = NULL;
    mod->summary_fn = NULL;
    mod->log_fn = NULL;
    mod->capabilities = 0;
    mod->config_count = 0;
    mod->scratch[0] = '\0';
    mod->sandbox = NULL;
    mod->sandbox_size = 0;
    for (size_t k = 0; k < 32; k++) {
        mod->config_keys[k][0] = '\0';
        mod->config_values[k][0] = '\0';
    }
    fossil_ai_jellyfish_init(&mod->persistent_mem);

    if (mod->on_load)
        mod->on_load(mod);

    return (int)(registry->count++);
}

/* ---------------------------------------------------------------------------
 * Unregister Module
 * --------------------------------------------------------------------------- */
int fossil_ai_module_unregister(fossil_ai_module_registry_t *registry, const char *name) {
    if (!registry || !name) return -1;

    for (size_t i = 0; i < registry->count; i++) {
        fossil_ai_module_t *mod = &registry->modules[i];
        if (strcmp(mod->name, name) == 0) {
            if (mod->on_unload)
                mod->on_unload(mod);
            // Shift remainder left
            for (size_t j = i; j < registry->count - 1; j++) {
                registry->modules[j] = registry->modules[j + 1];
            }
            registry->count--;
            return 0;
        }
    }
    return -1;  // Not found
}

/* ---------------------------------------------------------------------------
 * Get Module By Name
 * --------------------------------------------------------------------------- */
fossil_ai_module_t *fossil_ai_module_get(
    fossil_ai_module_registry_t *registry,
    const char *name
) {
    if (!registry || !name) return NULL;

    for (size_t i = 0; i < registry->count; i++) {
        if (strcmp(registry->modules[i].name, name) == 0)
            return &registry->modules[i];
    }
    return NULL;
}

/* ---------------------------------------------------------------------------
 * Registry Tick
 * --------------------------------------------------------------------------- */
void fossil_ai_module_registry_tick(
    fossil_ai_module_registry_t *registry,
    double dt
) {
    if (!registry) return;

    registry->tick_count++;

    for (size_t i = 0; i < registry->count; i++) {
        fossil_ai_module_t *mod = &registry->modules[i];
        if (mod->active && mod->on_tick)
            mod->on_tick(mod, registry->tick_count, dt);
    }
}

/* ---------------------------------------------------------------------------
 * Execute Module
 * --------------------------------------------------------------------------- */
int fossil_ai_module_execute(
    fossil_ai_module_registry_t *registry,
    const char *name,
    const char *input,
    char *output,
    size_t output_size
) {
    if (!registry || !name || !input || !output) return -1;

    fossil_ai_module_t *mod = fossil_ai_module_get(registry, name);
    if (!mod || !mod->active || !mod->execute)
        return -1;

    return mod->execute(input, output, output_size, &mod->persistent_mem);
}

/* ---------------------------------------------------------------------------
 * Save Module Memory
 * --------------------------------------------------------------------------- */
int fossil_ai_module_save(const fossil_ai_module_t *module, const char *filepath) {
    if (!module || !filepath) return -1;

    return fossil_ai_jellyfish_save(&module->persistent_mem, filepath);
}

/* ---------------------------------------------------------------------------
 * Load Module Memory
 * --------------------------------------------------------------------------- */
int fossil_ai_module_load(fossil_ai_module_t *module, const char *filepath) {
    if (!module || !filepath) return -1;

    return fossil_ai_jellyfish_load(&module->persistent_mem, filepath);
}

/* ---------------------------------------------------------------------------
 * List Modules
 * --------------------------------------------------------------------------- */
void fossil_ai_module_list(const fossil_ai_module_registry_t *registry) {
    if (!registry) return;

    printf("Registered AI Modules (%zu):\n", registry->count);
    for (size_t i = 0; i < registry->count; i++) {
        const fossil_ai_module_t *m = &registry->modules[i];
        printf(" - [%s] v%s (active=%d, capabilities=0x%llx)\n",
            m->name, m->version, m->active, (unsigned long long)m->capabilities);
    }
}

/* ---------------------------------------------------------------------------
 * Reflect Module
 * --------------------------------------------------------------------------- */
void fossil_ai_module_reflect(const fossil_ai_module_t *module) {
    if (!module) return;

    printf("Module Reflection: %s\n", module->name);
    printf("  Version: %s\n", module->version);
    printf("  Active: %d\n", module->active);
    printf("  Capabilities: 0x%llx\n", (unsigned long long)module->capabilities);
    printf("  Config count: %zu\n", module->config_count);
    fossil_ai_jellyfish_reflect(&module->persistent_mem);
}

/* ---------------------------------------------------------------------------
 * Model Interface Wrappers
 * --------------------------------------------------------------------------- */
int fossil_ai_model_ask(
    fossil_ai_module_t *module,
    const char *prompt,
    char *output,
    size_t output_size)
{
    if (module && module->ask_fn)
        return module->ask_fn(module, prompt, output, output_size);

    if (module && module->execute)
        return module->execute(prompt, output, output_size, &module->persistent_mem);

    return -1;
}

/* ---------------------------------------------------------------------------
 * Model Chat Interface
 * --------------------------------------------------------------------------- */
int fossil_ai_model_chat(
    fossil_ai_module_t *module,
    const char *input,
    char *output,
    size_t output_size,
    fossil_ai_jellyfish_chain_t *ctx)
{
    if (module && module->chat_fn)
        return module->chat_fn(module, input, output, output_size, ctx);

    if (!module || !module->execute || !ctx)
        return -1;

    /* default chat = vanilla prompt + append to context */
    fossil_ai_jellyfish_learn(ctx, "user", input);

    int rc = module->execute(input, output, output_size, ctx);
    fossil_ai_jellyfish_learn(ctx, "assistant", output);

    return rc;
}

/* ---------------------------------------------------------------------------
 * Model Summary Interface
 * --------------------------------------------------------------------------- */
int fossil_ai_model_summary(
    fossil_ai_module_t *module,
    fossil_ai_jellyfish_chain_t *chain,
    char *output,
    size_t output_size,
    int depth,
    int timestamps)
{
    if (module && module->summary_fn)
        return module->summary_fn(module, chain, output, output_size, depth, timestamps);

    if (!chain || !output)
        return -1;

    return fossil_ai_jellyfish_summarize(chain, output, output_size, depth);
}
