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

/* -----------------------------------------------------------------------------
 * Internal Helpers
 * -----------------------------------------------------------------------------
 */

static int _name_equals(const char *a, const char *b) {
    return (strcmp(a, b) == 0);
}

/* -----------------------------------------------------------------------------
 * Persistence Helpers (FSON-based)
 * -----------------------------------------------------------------------------
 */

static int _module_save_to_fson(const fossil_ai_module_t *module, const char *filepath)
{
    if (!module || !filepath) return -1;
    FILE *fp = fopen(filepath, "w");
    if (!fp) return -2;

    // Serialize module as FSON object using Jellyfish API
    fossil_ai_jellyfish_fson_value_t root;
    fossil_ai_jellyfish_fson_init(&root);
    fossil_ai_jellyfish_fson_make_object(&root);

    // Basic fields
    fossil_ai_jellyfish_fson_value_t v_name, v_version, v_author, v_type, v_caps, v_loaded, v_active;
    fossil_ai_jellyfish_fson_init(&v_name);
    fossil_ai_jellyfish_fson_set_cstr(&v_name, module->name);
    fossil_ai_jellyfish_fson_object_put(&root, "name", &v_name);

    fossil_ai_jellyfish_fson_init(&v_version);
    fossil_ai_jellyfish_fson_set_cstr(&v_version, module->version);
    fossil_ai_jellyfish_fson_object_put(&root, "version", &v_version);

    fossil_ai_jellyfish_fson_init(&v_author);
    fossil_ai_jellyfish_fson_set_cstr(&v_author, module->author);
    fossil_ai_jellyfish_fson_object_put(&root, "author", &v_author);

    fossil_ai_jellyfish_fson_init(&v_type);
    fossil_ai_jellyfish_fson_set_i64(&v_type, (int64_t)module->type);
    fossil_ai_jellyfish_fson_object_put(&root, "type", &v_type);

    fossil_ai_jellyfish_fson_init(&v_caps);
    fossil_ai_jellyfish_fson_set_i64(&v_caps, (int64_t)module->capabilities);
    fossil_ai_jellyfish_fson_object_put(&root, "capabilities", &v_caps);

    fossil_ai_jellyfish_fson_init(&v_loaded);
    fossil_ai_jellyfish_fson_set_bool(&v_loaded, module->loaded);
    fossil_ai_jellyfish_fson_object_put(&root, "loaded", &v_loaded);

    fossil_ai_jellyfish_fson_init(&v_active);
    fossil_ai_jellyfish_fson_set_bool(&v_active, module->active);
    fossil_ai_jellyfish_fson_object_put(&root, "active", &v_active);

    // --- Block Attributes ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t attr_obj;
        fossil_ai_jellyfish_fson_init(&attr_obj);
        fossil_ai_jellyfish_fson_make_object(&attr_obj);

        // Direct fields
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "immutable", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "immutable"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "valid", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "valid"));
        fossil_ai_jellyfish_fson_object_put_float(&attr_obj, "confidence", fossil_ai_jellyfish_fson_object_get_float(&module->config, "confidence"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "anomaly_detected", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "anomaly_detected"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "pruned", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "pruned"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "redacted", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "redacted"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "deduplicated", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "deduplicated"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "compressed", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "compressed"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "expired", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "expired"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "trusted", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "trusted"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "conflicted", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "conflicted"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "self_healed", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "self_healed"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "auto_recovered", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "auto_recovered"));
        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "reserved", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reserved"));
        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "error_code", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "error_code"));
        fossil_ai_jellyfish_fson_object_put_float(&attr_obj, "conflict_score", fossil_ai_jellyfish_fson_object_get_float(&module->config, "conflict_score"));
        fossil_ai_jellyfish_fson_object_put_cstr(&attr_obj, "rollback_reason", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "rollback_reason"));

        // Pattern recognition & comprehension extensions
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "pattern_recognized", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "pattern_recognized"));
        fossil_ai_jellyfish_fson_object_put_bool(&attr_obj, "comprehension_success", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "comprehension_success"));
        fossil_ai_jellyfish_fson_object_put_float(&attr_obj, "pattern_similarity", fossil_ai_jellyfish_fson_object_get_float(&module->config, "pattern_similarity"));
        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "pattern_id", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "pattern_id"));
        fossil_ai_jellyfish_fson_object_put_cstr(&attr_obj, "pattern_origin_chain", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "pattern_origin_chain"));

        // Arrays: confidence_history, trust_sources, pattern_evolution, cross_chain_refs
        fossil_ai_jellyfish_fson_value_t *v_conf_hist = fossil_ai_jellyfish_fson_object_get(&module->config, "confidence_history");
        if (v_conf_hist) fossil_ai_jellyfish_fson_object_put(&attr_obj, "confidence_history", v_conf_hist);

        fossil_ai_jellyfish_fson_value_t *v_trust_sources = fossil_ai_jellyfish_fson_object_get(&module->config, "trust_sources");
        if (v_trust_sources) fossil_ai_jellyfish_fson_object_put(&attr_obj, "trust_sources", v_trust_sources);

        fossil_ai_jellyfish_fson_value_t *v_pattern_evolution = fossil_ai_jellyfish_fson_object_get(&module->config, "pattern_evolution");
        if (v_pattern_evolution) fossil_ai_jellyfish_fson_object_put(&attr_obj, "pattern_evolution", v_pattern_evolution);

        fossil_ai_jellyfish_fson_value_t *v_cross_chain_refs = fossil_ai_jellyfish_fson_object_get(&module->config, "cross_chain_refs");
        if (v_cross_chain_refs) fossil_ai_jellyfish_fson_object_put(&attr_obj, "cross_chain_refs", v_cross_chain_refs);

        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "processing_cost_ms", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "processing_cost_ms"));
        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "resource_usage", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "resource_usage"));
        fossil_ai_jellyfish_fson_object_put_i64(&attr_obj, "reasoning_path_length", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reasoning_path_length"));
        fossil_ai_jellyfish_fson_object_put_float(&attr_obj, "branch_entropy", fossil_ai_jellyfish_fson_object_get_float(&module->config, "branch_entropy"));
        fossil_ai_jellyfish_fson_object_put_float(&attr_obj, "semantic_conflict_score", fossil_ai_jellyfish_fson_object_get_float(&module->config, "semantic_conflict_score"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_attributes", &attr_obj);
        fossil_ai_jellyfish_fson_free(&attr_obj);
    }

    // --- Block Timing ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t time_obj;
        fossil_ai_jellyfish_fson_init(&time_obj);
        fossil_ai_jellyfish_fson_make_object(&time_obj);

        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "timestamp", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "timestamp"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "delta_ms", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "delta_ms"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "duration_ms", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "duration_ms"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "updated_at", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "updated_at"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "expires_at", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "expires_at"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "validated_at", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "validated_at"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "last_accessed", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "last_accessed"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "ttl_ms", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "ttl_ms"));
        fossil_ai_jellyfish_fson_object_put_i64(&time_obj, "propagation_time_ms", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "propagation_time_ms"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_time", &time_obj);
        fossil_ai_jellyfish_fson_free(&time_obj);
    }

    // --- Block Identity ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t id_obj;
        fossil_ai_jellyfish_fson_init(&id_obj);
        fossil_ai_jellyfish_fson_make_object(&id_obj);

        fossil_ai_jellyfish_fson_value_t *v_commit_hash = fossil_ai_jellyfish_fson_object_get(&module->config, "commit_hash");
        if (v_commit_hash) fossil_ai_jellyfish_fson_object_put(&id_obj, "commit_hash", v_commit_hash);

        fossil_ai_jellyfish_fson_value_t *v_parent_hashes = fossil_ai_jellyfish_fson_object_get(&module->config, "parent_hashes");
        if (v_parent_hashes) fossil_ai_jellyfish_fson_object_put(&id_obj, "parent_hashes", v_parent_hashes);

        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "parent_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "parent_count"));

        fossil_ai_jellyfish_fson_value_t *v_tree_hash = fossil_ai_jellyfish_fson_object_get(&module->config, "tree_hash");
        if (v_tree_hash) fossil_ai_jellyfish_fson_object_put(&id_obj, "tree_hash", v_tree_hash);

        fossil_ai_jellyfish_fson_value_t *v_author_id = fossil_ai_jellyfish_fson_object_get(&module->config, "author_id");
        if (v_author_id) fossil_ai_jellyfish_fson_object_put(&id_obj, "author_id", v_author_id);

        fossil_ai_jellyfish_fson_value_t *v_committer_id = fossil_ai_jellyfish_fson_object_get(&module->config, "committer_id");
        if (v_committer_id) fossil_ai_jellyfish_fson_object_put(&id_obj, "committer_id", v_committer_id);

        fossil_ai_jellyfish_fson_value_t *v_signature = fossil_ai_jellyfish_fson_object_get(&module->config, "signature");
        if (v_signature) fossil_ai_jellyfish_fson_object_put(&id_obj, "signature", v_signature);

        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "signature_len", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "signature_len"));
        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "commit_index", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "commit_index"));
        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "branch_id", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "branch_id"));
        fossil_ai_jellyfish_fson_object_put_cstr(&id_obj, "commit_message", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "commit_message"));
        fossil_ai_jellyfish_fson_object_put_bool(&id_obj, "is_merge_commit", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "is_merge_commit"));
        fossil_ai_jellyfish_fson_object_put_bool(&id_obj, "detached", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "detached"));
        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "reserved", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reserved"));
        fossil_ai_jellyfish_fson_object_put_i64(&id_obj, "revision", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "revision"));

        fossil_ai_jellyfish_fson_value_t *v_origin_chain_id = fossil_ai_jellyfish_fson_object_get(&module->config, "origin_chain_id");
        if (v_origin_chain_id) fossil_ai_jellyfish_fson_object_put(&id_obj, "origin_chain_id", v_origin_chain_id);

        fossil_ai_jellyfish_fson_object_put_cstr(&id_obj, "fork_reason", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "fork_reason"));
        fossil_ai_jellyfish_fson_object_put_cstr(&id_obj, "branch_reason", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "branch_reason"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_identity", &id_obj);
        fossil_ai_jellyfish_fson_free(&id_obj);
    }

    // --- Block Classification ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t class_obj;
        fossil_ai_jellyfish_fson_init(&class_obj);
        fossil_ai_jellyfish_fson_make_object(&class_obj);

        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "derived_from_index", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "derived_from_index"));

        fossil_ai_jellyfish_fson_value_t *v_cross_refs = fossil_ai_jellyfish_fson_object_get(&module->config, "cross_refs");
        if (v_cross_refs) fossil_ai_jellyfish_fson_object_put(&class_obj, "cross_refs", v_cross_refs);

        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "cross_ref_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "cross_ref_count"));

        fossil_ai_jellyfish_fson_value_t *v_forward_refs = fossil_ai_jellyfish_fson_object_get(&module->config, "forward_refs");
        if (v_forward_refs) fossil_ai_jellyfish_fson_object_put(&class_obj, "forward_refs", v_forward_refs);

        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "forward_ref_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "forward_ref_count"));
        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "reasoning_depth", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reasoning_depth"));
        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "reserved", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reserved"));
        fossil_ai_jellyfish_fson_object_put_cstr(&class_obj, "classification_reason", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "classification_reason"));

        fossil_ai_jellyfish_fson_value_t *v_tags = fossil_ai_jellyfish_fson_object_get(&module->config, "tags");
        if (v_tags) fossil_ai_jellyfish_fson_object_put(&class_obj, "tags", v_tags);

        fossil_ai_jellyfish_fson_object_put_float(&class_obj, "similarity_score", fossil_ai_jellyfish_fson_object_get_float(&module->config, "similarity_score"));
        fossil_ai_jellyfish_fson_object_put_bool(&class_obj, "is_hallucinated", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "is_hallucinated"));
        fossil_ai_jellyfish_fson_object_put_bool(&class_obj, "is_contradicted", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "is_contradicted"));
        fossil_ai_jellyfish_fson_object_put_float(&class_obj, "semantic_conflict_score", fossil_ai_jellyfish_fson_object_get_float(&module->config, "semantic_conflict_score"));

        fossil_ai_jellyfish_fson_value_t *v_semantic_meta = fossil_ai_jellyfish_fson_object_get(&module->config, "semantic_meta");
        if (v_semantic_meta) fossil_ai_jellyfish_fson_object_put(&class_obj, "semantic_meta", v_semantic_meta);

        fossil_ai_jellyfish_fson_object_put_cstr(&class_obj, "matched_pattern_name", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "matched_pattern_name"));
        fossil_ai_jellyfish_fson_object_put_cstr(&class_obj, "comprehension_note", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "comprehension_note"));
        fossil_ai_jellyfish_fson_object_put_cstr(&class_obj, "pattern_origin_chain", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "pattern_origin_chain"));

        fossil_ai_jellyfish_fson_value_t *v_pattern_evolution = fossil_ai_jellyfish_fson_object_get(&module->config, "pattern_evolution");
        if (v_pattern_evolution) fossil_ai_jellyfish_fson_object_put(&class_obj, "pattern_evolution", v_pattern_evolution);

        fossil_ai_jellyfish_fson_object_put_i64(&class_obj, "pattern_evolution_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "pattern_evolution_count"));
        fossil_ai_jellyfish_fson_object_put_cstr(&class_obj, "unique_comprehension_id", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "unique_comprehension_id"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_classification", &class_obj);
        fossil_ai_jellyfish_fson_free(&class_obj);
    }

    // --- Block IO ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t io_obj;
        fossil_ai_jellyfish_fson_init(&io_obj);
        fossil_ai_jellyfish_fson_make_object(&io_obj);

        fossil_ai_jellyfish_fson_object_put_cstr(&io_obj, "input", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "input"));
        fossil_ai_jellyfish_fson_object_put_cstr(&io_obj, "output", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "output"));
        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "input_len", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "input_len"));
        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "output_len", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "output_len"));

        fossil_ai_jellyfish_fson_value_t *v_input_tokens = fossil_ai_jellyfish_fson_object_get(&module->config, "input_tokens");
        if (v_input_tokens) fossil_ai_jellyfish_fson_object_put(&io_obj, "input_tokens", v_input_tokens);

        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "input_token_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "input_token_count"));

        fossil_ai_jellyfish_fson_value_t *v_output_tokens = fossil_ai_jellyfish_fson_object_get(&module->config, "output_tokens");
        if (v_output_tokens) fossil_ai_jellyfish_fson_object_put(&io_obj, "output_tokens", v_output_tokens);

        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "output_token_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "output_token_count"));

        fossil_ai_jellyfish_fson_value_t *v_input_token_probabilities = fossil_ai_jellyfish_fson_object_get(&module->config, "input_token_probabilities");
        if (v_input_token_probabilities) fossil_ai_jellyfish_fson_object_put(&io_obj, "input_token_probabilities", v_input_token_probabilities);

        fossil_ai_jellyfish_fson_value_t *v_output_token_probabilities = fossil_ai_jellyfish_fson_object_get(&module->config, "output_token_probabilities");
        if (v_output_token_probabilities) fossil_ai_jellyfish_fson_object_put(&io_obj, "output_token_probabilities", v_output_token_probabilities);

        fossil_ai_jellyfish_fson_object_put_bool(&io_obj, "compressed", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "compressed"));
        fossil_ai_jellyfish_fson_object_put_bool(&io_obj, "redacted", fossil_ai_jellyfish_fson_object_get_bool(&module->config, "redacted"));
        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "reserved", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "reserved"));

        fossil_ai_jellyfish_fson_value_t *v_io_meta = fossil_ai_jellyfish_fson_object_get(&module->config, "io_meta");
        if (v_io_meta) fossil_ai_jellyfish_fson_object_put(&io_obj, "io_meta", v_io_meta);

        fossil_ai_jellyfish_fson_value_t *v_pattern_tokens = fossil_ai_jellyfish_fson_object_get(&module->config, "pattern_tokens");
        if (v_pattern_tokens) fossil_ai_jellyfish_fson_object_put(&io_obj, "pattern_tokens", v_pattern_tokens);

        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "pattern_token_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "pattern_token_count"));
        fossil_ai_jellyfish_fson_object_put_cstr(&io_obj, "comprehension_hint", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "comprehension_hint"));

        fossil_ai_jellyfish_fson_value_t *v_attention_map = fossil_ai_jellyfish_fson_object_get(&module->config, "attention_map");
        if (v_attention_map) fossil_ai_jellyfish_fson_object_put(&io_obj, "attention_map", v_attention_map);

        fossil_ai_jellyfish_fson_value_t *v_prior_reasoning_refs = fossil_ai_jellyfish_fson_object_get(&module->config, "prior_reasoning_refs");
        if (v_prior_reasoning_refs) fossil_ai_jellyfish_fson_object_put(&io_obj, "prior_reasoning_refs", v_prior_reasoning_refs);

        fossil_ai_jellyfish_fson_object_put_i64(&io_obj, "prior_reasoning_ref_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "prior_reasoning_ref_count"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_io", &io_obj);
        fossil_ai_jellyfish_fson_free(&io_obj);
    }

    // --- Block FSON Attachments ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t fson_obj;
        fossil_ai_jellyfish_fson_init(&fson_obj);
        fossil_ai_jellyfish_fson_make_object(&fson_obj);

        fossil_ai_jellyfish_fson_value_t *v_fson_root = fossil_ai_jellyfish_fson_object_get(&module->config, "fson_root");
        if (v_fson_root) fossil_ai_jellyfish_fson_object_put(&fson_obj, "root", v_fson_root);

        fossil_ai_jellyfish_fson_value_t *v_attachments = fossil_ai_jellyfish_fson_object_get(&module->config, "attachments");
        if (v_attachments) fossil_ai_jellyfish_fson_object_put(&fson_obj, "attachments", v_attachments);

        fossil_ai_jellyfish_fson_object_put_i64(&fson_obj, "attachment_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "attachment_count"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_fson", &fson_obj);
        fossil_ai_jellyfish_fson_free(&fson_obj);
    }

    // --- Block Audit Metadata ---
    if (module->config.type == JELLYFISH_FSON_TYPE_OBJECT) {
        fossil_ai_jellyfish_fson_value_t audit_obj;
        fossil_ai_jellyfish_fson_init(&audit_obj);
        fossil_ai_jellyfish_fson_make_object(&audit_obj);

        fossil_ai_jellyfish_fson_value_t *v_audit_meta_root = fossil_ai_jellyfish_fson_object_get(&module->config, "audit_meta_root");
        if (v_audit_meta_root) fossil_ai_jellyfish_fson_object_put(&audit_obj, "audit_meta_root", v_audit_meta_root);

        fossil_ai_jellyfish_fson_value_t *v_audit_trail = fossil_ai_jellyfish_fson_object_get(&module->config, "audit_trail");
        if (v_audit_trail) fossil_ai_jellyfish_fson_object_put(&audit_obj, "audit_trail", v_audit_trail);

        fossil_ai_jellyfish_fson_object_put_i64(&audit_obj, "audit_trail_count", fossil_ai_jellyfish_fson_object_get_i64(&module->config, "audit_trail_count"));
        fossil_ai_jellyfish_fson_object_put_cstr(&audit_obj, "validation_reason", fossil_ai_jellyfish_fson_object_get_cstr(&module->config, "validation_reason"));
        fossil_ai_jellyfish_fson_object_put_float(&audit_obj, "risk_score", fossil_ai_jellyfish_fson_object_get_float(&module->config, "risk_score"));

        fossil_ai_jellyfish_fson_object_put(&root, "block_audit_meta", &audit_obj);
        fossil_ai_jellyfish_fson_free(&audit_obj);
    }

    // Serialize config as FSON (using Jellyfish API)
    fossil_ai_jellyfish_fson_value_t config_copy;
    fossil_ai_jellyfish_fson_init(&config_copy);
    fossil_ai_jellyfish_fson_copy(&module->config, &config_copy);
    fossil_ai_jellyfish_fson_object_put(&root, "config", &config_copy);

    // Serialize FSON to string
    char *fson_str = fossil_ai_jellyfish_fson_serialize(&root);
    if (fson_str) {
        fprintf(fp, "%s\n", fson_str);
        fossil_ai_jellyfish_fson_free_string(fson_str);
    }

    fossil_ai_jellyfish_fson_free(&root);
    fclose(fp);
    return 0;
}

