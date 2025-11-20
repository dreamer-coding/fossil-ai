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
#ifndef FOSSIL_JELLYFISH_MODULE_H
#define FOSSIL_JELLYFISH_MODULE_H

#include "jellyfish.h"

#define FOSSIL_AI_MODULE_MAX_NAME 64
#define FOSSIL_AI_MODULE_MAX_MODULES 64

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct fossil_ai_module fossil_ai_module_t;

/**
 * Primary function pointer for module execution.
 * Modules can perform AI reasoning using `persistent_mem`.
 *
 * Returns 0 on success, non-zero on error.
 */
typedef int (*fossil_ai_module_fn)(
    const char *input,                       // Input string for reasoning
    char *output,                            // Output buffer for result
    size_t output_size,                      // Size of output buffer
    fossil_ai_jellyfish_chain_t *persistent_mem // Persistent memory chain for module state
);

/**
 * Optional: called when a module is first loaded or registered.
 * Used for initialization tasks.
 */
typedef int (*fossil_ai_module_on_load_fn)(fossil_ai_module_t *module);

/**
 * Optional: called before a module is deleted or replaced.
 * Used for cleanup tasks.
 */
typedef int (*fossil_ai_module_on_unload_fn)(fossil_ai_module_t *module);

/**
 * Model interface: one-shot prompt/response.
 * Used for direct question-answering.
 */
typedef int (*fossil_ai_model_ask_fn)(
    fossil_ai_module_t *module,              // Module instance
    const char *prompt,                      // Prompt string
    char *output,                            // Output buffer
    size_t output_size                       // Output buffer size
);

/**
 * Model interface: stateful conversation.
 * Used for multi-turn chat scenarios.
 */
typedef int (*fossil_ai_model_chat_fn)(
    fossil_ai_module_t *module,              // Module instance
    const char *input,                       // Input string
    char *output,                            // Output buffer
    size_t output_size,                      // Output buffer size
    fossil_ai_jellyfish_chain_t *conversation_chain // Conversation state chain
);

/**
 * Model interface: summarize or compress a dataset.
 * Used for generating summaries or extracting key info.
 */
typedef int (*fossil_ai_model_summary_fn)(
    fossil_ai_module_t *module,              // Module instance
    fossil_ai_jellyfish_chain_t *target_chain, // Target data chain
    char *output,                            // Output buffer
    size_t output_size,                      // Output buffer size
    int depth,                               // Summary depth/level
    int include_timestamps                   // Whether to include timestamps
);

/**
 * Optional: periodic tick/update event (AI agents, monitors, daemons).
 * Called by a scheduler or event loop.
 */
typedef int (*fossil_ai_module_tick_fn)(
    fossil_ai_module_t *module,              // Module instance
    uint64_t tick_count,                     // Global tick counter
    double dt_seconds                        // Delta time in seconds
);

/**
 * Optional logging/debug callback.
 * Used for custom logging or debugging output.
 */
typedef void (*fossil_ai_module_log_fn)(
    fossil_ai_module_t *module,              // Module instance
    const char *message                      // Log message
);

/**
 * Extended AI Module Metadata & State
 * Represents a single AI module with execution logic, lifecycle hooks,
 * persistent memory, configuration, and optional model interfaces.
 */
struct fossil_ai_module {
    char name[FOSSIL_AI_MODULE_MAX_NAME];    // Module name (unique identifier)
    char version[16];                        // Module version string
    int active;                              // Active/running flag

    fossil_ai_module_fn execute;             // Main execution function

    fossil_ai_module_on_load_fn   on_load;   // Called when module is loaded
    fossil_ai_module_on_unload_fn on_unload; // Called before module is unloaded
    fossil_ai_module_tick_fn      on_tick;   // Periodic tick/update callback

    fossil_ai_jellyfish_chain_t persistent_mem; // Persistent memory chain

    /* Model interface functions */
    fossil_ai_model_ask_fn     ask_fn;     // One-shot prompt/response
    fossil_ai_model_chat_fn    chat_fn;    // Stateful conversation
    fossil_ai_model_summary_fn summary_fn; // Summaries/dataset compression

    /* Configuration key-value pairs */
    char config_keys[32][64];               // Config keys
    char config_values[32][256];            // Config values
    size_t config_count;                    // Number of config entries

    uint64_t capabilities;                  // Capability flags (bitmask)

    char scratch[1024];                     // Scratch buffer for temporary data

    fossil_ai_module_log_fn log_fn;         // Optional logging callback
    void *sandbox;                          // Optional sandbox memory/region
    size_t sandbox_size;                    // Size of sandbox region
};

/**
 * Module Registry
 * Holds all registered AI modules and manages global lifecycle/events.
 */
typedef struct {
    fossil_ai_module_t modules[FOSSIL_AI_MODULE_MAX_MODULES]; /**< Array of registered modules */
    size_t count;                                             /**< Number of modules currently registered */

    uint64_t tick_count;                                      /**< Registry-level tick counter for scheduling/events */

    fossil_ai_module_log_fn global_log_fn;                    /**< Optional global logging callback for all modules */
} fossil_ai_module_registry_t;

