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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ============================================================================
 * Module Types & Capabilities
 * ============================================================================
 */

/**
 * @enum fossil_ai_module_type_t
 * @brief Enumerates the types of AI modules supported by Fossil Logic.
 *
 * Each type describes the primary role of the module within the system.
 */
typedef enum {
    FOSSIL_AI_MODULE_UNKNOWN      = 0, /**< Unknown or unspecified module type */
    FOSSIL_AI_MODULE_ANALYZER     = 1, /**< Reads commits, provides metrics/flags */
    FOSSIL_AI_MODULE_TRANSFORMER  = 2, /**< Rewrites/modifies commits (PATCH, REBASE) */
    FOSSIL_AI_MODULE_REASONER     = 3, /**< Enhances fossil_ai_jellyfish_reason() */
    FOSSIL_AI_MODULE_CLASSIFIER   = 4, /**< Writes tags, similarity, metadata */
    FOSSIL_AI_MODULE_OBSERVER     = 5, /**< Watches chain events and logs */
    FOSSIL_AI_MODULE_TOOL         = 6, /**< General-purpose AI extension */
    FOSSIL_AI_MODULE_SECURITY     = 7, /**< Validation, scanning, integrity checks */
    FOSSIL_AI_MODULE_STORAGE      = 8  /**< Custom persistence, indexing, caching */
} fossil_ai_module_type_t;

/**
 * @enum fossil_ai_module_cap_t
 * @brief Bitmask flags describing module capabilities.
 *
 * Capabilities determine which hooks and operations a module can perform.
 */
typedef enum {
    FOSSIL_AI_MODULE_CAP_NONE          = 0,        /**< No capabilities */
    FOSSIL_AI_MODULE_CAP_READ          = 1 << 0,   /**< Can read chain/commits */
    FOSSIL_AI_MODULE_CAP_WRITE         = 1 << 1,   /**< Can write/modify commits */
    FOSSIL_AI_MODULE_CAP_INTERCEPT_IO  = 1 << 2,   /**< Can intercept input/output */
    FOSSIL_AI_MODULE_CAP_REASON_HOOK   = 1 << 3,   /**< Can hook into reasoning */
    FOSSIL_AI_MODULE_CAP_COMMIT_HOOK   = 1 << 4,   /**< Can hook into commit events */
    FOSSIL_AI_MODULE_CAP_FSON_ACCESS   = 1 << 5,   /**< Can access FSON config/data */
    FOSSIL_AI_MODULE_CAP_SECURITY      = 1 << 6    /**< Can perform security checks */
} fossil_ai_module_cap_t;

/**
 * ============================================================================
 * Module Lifecycle Structure
 * ============================================================================
 */

/**
 * @struct fossil_ai_module
 * @brief Represents a single AI module and its lifecycle/event hooks.
 *
 * This structure contains metadata, configuration, and function pointers
 * for lifecycle management and event interception.
 */
typedef struct fossil_ai_module {

    char     name[64];        /**< Module name (unique identifier) */
    char     version[32];     /**< Module version string */
    char     author[64];      /**< Module author string */

    fossil_ai_module_type_t type;    /**< Module type (see fossil_ai_module_type_t) */
    uint32_t capabilities;           /**< Bitmask of fossil_ai_module_cap_t */

    int active;              /**< Nonzero if module is currently running */
    int loaded;              /**< Nonzero if module is loaded into registry */

    /**
     * @brief Optional FSON config root for module settings.
     * Used to store module-specific configuration data.
     */
    fossil_ai_jellyfish_fson_value_t config;

    // --------------------------------------------
    // Lifecycle handlers
    // --------------------------------------------

    /**
     * @brief Called when module first loads.
     * @param m Pointer to this module.
     * @return 0 on success, nonzero on error.
     */
    int (*on_load)(struct fossil_ai_module *m);

    /**
     * @brief Called when attaching to a Jellyfish chain.
     * @param m Pointer to this module.
     * @param chain Pointer to Jellyfish chain.
     * @return 0 on success, nonzero on error.
     */
    int (*on_init_chain)(struct fossil_ai_module *m, fossil_ai_jellyfish_chain_t *chain);

    /**
     * @brief Called before shutdown/unload.
     * @param m Pointer to this module.
     * @return 0 on success, nonzero on error.
     */
    int (*on_unload)(struct fossil_ai_module *m);

    // --------------------------------------------
    // Event hooks (optional)
    // --------------------------------------------

    /**
     * @brief Called after any commit is added.
     * @param m Pointer to this module.
     * @param chain Pointer to Jellyfish chain.
     * @param block Pointer to Jellyfish block.
     * @return 0 on success, nonzero on error.
     */
    int (*on_commit)(
        struct fossil_ai_module *m,
        fossil_ai_jellyfish_chain_t *chain,
        fossil_ai_jellyfish_block_t *block);

    /**
     * @brief Pre-IO interception: modify or inspect input/output BEFORE commit creation.
     * @param m Pointer to this module.
     * @param inout_input Input string (modifiable).
     * @param inout_output Output string (modifiable).
     * @return 0 on success, nonzero on error.
     */
    int (*on_intercept_io)(
        struct fossil_ai_module *m,
        char *inout_input,
        char *inout_output);

    /**
     * @brief Intercepts reasoning results.
     * @param m Pointer to this module.
     * @param input Input string.
     * @param inout_output Output string (modifiable).
     * @param inout_confidence Confidence value (modifiable).
     * @return 0 on success, nonzero on error.
     */
    int (*on_reason)(
        struct fossil_ai_module *m,
        const char *input,
        char *inout_output,
        float *inout_confidence);

    /**
     * @brief Periodic maintenance hook (optional).
     * @param m Pointer to this module.
     * @param chain Pointer to Jellyfish chain.
     * @return 0 on success, nonzero on error.
     */
    int (*on_maintenance)(
        struct fossil_ai_module *m,
        fossil_ai_jellyfish_chain_t *chain);

    /**
     * @brief Optional: module-specific context pointer.
     * Used for storing user-defined data.
     */
    void *user_context;

} fossil_ai_module_t;