static int _module_load_from_fson(fossil_ai_module_t *module, const char *filepath)
{
    if (!module || !filepath) return -1;
    FILE *fp = fopen(filepath, "r");
    if (!fp) return -2;

    // Read entire file into buffer
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = calloc(1, fsize + 1);
    if (!buffer) {
        fclose(fp);
        return -3;
    }
    fread(buffer, 1, fsize, fp);
    fclose(fp);

    // Parse FSON using Jellyfish API
    fossil_ai_jellyfish_fson_value_t root;
    fossil_ai_jellyfish_fson_init(&root);
    if (fossil_ai_jellyfish_fson_parse(&root, buffer) != 0) {
        free(buffer);
        fossil_ai_jellyfish_fson_free(&root);
        return -4;
    }
    free(buffer);

    // Extract fields using Jellyfish API
    fossil_ai_jellyfish_fson_value_t *v_name = fossil_ai_jellyfish_fson_object_get(&root, "name");
    fossil_ai_jellyfish_fson_value_t *v_version = fossil_ai_jellyfish_fson_object_get(&root, "version");
    fossil_ai_jellyfish_fson_value_t *v_author = fossil_ai_jellyfish_fson_object_get(&root, "author");
    fossil_ai_jellyfish_fson_value_t *v_type = fossil_ai_jellyfish_fson_object_get(&root, "type");
    fossil_ai_jellyfish_fson_value_t *v_caps = fossil_ai_jellyfish_fson_object_get(&root, "capabilities");
    fossil_ai_jellyfish_fson_value_t *v_loaded = fossil_ai_jellyfish_fson_object_get(&root, "loaded");
    fossil_ai_jellyfish_fson_value_t *v_active = fossil_ai_jellyfish_fson_object_get(&root, "active");
    fossil_ai_jellyfish_fson_value_t *v_config = fossil_ai_jellyfish_fson_object_get(&root, "config");

    if (v_name && v_name->type == JELLYFISH_FSON_TYPE_CSTR)
        strncpy(module->name, v_name->u.cstr, sizeof(module->name) - 1);
    if (v_version && v_version->type == JELLYFISH_FSON_TYPE_CSTR)
        strncpy(module->version, v_version->u.cstr, sizeof(module->version) - 1);
    if (v_author && v_author->type == JELLYFISH_FSON_TYPE_CSTR)
        strncpy(module->author, v_author->u.cstr, sizeof(module->author) - 1);
    if (v_type && (v_type->type == JELLYFISH_FSON_TYPE_I64 || v_type->type == JELLYFISH_FSON_TYPE_I32))
        module->type = (fossil_ai_module_type_t)v_type->u.i64;
    if (v_caps && (v_caps->type == JELLYFISH_FSON_TYPE_I64 || v_caps->type == JELLYFISH_FSON_TYPE_I32))
        module->capabilities = (uint32_t)v_caps->u.i64;
    if (v_loaded && v_loaded->type == JELLYFISH_FSON_TYPE_BOOL)
        module->loaded = v_loaded->u.boolean;
    if (v_active && v_active->type == JELLYFISH_FSON_TYPE_BOOL)
        module->active = v_active->u.boolean;

    // Deserialize config
    fossil_ai_jellyfish_fson_free(&module->config);
    if (v_config)
        fossil_ai_jellyfish_fson_copy(v_config, &module->config);

    fossil_ai_jellyfish_fson_free(&root);
    return 0;
}