/* ------------------------ Registry Operations ----------------------------- */

/**
 * @brief Initialize the module registry.
 * Clears all registered modules and resets counters.
 *
 * @param registry Pointer to module registry.
 */
void fossil_ai_module_registry_init(fossil_ai_module_registry_t *registry);

/**
 * @brief Register a new module.
 * Adds a module with the given name and execution function.
 *
 * @param registry Pointer to module registry.
 * @param name Unique module name.
 * @param execute Function pointer for module execution.
 * @return Index of registered module on success, -1 if registry is full.
 */
int fossil_ai_module_register(
    fossil_ai_module_registry_t *registry,
    const char *name,
    fossil_ai_module_fn execute
);

/**
 * @brief Unregister a module by name.
 * Removes the module from the registry.
 *
 * @param registry Pointer to module registry.
 * @param name Module name to unregister.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_module_unregister(fossil_ai_module_registry_t *registry, const char *name);

/**
 * @brief Retrieve a module by name.
 *
 * @param registry Pointer to module registry.
 * @param name Module name to look up.
 * @return Pointer to module if found, NULL otherwise.
 */
fossil_ai_module_t *fossil_ai_module_get(fossil_ai_module_registry_t *registry, const char *name);

/* ------------------------ Module Execution ------------------------------- */

/**
 * @brief Execute a registered module by name.
 * Calls the module's execute function.
 *
 * @param registry Pointer to module registry.
 * @param name Module name.
 * @param input Input string for execution.
 * @param output Output buffer for result.
 * @param output_size Size of output buffer.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_module_execute(
    fossil_ai_module_registry_t *registry,
    const char *name,
    const char *input,
    char *output,
    size_t output_size
);

/* ------------------------ Persistence ----------------------------------- */

/**
 * @brief Save module persistent memory to file.
 *
 * @param module Pointer to module.
 * @param filepath Path to file for saving.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_module_save(const fossil_ai_module_t *module, const char *filepath);

/**
 * @brief Load module persistent memory from file.
 *
 * @param module Pointer to module.
 * @param filepath Path to file for loading.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_module_load(fossil_ai_module_t *module, const char *filepath);

/* ------------------------ Introspection --------------------------------- */

/**
 * @brief List all active modules in the registry.
 *
 * @param registry Pointer to module registry.
 */
void fossil_ai_module_list(const fossil_ai_module_registry_t *registry);

/**
 * @brief Reflect on a module.
 * Summarizes persistent memory statistics and metadata.
 *
 * @param module Pointer to module.
 */
void fossil_ai_module_reflect(const fossil_ai_module_t *module);

/**
 * @brief One-shot prompt/response interface for AI models.
 * Calls the module's ask_fn if available.
 *
 * @param module Pointer to AI module.
 * @param prompt Input prompt string.
 * @param output Output buffer for response.
 * @param output_size Size of output buffer.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_model_ask(
    fossil_ai_module_t *module,
    const char *prompt,
    char *output,
    size_t output_size);

/**
 * @brief Stateful conversation interface for AI models.
 * Calls the module's chat_fn if available.
 *
 * @param module Pointer to AI module.
 * @param input Input string for conversation.
 * @param output Output buffer for response.
 * @param output_size Size of output buffer.
 * @param ctx Pointer to conversation state chain.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_model_chat(
    fossil_ai_module_t *module,
    const char *input,
    char *output,
    size_t output_size,
    fossil_ai_jellyfish_chain_t *ctx);

/**
 * @brief Summarize or compress a dataset using the module's summary_fn.
 *
 * @param module Pointer to AI module.
 * @param chain Target data chain to summarize.
 * @param output Output buffer for summary.
 * @param output_size Size of output buffer.
 * @param depth Summary depth/level.
 * @param timestamps Whether to include timestamps in summary.
 * @return 0 on success, non-zero on error.
 */
int fossil_ai_model_summary(
    fossil_ai_module_t *module,
    fossil_ai_jellyfish_chain_t *chain,
    char *output,
    size_t output_size,
    int depth,
    int timestamps);

#ifdef __cplusplus
}
#include <stdexcept>
#include <vector>
#include <string>

namespace fossil {

    namespace ai {

        /**
         * @brief C++ wrapper for fossil_ai_module_t structure.
         * Provides convenient access to module properties and lifecycle/event hooks.
         */
        class Module {
        public:
            /**
             * @brief Construct a Module wrapper for a given fossil_ai_module_t pointer.
             * @param mod Pointer to fossil_ai_module_t.
             */
            Module(fossil_ai_module_t *mod) : module_(mod) {}

            /**
             * @brief Get the module's name.
             * @return Module name string.
             */
            const char* name() const { return module_->name; }

            /**
             * @brief Check if the module is active.
             * @return True if active, false otherwise.
             */
            bool is_active() const { return module_->active != 0; }