/**
 * ============================================================================
 * Module Registry
 * ============================================================================
 */

/**
 * @def FOSSIL_AI_MODULE_MAX
 * @brief Maximum number of modules supported in a registry.
 */
#define FOSSIL_AI_MODULE_MAX 32

/**
 * @struct fossil_ai_module_registry_t
 * @brief Registry for managing loaded AI modules.
 *
 * Contains an array of module pointers and a count of active modules.
 */
typedef struct {
    fossil_ai_module_t *modules[FOSSIL_AI_MODULE_MAX]; /**< Array of module pointers */
    size_t count;                                     /**< Number of modules loaded */
} fossil_ai_module_registry_t;

/**
 * ============================================================================
 * Global Registry Operations
 * ============================================================================
 */

/**
 * Create a new AI module with default values.
 * Allocates heap memory for the module.
 *
 * @param name Name of the module (required)
 * @param version Module version string
 * @param author Module author
 * @param type Module type from fossil_ai_module_type_t
 * @param capabilities Bitmask from fossil_ai_module_cap_t
 * @return Pointer to allocated module, or NULL on error
 */
fossil_ai_module_t *fossil_ai_module_create(
    const char *name,
    const char *version,
    const char *author,
    fossil_ai_module_type_t type,
    uint32_t capabilities);

/**
 * Destroy a module created with fossil_ai_module_create().
 * Does NOT unload from registry — caller must unregister first.
 */
void fossil_ai_module_destroy(fossil_ai_module_t *module);

/**
 * @brief Initialize module system registry.
 * @param reg Pointer to module registry.
 */
void fossil_ai_module_registry_init(fossil_ai_module_registry_t *reg);

/**
 * @brief Load module into registry (does not activate yet).
 * @param reg Pointer to module registry.
 * @param module Pointer to module to load.
 * @return 0 on success, nonzero on error.
 */
int fossil_ai_module_load(fossil_ai_module_registry_t *reg, fossil_ai_module_t *module);

/**
 * @brief Unload module from registry.
 * @param reg Pointer to module registry.
 * @param module_name Name of module to unload.
 * @return 0 on success, nonzero on error.
 */
int fossil_ai_module_unload(fossil_ai_module_registry_t *reg, const char *module_name);

/**
 * @brief Activate module (calls on_load, marks active).
 * @param reg Pointer to module registry.
 * @param module_name Name of module to activate.
 * @return 0 on success, nonzero on error.
 */
int fossil_ai_module_start(fossil_ai_module_registry_t *reg, const char *module_name);

/**
 * @brief Deactivate module (calls on_unload).
 * @param reg Pointer to module registry.
 * @param module_name Name of module to deactivate.
 * @return 0 on success, nonzero on error.
 */
int fossil_ai_module_stop(fossil_ai_module_registry_t *reg, const char *module_name);

/**
 * @brief Find module by name.
 * @param reg Pointer to module registry.
 * @param module_name Name of module to find.
 * @return Pointer to found module, or NULL if not found.
 */
fossil_ai_module_t *fossil_ai_module_find(
    fossil_ai_module_registry_t *reg, const char *module_name);