/* ============================================================================
 * Module Factory
 * ============================================================================
 */

fossil_ai_module_t *fossil_ai_module_create(
    const char *name,
    const char *version,
    const char *author,
    fossil_ai_module_type_t type,
    uint32_t capabilities)
{
    if (!name || !version || !author)
        return NULL;

    fossil_ai_module_t *m = calloc(1, sizeof(fossil_ai_module_t));
    if (!m) return NULL;

    // Fill fields
    strncpy(m->name, name, sizeof(m->name) - 1);
    strncpy(m->version, version, sizeof(m->version) - 1);
    strncpy(m->author, author, sizeof(m->author) - 1);

    m->type = type;
    m->capabilities = capabilities;
    m->loaded = 0;
    m->active = 0;
    m->user_context = NULL;

    // Initialize empty FSON config using Jellyfish API
    fossil_ai_jellyfish_fson_init(&m->config);
    fossil_ai_jellyfish_fson_make_object(&m->config);

    // Function pointers are NULL until user assigns them
    m->on_load = NULL;
    m->on_init_chain = NULL;
    m->on_unload = NULL;
    m->on_commit = NULL;
    m->on_intercept_io = NULL;
    m->on_reason = NULL;
    m->on_maintenance = NULL;

    return m;
}

/* -----------------------------------------------------------------------------
 * Destroy Module
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_destroy(fossil_ai_module_t *module)
{
    if (!module) return;

    // Clean FSON tree using Jellyfish API
    fossil_ai_jellyfish_fson_free(&module->config);

    free(module);
}

/* -----------------------------------------------------------------------------
 * Registry Init
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_registry_init(fossil_ai_module_registry_t *reg)
{
    if (!reg) return;
    memset(reg->modules, 0, sizeof(reg->modules));
    reg->count = 0;
}

/* -----------------------------------------------------------------------------
 * Persistence: Save Registry (FSON)
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_registry_save(const fossil_ai_module_registry_t *reg, const char *dirpath)
{
    if (!reg || !dirpath) return -1;
    char filepath[512];

#if defined(_WIN32)
    const char sep = '\\';
#else
    const char sep = '/';
#endif

    for (size_t i = 0; i < reg->count; i++) {
        snprintf(filepath, sizeof(filepath), "%s%cmodule_%s.fson", dirpath, sep, reg->modules[i]->name);
        int rc = _module_save_to_fson(reg->modules[i], filepath);
        if (rc != 0) return rc;
    }
    return 0;
}

/* -----------------------------------------------------------------------------
 * Persistence: Load Registry (FSON)
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_registry_load(fossil_ai_module_registry_t *reg, const char *dirpath)
{
    if (!reg || !dirpath) return -1;
    reg->count = 0;

#if defined(_WIN32)
    // Windows: use FindFirstFile/FindNextFile
    WIN32_FIND_DATAA find_data;
    char search_path[512];
    snprintf(search_path, sizeof(search_path), "%s\\module_*.fson", dirpath);
    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s\\%s", dirpath, find_data.cFileName);

        fossil_ai_module_t *mod = calloc(1, sizeof(fossil_ai_module_t));
        if (!mod) continue;
        if (_module_load_from_fson(mod, filepath) == 0 && reg->count < FOSSIL_AI_MODULE_MAX) {
            reg->modules[reg->count++] = mod;
            mod->loaded = 1;
            mod->active = 0;
        } else {
            fossil_ai_module_destroy(mod);
        }
    } while (FindNextFileA(hFind, &find_data));
    FindClose(hFind);

#else
    // POSIX: use opendir/readdir
    DIR *dir = opendir(dirpath);
    if (!dir) return -2;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "module_") == entry->d_name &&
            strstr(entry->d_name, ".fson") != NULL)
        {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);

            fossil_ai_module_t *mod = calloc(1, sizeof(fossil_ai_module_t));
            if (!mod) continue;
            if (_module_load_from_fson(mod, filepath) == 0 && reg->count < FOSSIL_AI_MODULE_MAX) {
                reg->modules[reg->count++] = mod;
                mod->loaded = 1;
                mod->active = 0;
            } else {
                fossil_ai_module_destroy(mod);
            }
        }
    }
    closedir(dir);
#endif

    return 0;
}

/* -----------------------------------------------------------------------------
 * Load Module
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_load(fossil_ai_module_registry_t *reg, fossil_ai_module_t *module)
{
    if (!reg || !module) return -1;

    // Ensure registry is not full
    if (reg->count >= FOSSIL_AI_MODULE_MAX)
        return -2;  // registry full

    // Prevent duplicate module names
    for (size_t i = 0; i < reg->count; i++) {
        if (_name_equals(reg->modules[i]->name, module->name)) {
            return -3;  // duplicate
        }
    }

    // Add module to registry
    reg->modules[reg->count++] = module;
    module->loaded = 1;
    module->active = 0;

    // Optionally call on_load if provided
    if (module->on_load) {
        int rc = module->on_load(module);
        if (rc != 0) {
            module->loaded = 0;
            module->active = 0;
            reg->count--;
            return -4; // module refused loading
        }
    }

    return 0;
}

/* -----------------------------------------------------------------------------
 * Unload Module
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_unload(fossil_ai_module_registry_t *reg, const char *module_name)
{
    if (!reg || !module_name) return -1;

    for (size_t i = 0; i < reg->count; i++) {
        fossil_ai_module_t *m = reg->modules[i];

        if (_name_equals(m->name, module_name)) {
            // Ensure module is stopped first
            if (m->active && m->on_unload)
                m->on_unload(m);

            // Destroy module and free memory
            fossil_ai_module_destroy(m);

            // Compact array (cross-platform, works for both Windows and POSIX)
            for (size_t j = i; j + 1 < reg->count; j++)
                reg->modules[j] = reg->modules[j + 1];

            reg->modules[reg->count - 1] = NULL;
            reg->count--;

            return 0;
        }
    }

    return -2; // not found
}

/* -----------------------------------------------------------------------------
 * Start Module
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_start(fossil_ai_module_registry_t *reg, const char *module_name)
{
    if (!reg || !module_name) return -1;

    fossil_ai_module_t *m = fossil_ai_module_find(reg, module_name);
    if (!m) return -2;

    if (m->active)
        return 1; // already active

    if (m->on_load) {
        int rc = m->on_load(m);
        if (rc != 0)
            return -3; // module refused loading
    }

    m->active = 1;
    return 0;
}

/* -----------------------------------------------------------------------------
 * Stop Module
 * -----------------------------------------------------------------------------
 */