            /**
             * @brief Execute the module's main function.
             * @param input Input string.
             * @param output Output buffer.
             * @param output_size Size of output buffer.
             * @param chain Persistent memory chain.
             * @return 0 on success, non-zero on error.
             */
            int execute(const char *input, char *output, size_t output_size, fossil_ai_jellyfish_chain_t *chain) {
            if (module_->execute)
                return module_->execute(input, output, output_size, chain);
            return -1;
            }

            /**
             * @brief Access the module's persistent memory chain.
             * @return Reference to persistent memory chain.
             */
            fossil_ai_jellyfish_chain_t& persistent_mem() { return module_->persistent_mem; }
            const fossil_ai_jellyfish_chain_t& persistent_mem() const { return module_->persistent_mem; }

            /**
             * @brief Save the module's persistent memory to a file.
             * @param filepath Path to save file.
             * @return 0 on success, non-zero on error.
             */
            int save(const std::string &filepath) const {
            return fossil_ai_module_save(module_, filepath.c_str());
            }

            /**
             * @brief Load the module's persistent memory from a file.
             * @param filepath Path to load file.
             * @return 0 on success, non-zero on error.
             */
            int load(const std::string &filepath) {
            return fossil_ai_module_load(module_, filepath.c_str());
            }

            /**
             * @brief Reflect on the module's metadata and persistent memory.
             */
            void reflect() const {
            fossil_ai_module_reflect(module_);
            }

            /**
             * @brief One-shot prompt/response interface for AI models.
             * @param prompt Input prompt string.
             * @param output Output buffer.
             * @param output_size Size of output buffer.
             * @return 0 on success, non-zero on error.
             */
            int ask(const char *prompt, char *output, size_t output_size) {
            return fossil_ai_model_ask(module_, prompt, output, output_size);
            }

            /**
             * @brief Stateful conversation interface for AI models.
             * @param input Input string.
             * @param output Output buffer.
             * @param output_size Size of output buffer.
             * @param ctx Conversation state chain.
             * @return 0 on success, non-zero on error.
             */
            int chat(const char *input, char *output, size_t output_size, fossil_ai_jellyfish_chain_t *ctx) {
            return fossil_ai_model_chat(module_, input, output, output_size, ctx);
            }

            /**
             * @brief Summarize or compress a dataset using the module's summary_fn.
             * @param chain Target data chain.
             * @param output Output buffer.
             * @param output_size Size of output buffer.
             * @param depth Summary depth/level.
             * @param timestamps Whether to include timestamps.
             * @return 0 on success, non-zero on error.
             */
            int summary(fossil_ai_jellyfish_chain_t *chain, char *output, size_t output_size, int depth, int timestamps) {
            return fossil_ai_model_summary(module_, chain, output, output_size, depth, timestamps);
            }

        public:
            fossil_ai_module_t *module_; /**< Underlying C module pointer */
        };

        /**
         * @brief C++ wrapper for fossil_ai_module_registry_t.
         * Manages a registry of modules and provides lifecycle/event operations.
         */
        class ModuleRegistry {
        public:
            /**
             * @brief Construct a ModuleRegistry wrapper for a given fossil_ai_module_registry_t pointer.
             * @param reg Pointer to fossil_ai_module_registry_t.
             */
            ModuleRegistry(fossil_ai_module_registry_t *reg) : registry_(reg) {}

            /**
             * @brief Initialize the module registry.
             */
            void init() { fossil_ai_module_registry_init(registry_); }

            /**
             * @brief Register a new module.
             * @param name Unique module name.
             * @param execute Function pointer for module execution.
             * @return Index of registered module on success, -1 if registry is full.
             */
            int register_module(const std::string &name, fossil_ai_module_fn execute) {
            return fossil_ai_module_register(registry_, name.c_str(), execute);
            }

            /**
             * @brief Unregister a module by name.
             * @param name Module name to unregister.
             * @return 0 on success, non-zero on error.
             */
            int unregister_module(const std::string &name) {
            return fossil_ai_module_unregister(registry_, name.c_str());
            }

            /**
             * @brief Retrieve a module by name.
             * @param name Module name.
             * @return Module wrapper object.
             * @throws std::runtime_error if module not found.
             */
            Module get(const std::string &name) {
            fossil_ai_module_t *mod = fossil_ai_module_get(registry_, name.c_str());
            if (!mod) throw std::runtime_error("Module not found");
            return Module(mod);
            }

            /**
             * @brief Execute a registered module by name.
             * @param name Module name.
             * @param input Input string.
             * @param output Output buffer.
             * @param output_size Size of output buffer.
             * @return 0 on success, non-zero on error.
             */
            int execute(const std::string &name, const char *input, char *output, size_t output_size) {
            return fossil_ai_module_execute(registry_, name.c_str(), input, output, output_size);
            }

            /**
             * @brief List all active modules in the registry.
             */
            void list() const { fossil_ai_module_list(registry_); }

            /**
             * @brief Get the number of registered modules.
             * @return Module count.
             */
            size_t count() const { return registry_->count; }

        private:
            fossil_ai_module_registry_t *registry_; /**< Underlying C registry pointer */
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