/**
 * @brief Broadcast maintenance call to all modules.
 * @param reg Pointer to module registry.
 * @param chain Pointer to Jellyfish chain.
 */
void fossil_ai_module_maintenance_all(
    fossil_ai_module_registry_t *reg, fossil_ai_jellyfish_chain_t *chain);

/**
 * ============================================================================
 * Integration Hooks to Jellyfish AI
 * ============================================================================
 *
 * These functions are intended to be called inside Jellyfish operations.
 */

/**
 * @brief After commit creation, call commit hooks for all modules.
 * @param reg Pointer to module registry.
 * @param chain Pointer to Jellyfish chain.
 * @param block Pointer to Jellyfish block.
 */
void fossil_ai_module_hook_commit(
    fossil_ai_module_registry_t *reg,
    fossil_ai_jellyfish_chain_t *chain,
    fossil_ai_jellyfish_block_t *block);

/**
 * @brief Before commit creation (input/output interception).
 * Calls IO interception hooks for all modules.
 * @param reg Pointer to module registry.
 * @param input Input string (modifiable).
 * @param output Output string (modifiable).
 */
void fossil_ai_module_hook_intercept_io(
    fossil_ai_module_registry_t *reg,
    char *input,
    char *output);

/**
 * @brief After Jellyfish reason() picks an output, call reason hooks for all modules.
 * @param reg Pointer to module registry.
 * @param input Input string.
 * @param inout_output Output string (modifiable).
 * @param inout_confidence Confidence value (modifiable).
 */
void fossil_ai_module_hook_reason(
    fossil_ai_module_registry_t *reg,
    const char *input,
    char *inout_output,
    float *inout_confidence);

/**
 * @brief Print module registry state to stdout.
 * @param reg Pointer to module registry.
 */