int fossil_ai_module_stop(fossil_ai_module_registry_t *reg, const char *module_name)
{
    if (!reg || !module_name) return -1;

    fossil_ai_module_t *m = fossil_ai_module_find(reg, module_name);
    if (!m) return -2;

    if (!m->active)
        return 1; // already stopped

    if (m->on_unload)
        m->on_unload(m);

    m->active = 0;

    return 0;
}

/* -----------------------------------------------------------------------------
 * Find Module by Name
 * -----------------------------------------------------------------------------
 */

fossil_ai_module_t *fossil_ai_module_find(
    fossil_ai_module_registry_t *reg,
    const char *module_name)
{
    if (!reg || !module_name) return NULL;

    for (size_t i = 0; i < reg->count; i++) {
        if (_name_equals(reg->modules[i]->name, module_name))
            return reg->modules[i];
    }

    return NULL;
}

/* -----------------------------------------------------------------------------
 * Broadcast Maintenance
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_maintenance_all(
    fossil_ai_module_registry_t *reg,
    fossil_ai_jellyfish_chain_t *chain)
{
    if (!reg) return;

    // Optionally run Jellyfish maintenance before module hooks
    fossil_ai_jellyfish_cleanup(chain);

    for (size_t i = 0; i < reg->count; i++) {
        fossil_ai_module_t *m = reg->modules[i];
        if (m->active && m->on_maintenance) {
            m->on_maintenance(m, chain);
        }
    }
}

/* -----------------------------------------------------------------------------
 * Hook: on_commit
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_hook_commit(
    fossil_ai_module_registry_t *reg,
    fossil_ai_jellyfish_chain_t *chain,
    fossil_ai_jellyfish_block_t *block)
{
    if (!reg || !chain || !block) return;

    // Optionally audit block before module hooks
    fossil_ai_jellyfish_audit(chain);

    for (size_t i = 0; i < reg->count; i++) {
        fossil_ai_module_t *m = reg->modules[i];
        if (!m->active) continue;

        if ((m->capabilities & FOSSIL_AI_MODULE_CAP_COMMIT_HOOK) &&
            m->on_commit)
        {
            m->on_commit(m, chain, block);
        }
    }
}

/* -----------------------------------------------------------------------------
 * Hook: on_intercept_io
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_hook_intercept_io(
    fossil_ai_module_registry_t *reg,
    char *input,
    char *output)
{
    if (!reg || !input || !output) return;

    for (size_t i = 0; i < reg->count; i++) {
        fossil_ai_module_t *m = reg->modules[i];
        if (!m->active) continue;

        if ((m->capabilities & FOSSIL_AI_MODULE_CAP_INTERCEPT_IO) &&
            m->on_intercept_io)
        {
            m->on_intercept_io(m, input, output);
        }
    }
}

/* -----------------------------------------------------------------------------
 * Hook: on_reason
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_hook_reason(
    fossil_ai_module_registry_t *reg,
    const char *input,
    char *inout_output,
    float *inout_confidence)
{
    if (!reg || !input || !inout_output || !inout_confidence) return;

    for (size_t i = 0; i < reg->count; i++) {
        fossil_ai_module_t *m = reg->modules[i];
        if (!m->active) continue;

        if ((m->capabilities & FOSSIL_AI_MODULE_CAP_REASON_HOOK) &&
            m->on_reason)
        {
            m->on_reason(m, input, inout_output, inout_confidence);
        }
    }
}

/* -----------------------------------------------------------------------------
 * Dump
 * -----------------------------------------------------------------------------
 */

void fossil_ai_module_dump(const fossil_ai_module_registry_t *reg)
{
    if (!reg) return;

    // Use platform-independent output (printf is available everywhere)
    printf("=== Fossil AI Module Registry ===\n");
    printf("Loaded modules: %zu\n", reg->count);

    for (size_t i = 0; i < reg->count; i++) {
        const fossil_ai_module_t *m = reg->modules[i];
        printf(" - %s (v%s) [%s] | loaded=%d active=%d caps=0x%X\n",
            m->name,
            m->version,
            m->author,
            m->loaded,
            m->active,
            (unsigned)m->capabilities);
    }
}