void fossil_ai_module_dump(const fossil_ai_module_registry_t *reg);

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
             * @brief Construct a Module wrapper from a fossil_ai_module_t pointer.
             * @param mod Pointer to fossil_ai_module_t.
             */
            Module(fossil_ai_module_t *mod) : module_(mod) {}

            /** @brief Get module name. */
            const char* name() const { return module_->name; }
            /** @brief Get module version string. */
            const char* version() const { return module_->version; }
            /** @brief Get module author string. */
            const char* author() const { return module_->author; }
            /** @brief Get module type (enum fossil_ai_module_type_t). */
            fossil_ai_module_type_t type() const { return module_->type; }
            /** @brief Get module capabilities bitmask. */
            uint32_t capabilities() const { return module_->capabilities; }
            /** @brief Returns true if module is active (running). */
            bool is_active() const { return module_->active != 0; }
            /** @brief Returns true if module is loaded in registry. */
            bool is_loaded() const { return module_->loaded != 0; }

            /**
             * @brief Call module's on_load handler if present.
             * @return Handler return value or 0 if not present.
             */
            int on_load() {
            if (module_->on_load)
                return module_->on_load(module_);
            return 0;
            }

            /**
             * @brief Call module's on_init_chain handler if present.
             * @param chain Pointer to Jellyfish chain.
             * @return Handler return value or 0 if not present.
             */
            int on_init_chain(fossil_ai_jellyfish_chain_t *chain) {
            if (module_->on_init_chain)
                return module_->on_init_chain(module_, chain);
            return 0;
            }

            /**
             * @brief Call module's on_unload handler if present.
             * @return Handler return value or 0 if not present.
             */
            int on_unload() {
            if (module_->on_unload)
                return module_->on_unload(module_);
            return 0;
            }

            /**
             * @brief Call module's on_commit event hook if present.
             * @param chain Pointer to Jellyfish chain.
             * @param block Pointer to Jellyfish block.
             * @return Handler return value or 0 if not present.
             */
            int on_commit(fossil_ai_jellyfish_chain_t *chain, fossil_ai_jellyfish_block_t *block) {
            if (module_->on_commit)
                return module_->on_commit(module_, chain, block);
            return 0;
            }

            /**
             * @brief Call module's on_intercept_io event hook if present.
             * Allows inspection/modification of input/output before commit creation.
             * @param inout_input Input string (modifiable).
             * @param inout_output Output string (modifiable).
             * @return Handler return value or 0 if not present.
             */
            int on_intercept_io(char *inout_input, char *inout_output) {
            if (module_->on_intercept_io)
                return module_->on_intercept_io(module_, inout_input, inout_output);
            return 0;
            }

            /**
             * @brief Call module's on_reason event hook if present.
             * Allows interception of reasoning results.
             * @param input Input string.
             * @param inout_output Output string (modifiable).
             * @param inout_confidence Confidence value (modifiable).
             * @return Handler return value or 0 if not present.
             */
            int on_reason(const char *input, char *inout_output, float *inout_confidence) {
            if (module_->on_reason)
                return module_->on_reason(module_, input, inout_output, inout_confidence);
            return 0;
            }

            /**
             * @brief Call module's on_maintenance event hook if present.
             * Used for periodic maintenance tasks.
             * @param chain Pointer to Jellyfish chain.
             * @return Handler return value or 0 if not present.
             */
            int on_maintenance(fossil_ai_jellyfish_chain_t *chain) {
            if (module_->on_maintenance)
                return module_->on_maintenance(module_, chain);
            return 0;
            }

            /** @brief Get module-specific user context pointer. */
            void* user_context() const { return module_->user_context; }
            /** @brief Set module-specific user context pointer. */
            void set_user_context(void* ctx) { module_->user_context = ctx; }

            /** @brief Access module configuration (FSON value, mutable). */
            fossil_ai_jellyfish_fson_value_t& config() { return module_->config; }
            /** @brief Access module configuration (FSON value, const). */
            const fossil_ai_jellyfish_fson_value_t& config() const { return module_->config; }

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
             * @brief Construct a ModuleRegistry wrapper from a fossil_ai_module_registry_t pointer.
             * @param reg Pointer to fossil_ai_module_registry_t.
             */
            ModuleRegistry(fossil_ai_module_registry_t *reg) : registry_(reg) {}

            /** @brief Initialize the module registry. */
            void init() { fossil_ai_module_registry_init(registry_); }

            /**
             * @brief Load a module into the registry (does not activate).
             * @param mod Module wrapper.
             * @return 0 on success, nonzero on error.
             */
            int load(Module &mod) { return fossil_ai_module_load(registry_, mod.module_); }

            /**
             * @brief Unload a module from the registry by name.
             * @param name Module name.
             * @return 0 on success, nonzero on error.
             */
            int unload(const std::string &name) { return fossil_ai_module_unload(registry_, name.c_str()); }

            /**
             * @brief Activate a module by name (calls on_load).
             * @param name Module name.
             * @return 0 on success, nonzero on error.
             */
            int start(const std::string &name) { return fossil_ai_module_start(registry_, name.c_str()); }

            /**
             * @brief Deactivate a module by name (calls on_unload).
             * @param name Module name.
             * @return 0 on success, nonzero on error.
             */
            int stop(const std::string &name) { return fossil_ai_module_stop(registry_, name.c_str()); }

            /**
             * @brief Find a module by name.
             * @param name Module name.
             * @return Module wrapper for found module.
             * @throws std::runtime_error if not found.
             */
            Module find(const std::string &name) {
            fossil_ai_module_t *mod = fossil_ai_module_find(registry_, name.c_str());
            if (!mod) throw std::runtime_error("Module not found");
            return Module(mod);
            }

            /**
             * @brief Broadcast maintenance call to all modules.
             * @param chain Pointer to Jellyfish chain.
             */
            void maintenance_all(fossil_ai_jellyfish_chain_t *chain) {
            fossil_ai_module_maintenance_all(registry_, chain);
            }

            /**
             * @brief Call commit hook for all modules after commit creation.
             * @param chain Pointer to Jellyfish chain.
             * @param block Pointer to Jellyfish block.
             */
            void hook_commit(fossil_ai_jellyfish_chain_t *chain, fossil_ai_jellyfish_block_t *block) {
            fossil_ai_module_hook_commit(registry_, chain, block);
            }

            /**
             * @brief Call IO interception hook for all modules before commit creation.
             * @param input Input string (modifiable).
             * @param output Output string (modifiable).
             */
            void hook_intercept_io(char *input, char *output) {
            fossil_ai_module_hook_intercept_io(registry_, input, output);
            }

            /**
             * @brief Call reason hook for all modules after reasoning output.
             * @param input Input string.
             * @param inout_output Output string (modifiable).
             * @param inout_confidence Confidence value (modifiable).
             */
            void hook_reason(const char *input, char *inout_output, float *inout_confidence) {
            fossil_ai_module_hook_reason(registry_, input, inout_output, inout_confidence);
            }

            /** @brief Print module registry state to stdout. */
            void dump() const { fossil_ai_module_dump(registry_); }

            /** @brief Get number of modules currently in registry. */
            size_t count() const { return registry_->count; }

        private:
            fossil_ai_module_registry_t *registry_; /**< Underlying C registry pointer */
        };

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
