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
#ifndef FOSSIL_JELLYFISH_AI_H
#define FOSSIL_JELLYFISH_AI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <float.h>
#include <time.h>
#include <math.h>

enum {
    FOSSIL_JELLYFISH_MAX_MEM          = 128,
    FOSSIL_JELLYFISH_HASH_SIZE        = 32,
    FOSSIL_JELLYFISH_INPUT_SIZE       = 64,
    FOSSIL_JELLYFISH_OUTPUT_SIZE      = 64,
    FOSSIL_JELLYFISH_MAX_MODELS       = 32,
    FOSSIL_JELLYFISH_MAX_TOKENS       = 16,
    FOSSIL_JELLYFISH_TOKEN_SIZE       = 16,
    FOSSIL_JELLYFISH_MAX_MODEL_FILES  = 16,
    FOSSIL_JELLYFISH_MAX_TAGS         = 8,
    FOSSIL_JELLYFISH_MAX_BRANCHES      = 8
};

#define FOSSIL_DEVICE_ID_SIZE      16   // E.g., 128-bit hardware ID
#define FOSSIL_SIGNATURE_SIZE      64   // ECDSA, ED25519, etc.
#define FOSSIL_JELLYFISH_MAX_LINKS 4

#ifdef __cplusplus
extern "C"
{
#endif

// *****************************************************************************
// Type definitions — Jellyfish AI Git-Chain Hybrid (Enhanced)
// *****************************************************************************

/**
 * @brief Enumerates commit types for the Jellyfish AI Git-chain hybrid.
 *
 * These types model how knowledge evolves, diverges, merges, and stabilizes,
 * mirroring Git operations while preserving AI reasoning intent.
 */
typedef enum {
    // -----------------------------------------------------------------
    // Core commits — foundational and reasoning states
    // -----------------------------------------------------------------
    JELLY_COMMIT_UNKNOWN = 0,          /**< Undefined or placeholder commit */
    JELLY_COMMIT_INIT = 1,             /**< Genesis or initial commit in a chain */
    JELLY_COMMIT_OBSERVE = 2,          /**< Observation or raw data intake */
    JELLY_COMMIT_INFER = 3,            /**< Logical derivation or reasoning commit */
    JELLY_COMMIT_VALIDATE = 4,         /**< Verified or confirmed (trusted) result */
    JELLY_COMMIT_PATCH = 5,            /**< Correction or manual hotfix commit */

    // -----------------------------------------------------------------
    // Branching and merging — reasoning paths and reconciliation
    // -----------------------------------------------------------------
    JELLY_COMMIT_BRANCH = 10,          /**< Divergent reasoning path or hypothesis branch */
    JELLY_COMMIT_MERGE = 11,           /**< Merge of two or more reasoning lines */
    JELLY_COMMIT_REBASE = 12,          /**< Rebased logic onto new foundation */
    JELLY_COMMIT_CHERRY_PICK = 13,     /**< Selective logic adoption (copy commit) */
    JELLY_COMMIT_FORK = 14,            /**< Forked repository / cloned memory state */
    JELLY_COMMIT_RESOLVE_MANUAL = 15,  /**< Conflict resolved manually (human-assisted) */
    JELLY_COMMIT_RESOLVE_AUTO = 16,    /**< Conflict resolved automatically (AI) */

    // -----------------------------------------------------------------
    // Tagging, releases, and archival
    // -----------------------------------------------------------------
    JELLY_COMMIT_TAG = 20,             /**< Stable tagged version (snapshot) */
    JELLY_COMMIT_RELEASE = 21,         /**< Publicly declared stable reasoning state */
    JELLY_COMMIT_ARCHIVE = 22,         /**< Frozen, immutable version */
    JELLY_COMMIT_SNAPSHOT = 23,        /**< Temporary snapshot (autosave/checkpoint) */

    // -----------------------------------------------------------------
    // Experimental and ephemeral states
    // -----------------------------------------------------------------
    JELLY_COMMIT_EXPERIMENT = 30,      /**< Experimental reasoning branch */
    JELLY_COMMIT_STASH = 31,           /**< Temporary hold or deferred logic */
    JELLY_COMMIT_DRAFT = 32,           /**< Work-in-progress state (unvalidated) */
    JELLY_COMMIT_REVERT = 33,          /**< Undo/reversal of prior commit */
    JELLY_COMMIT_ROLLBACK = 34,        /**< Forced rollback (AI self-correction) */

    // -----------------------------------------------------------------
    // Collaboration, synchronization, and meta-commits
    // -----------------------------------------------------------------
    JELLY_COMMIT_SYNC = 40,            /**< Synced with external node or agent */
    JELLY_COMMIT_MIRROR = 41,          /**< Mirrored repository state */
    JELLY_COMMIT_IMPORT = 42,          /**< Imported external dataset or logic */
    JELLY_COMMIT_EXPORT = 43,          /**< Exported memory snapshot */
    JELLY_COMMIT_SIGNED = 44,          /**< Verified signed commit (authoritative) */
    JELLY_COMMIT_REVIEW = 45,          /**< Peer-reviewed or audited commit */

    // -----------------------------------------------------------------
    // Special and terminal states
    // -----------------------------------------------------------------
    JELLY_COMMIT_DETACHED = 50,        /**< Detached HEAD / isolated reasoning path */
    JELLY_COMMIT_ABANDONED = 51,       /**< Dropped or deprecated branch */
    JELLY_COMMIT_CONFLICT = 52,        /**< Conflict-resolving commit */
    JELLY_COMMIT_PRUNE = 53,           /**< Commit pruned from history */
    JELLY_COMMIT_FINAL = 54            /**< Terminal or end-of-life commit */
} fossil_ai_jellyfish_commit_type_t;

/* Capacity limits (tunable) */
enum {
    FOSSIL_JELLYFISH_FSON_MAX_ARRAY  = 32,   /**< Maximum array elements */
    FOSSIL_JELLYFISH_FSON_MAX_OBJECT = 32,   /**< Maximum object key/value pairs */
    FOSSIL_JELLYFISH_KEY_SIZE        = 32,   /**< Maximum key string length */
    FOSSIL_JELLYFISH_CONFIDENCE_HISTORY_SIZE = 8, /**< Confidence history array size */
    FOSSIL_JELLYFISH_TRUST_SOURCES_SIZE = 8,      /**< Trust sources array size */
    FOSSIL_JELLYFISH_PATTERN_EVOLUTION_SIZE = 8,  /**< Pattern evolution array size */
    FOSSIL_JELLYFISH_AUDIT_TRAIL_SIZE = 8,        /**< Audit trail array size */
    FOSSIL_JELLYFISH_TOKEN_PROB_SIZE = FOSSIL_JELLYFISH_MAX_TOKENS
};

/**
 * @brief Block Attributes
 * Similar to commit metadata and trust heuristics.
 * Extended to support pattern recognition, comprehension metrics,
 * and cross-referencing for rapid AI reasoning and common sense logic.
 */
typedef struct {
    int immutable;                   /**< 1 if block is immutable */
    int valid;                       /**< 1 if block is valid */
    float confidence;                /**< Confidence score (0.0-1.0) */
    float confidence_history[FOSSIL_JELLYFISH_CONFIDENCE_HISTORY_SIZE]; /**< Temporal confidence evolution */
    size_t confidence_history_count; /**< Number of confidence history entries */
    char trust_sources[FOSSIL_JELLYFISH_TRUST_SOURCES_SIZE][32]; /**< Nodes/agents contributing to validation */
    size_t trust_sources_count;      /**< Number of trust sources */
    int anomaly_detected;            /**< 1 if unusual reasoning pattern detected */
    int pruned;                      /**< 1 if block is pruned */
    int redacted;                    /**< 1 if block is redacted */
    int deduplicated;                /**< 1 if block is deduplicated */
    int compressed;                  /**< 1 if block is compressed */
    int expired;                     /**< 1 if block is expired */
    int trusted;                     /**< 1 if block is trusted */
    int conflicted;                  /**< 1 if block is conflicted */
    int self_healed;                 /**< 1 if block was self-healed */
    int auto_recovered;              /**< 1 if block was auto-recovered */
    int reserved;                    /**< Reserved for future use */
    int error_code;                  /**< Error/exception code for debugging */
    float conflict_score;            /**< Quantifies conflict severity */
    char rollback_reason[64];        /**< Reason for rollback (if any) */

    // --- Pattern recognition & comprehension extensions ---
    int pattern_recognized;          /**< 1 if a known pattern was matched */
    int comprehension_success;       /**< 1 if block enabled rapid solution via comprehension */
    float pattern_similarity;        /**< Similarity score to known patterns (0.0-1.0) */
    uint32_t pattern_id;             /**< Optional: ID/index of matched pattern */
    char pattern_origin_chain[FOSSIL_DEVICE_ID_SIZE]; /**< Chain ID where pattern originated */
    float pattern_evolution[FOSSIL_JELLYFISH_PATTERN_EVOLUTION_SIZE]; /**< Pattern change history */
    size_t pattern_evolution_count;  /**< Number of pattern evolution entries */

    // --- Cross-referencing for memory chains ---
    uint32_t cross_chain_refs[FOSSIL_JELLYFISH_MAX_LINKS]; /**< References to other chains for memory linking */
    size_t cross_chain_ref_count;     /**< Number of cross-chain references */

    // --- Metrics & analytics ---
    uint32_t processing_cost_ms;      /**< Processing cost in ms */
    uint32_t resource_usage;          /**< Resource usage metric */
    uint16_t reasoning_path_length;   /**< DAG traversal depth */
    float branch_entropy;             /**< Divergence complexity metric */
    float semantic_conflict_score;    /**< Contradictory semantic mapping score */
} fossil_ai_jellyfish_block_attributes_t;

/**
 * @brief Block Timing Information
 * Stores timestamps and durations for block lifecycle.
 */
typedef struct {
    uint64_t timestamp;              /**< Creation time (commit time) */
    uint32_t delta_ms;               /**< Time since parent commit (ms) */
    uint32_t duration_ms;            /**< Processing or IO duration (ms) */
    uint64_t updated_at;             /**< Last updated timestamp */
    uint64_t expires_at;             /**< Expiry timestamp */
    uint64_t validated_at;           /**< Validation timestamp */
    uint64_t last_accessed;          /**< Last accessed timestamp */
    uint32_t ttl_ms;                 /**< Time-to-live in ms for ephemeral blocks */
    uint32_t propagation_time_ms;    /**< Cross-chain syncing latency */
} fossil_ai_jellyfish_block_time_t;

/**
 * @brief Identification & Ancestry (Git-style DAG commit model)
 * Stores hashes, author/committer IDs, and ancestry info.
 */
typedef struct {
    uint8_t commit_hash[FOSSIL_JELLYFISH_HASH_SIZE];       /**< Unique commit identifier */
    uint8_t parent_hashes[4][FOSSIL_JELLYFISH_HASH_SIZE];  /**< Up to 4 parent hashes (for merges) */
    size_t parent_count;                                   /**< Number of parent commits */
    uint8_t tree_hash[FOSSIL_JELLYFISH_HASH_SIZE];         /**< Tree-like content snapshot hash */
    uint8_t author_id[FOSSIL_DEVICE_ID_SIZE];              /**< Author (device/user) ID */
    uint8_t committer_id[FOSSIL_DEVICE_ID_SIZE];           /**< Committer (system or AI agent) ID */
    uint8_t signature[FOSSIL_SIGNATURE_SIZE];              /**< Signature for integrity */
    uint32_t signature_len;                                /**< Length of signature */
    uint32_t commit_index;                                 /**< Local index in memory */
    uint32_t branch_id;                                    /**< Logical branch index (for merges) */
    char commit_message[256];                              /**< Human-readable reasoning summary */
    int is_merge_commit;                                   /**< 1 if multi-parent */
    int detached;                                          /**< 1 if not attached to mainline */
    uint32_t reserved;                                     /**< Reserved for future use */
    uint32_t revision;                                     /**< Explicit revision number */
    uint8_t origin_chain_id[FOSSIL_DEVICE_ID_SIZE];        /**< Chain where block was first committed */
    char fork_reason[128];                                 /**< Reason for fork/divergence */
    char branch_reason[128];                               /**< Reason for branch creation */
} fossil_ai_jellyfish_block_identity_t;

/**
 * @brief Classification / Semantic Relationships
 * Extended for pattern and comprehension tracking,
 * and cross-chain comprehension for rapid, common sense logic.
 */
typedef struct {
    uint32_t derived_from_index;                           /**< Logical origin index */
    uint32_t cross_refs[FOSSIL_JELLYFISH_MAX_LINKS];       /**< Cross-branch references */
    size_t cross_ref_count;                                /**< Number of cross references */
    uint32_t forward_refs[FOSSIL_JELLYFISH_MAX_LINKS];     /**< Future derivations */
    size_t forward_ref_count;                              /**< Number of forward references */
    uint16_t reasoning_depth;                              /**< Reasoning depth */
    uint16_t reserved;                                     /**< Reserved for future use */
    char classification_reason[128];                       /**< Reason for classification */
    char tags[FOSSIL_JELLYFISH_MAX_TAGS][32];              /**< Classification tags */
    float similarity_score;                                /**< Similarity score */
    int is_hallucinated;                                   /**< 1 if block is hallucinated */
    int is_contradicted;                                   /**< 1 if block is contradicted */
    float semantic_conflict_score;                         /**< Contradictory semantic mapping score */

    // --- Pattern recognition & comprehension extensions ---
    char matched_pattern_name[64];                         /**< Name/label of matched pattern (if any) */
    char comprehension_note[128];                          /**< Explanation of comprehension (if rapid solution applied) */
    char pattern_origin_chain[FOSSIL_DEVICE_ID_SIZE];      /**< Chain ID where pattern originated */
    float pattern_evolution[FOSSIL_JELLYFISH_PATTERN_EVOLUTION_SIZE]; /**< Pattern change history */
    size_t pattern_evolution_count;                        /**< Number of pattern evolution entries */

    // --- Unique comprehension for rapid logic ---
    char unique_comprehension_id[32];                      /**< Unique ID for comprehension logic (cross-chain) */
} fossil_ai_jellyfish_block_classification_t;

/**
 * @brief Input/Output Core Payload
 * Extended for pattern tokens and comprehension hints.
 */
typedef struct {
    char input[FOSSIL_JELLYFISH_INPUT_SIZE];               /**< Input string */
    char output[FOSSIL_JELLYFISH_OUTPUT_SIZE];             /**< Output string */
    size_t input_len;                                      /**< Length of input */
    size_t output_len;                                     /**< Length of output */
    char input_tokens[FOSSIL_JELLYFISH_MAX_TOKENS][FOSSIL_JELLYFISH_TOKEN_SIZE];   /**< Tokenized input */
    size_t input_token_count;                              /**< Number of input tokens */
    char output_tokens[FOSSIL_JELLYFISH_MAX_TOKENS][FOSSIL_JELLYFISH_TOKEN_SIZE];  /**< Tokenized output */
    size_t output_token_count;                             /**< Number of output tokens */
    float input_token_probabilities[FOSSIL_JELLYFISH_TOKEN_PROB_SIZE];             /**< AI certainty per input token */
    float output_token_probabilities[FOSSIL_JELLYFISH_TOKEN_PROB_SIZE];            /**< AI certainty per output token */
    int compressed;                                        /**< 1 if compressed */
    int redacted;                                          /**< 1 if redacted */
    int reserved;                                          /**< Reserved for future use */

    // --- Pattern recognition & comprehension extensions ---
    char pattern_tokens[FOSSIL_JELLYFISH_MAX_TOKENS][FOSSIL_JELLYFISH_TOKEN_SIZE]; /**< Tokens for pattern matching */
    size_t pattern_token_count;                            /**< Number of pattern tokens */
    char comprehension_hint[64];                           /**< Hint for rapid comprehension (e.g., "Pythagorean") */
    float attention_map[FOSSIL_JELLYFISH_MAX_TOKENS];      /**< Attention weights per token */
    uint32_t prior_reasoning_refs[FOSSIL_JELLYFISH_MAX_LINKS]; /**< References to prior reasoning steps */
    size_t prior_reasoning_ref_count;                      /**< Number of prior reasoning references */
} fossil_ai_jellyfish_block_io_t;

/**
 * @brief Flexible Structured Object Notation (FSON) attachments
 * Allows arbitrary structured metadata (extended reasoning traces, metrics, lineage).
 */
typedef enum {
    JELLYFISH_ATTACHMENT_TRACE = 0,
    JELLYFISH_ATTACHMENT_LOG,
    JELLYFISH_ATTACHMENT_EMBEDDING,
    JELLYFISH_ATTACHMENT_ANNOTATION,
    JELLYFISH_ATTACHMENT_OTHER
} fossil_ai_jellyfish_attachment_type_t;

/**
 * @brief Audit Metadata & Compliance
 * Extended for chain-of-evidence and risk tracking.
 */
typedef struct {
    uint8_t audit_trail[FOSSIL_JELLYFISH_AUDIT_TRAIL_SIZE][FOSSIL_JELLYFISH_HASH_SIZE]; /**< Hashes of prior audits */
    size_t audit_trail_count;                              /**< Number of audit trail entries */
    char validation_reason[128];                           /**< Reason for validation */
    float risk_score;                                      /**< Potential reasoning risk */
} fossil_ai_jellyfish_block_audit_meta_t;

/**
 * @brief Complete Git-Chain Block (Commit)
 * Capable of cross-referencing memory chains and comprehension logic.
 */
typedef struct {
    fossil_ai_jellyfish_block_io_t io;                     /**< Input/output payload */
    fossil_ai_jellyfish_block_identity_t identity;         /**< Identification and ancestry */
    fossil_ai_jellyfish_block_time_t time;                 /**< Timing information */
    fossil_ai_jellyfish_block_attributes_t attributes;     /**< Block attributes and trust metrics */
    fossil_ai_jellyfish_commit_type_t block_type;          /**< Commit type */
    fossil_ai_jellyfish_block_classification_t classify;   /**< Classification and semantic relationships */
    fossil_ai_jellyfish_block_audit_meta_t audit_meta;     /**< Per-block audit/validation record (OBJECT) */
} fossil_ai_jellyfish_block_t;

/**
 * @brief Git-like Repository of Jellyfish Blocks
 * Supports cross-chain memory referencing and comprehension logic.
 * This struct is the runtime container for Jellyfish AI.
 */
typedef struct {
    fossil_ai_jellyfish_block_t commits[FOSSIL_JELLYFISH_MAX_MEM]; /**< Array of blocks (commits) */
    size_t count;                                                  /**< Number of blocks in chain */

    uint8_t repo_id[FOSSIL_DEVICE_ID_SIZE];                        /**< Unique repository/device ID */
    char default_branch[64];                                       /**< Default branch name */
    uint64_t created_at;                                           /**< Repository creation timestamp */
    uint64_t updated_at;                                           /**< Last update timestamp */

    // Branch & reference metadata
    struct {
        char name[64];                                             /**< Branch name */
        uint8_t head_hash[FOSSIL_JELLYFISH_HASH_SIZE];             /**< Head commit hash for branch */
        char branch_reason[128];                                   /**< Reason for branch creation */
    } branches[FOSSIL_JELLYFISH_MAX_BRANCHES];
    size_t branch_count;                                           /**< Number of branches */

    // --- Cross-chain comprehension references ---
    uint8_t cross_chain_ids[FOSSIL_JELLYFISH_MAX_LINKS][FOSSIL_DEVICE_ID_SIZE]; /**< Referenced external chains */
    size_t cross_chain_count;                                      /**< Number of cross-chain references */
} fossil_ai_jellyfish_chain_t;

// *****************************************************************************
// Function prototypes
// *****************************************************************************

/**
 * Generate a hash for the given input and output.
 * Uses a custom FNV/SBOX/avalanche hash with device salt and time nonce.
 * Not cryptographically secure; suitable for content addressing and
 * fingerprinting only.
 *
 * @param input     Input string (null-terminated).
 * @param output    Output string (null-terminated).
 * @param hash_out  Buffer for resulting hash (FOSSIL_JELLYFISH_HASH_SIZE bytes).
 */
void fossil_ai_jellyfish_hash(const char *input, const char *output, uint8_t *hash_out);

/**
 * Initialize the jellyfish chain (Git-chain + FSON aware).
 * Resets all commit slots, initializes repo/branch metadata, sets timestamps,
 * and prepares FSON roots. Safe for reuse; does not leak heap memory.
 */
void fossil_ai_jellyfish_init(fossil_ai_jellyfish_chain_t *chain);

/**
 * Learn a new input-output pair (creates an INFER commit).
 * Tokenizes input/output, computes hash, initializes confidence, and
 * updates branch head. Uses first available slot or reuses invalidated slots.
 */
void fossil_ai_jellyfish_learn(fossil_ai_jellyfish_chain_t *chain, const char *input, const char *output);

/**
 * Logically remove a block at index (marks as invalid/pruned).
 * Physical compaction deferred to chain_compact.
 */
void fossil_ai_jellyfish_remove(fossil_ai_jellyfish_chain_t *chain, size_t index);

/**
 * Find a block by its hash (linear scan, valid blocks only).
 * Returns pointer to block or NULL.
 */
fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_find(fossil_ai_jellyfish_chain_t *chain, const uint8_t *hash);

/**
 * Update a block's IO payload (if not immutable) and rehash content.
 * Retokenizes, upgrades type if needed, and updates branch heads.
 */
void fossil_ai_jellyfish_update(fossil_ai_jellyfish_chain_t *chain, size_t index, const char *input, const char *output);

/* ---------------------------- Persistence ---------------------------------- */

/**
 * Serialize chain to a binary file (fixed-structure dump).
 * Only valid blocks are written; FSON subtrees are reinitialized as empty.
 * Returns 0 on success, -1 on error.
 */
int fossil_ai_jellyfish_save(const fossil_ai_jellyfish_chain_t *chain, const char *filepath);

/**
 * Load chain from binary snapshot (overwrites in-memory state).
 * Reconstructs valid blocks and branch metadata; FSON subtrees are empty.
 * Returns 0 on success, -1 on error.
 */
int fossil_ai_jellyfish_load(fossil_ai_jellyfish_chain_t *chain, const char *filepath);

/* ----------------------------- Maintenance --------------------------------- */

/**
 * Cleanup pass: expires blocks, normalizes counts, sanitizes branch heads,
 * and shrinks chain->count to highest valid+1.
 */
void fossil_ai_jellyfish_cleanup(fossil_ai_jellyfish_chain_t *chain);

/**
 * Audit pass: scans blocks for anomalies (hash mismatch, parent errors, etc.).
 * Returns number of anomalies found, or -1 on error.
 */
int  fossil_ai_jellyfish_audit(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Prune: invalidates blocks below min_confidence or expired.
 * Returns count pruned.
 */
int  fossil_ai_jellyfish_prune(fossil_ai_jellyfish_chain_t *chain, float min_confidence);

/* ------------------------------ Reasoning ---------------------------------- */

/**
 * Reason: retrieves best output for input (token similarity, confidence, freshness).
 * Returns output string or "Unknown".
 */
const char *fossil_ai_jellyfish_reason(fossil_ai_jellyfish_chain_t *chain, const char *input);

/**
 * Verbose reasoning: provides output, confidence score, and pointer to matched block.
 * Returns true if match found.
 */
bool fossil_ai_jellyfish_reason_verbose(const fossil_ai_jellyfish_chain_t *chain, const char *input,
                                     char *out_output, float *out_confidence,
                                     const fossil_ai_jellyfish_block_t **out_block);

/**
 * Best approximate match (token overlap, confidence, freshness).
 * Returns pointer to best block or NULL.
 */
const fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_best_match(const fossil_ai_jellyfish_chain_t *chain,
                                                            const char *input);

/* ------------------------------- Diagnostics ------------------------------- */

/**
 * Dump chain to stdout (debug summary and details).
 */
void fossil_ai_jellyfish_dump(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Reflect: prints summary statistics (confidence, age, type histogram).
 */
void fossil_ai_jellyfish_reflect(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Validation report: prints per-block anomaly flags and summary.
 */
void fossil_ai_jellyfish_validation_report(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Verify entire chain (hash, parent, type, and branch integrity).
 * Returns true if valid.
 */
bool fossil_ai_jellyfish_verify_chain(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Verify single block (hash, parent, type, signature, length).
 * Returns true if valid.
 */
bool fossil_ai_jellyfish_verify_block(const fossil_ai_jellyfish_block_t *block);

/**
 * Chain trust aggregate (weighted mean over valid blocks).
 * Returns normalized trust score [0.0, 1.0].
 */
float fossil_ai_jellyfish_chain_trust_score(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Compute repository fingerprint (aggregate hash over valid blocks).
 * Writes FOSSIL_JELLYFISH_HASH_SIZE bytes to out_hash.
 */
void fossil_ai_jellyfish_chain_fingerprint(const fossil_ai_jellyfish_chain_t *chain, uint8_t *out_hash);

/**
 * Collect stats arrays (valid counts, average confidence, immutable ratio) by commit group.
 */
void fossil_ai_jellyfish_chain_stats(const fossil_ai_jellyfish_chain_t *chain,
                                  size_t out_valid_count[5],
                                  float  out_avg_confidence[5],
                                  float  out_immutable_ratio[5]);

/**
 * Compare two chains (block-by-block hash and semantic delta).
 * Returns differing block count, or -1 on error.
 */
int fossil_ai_jellyfish_compare_chains(const fossil_ai_jellyfish_chain_t *a,
                                    const fossil_ai_jellyfish_chain_t *b);

/**
 * Compute block age (microseconds since timestamp).
 */
uint64_t fossil_ai_jellyfish_block_age(const fossil_ai_jellyfish_block_t *block, uint64_t now);

/**
 * Produce concise diagnostic string for a block.
 */
void fossil_ai_jellyfish_block_explain(const fossil_ai_jellyfish_block_t *block, char *out, size_t size);

/* ----------------------------- Optimization -------------------------------- */

/**
 * Decay confidence for all blocks: confidence *= (1 - decay_rate).
 * Respects immutability; updates confidence history.
 */
void fossil_ai_jellyfish_decay_confidence(fossil_ai_jellyfish_chain_t *chain, float decay_rate);

/**
 * Trim chain to max_blocks by confidence, recency, and type.
 * Returns number of blocks pruned.
 */
int  fossil_ai_jellyfish_trim(fossil_ai_jellyfish_chain_t *chain, size_t max_blocks);

/**
 * Compact: physically packs valid blocks forward, zeroes trailing slots.
 * Returns number of blocks moved.
 */
int  fossil_ai_jellyfish_chain_compact(fossil_ai_jellyfish_chain_t *chain);

/**
 * Deduplicate identical input/output pairs (retains higher-confidence/earlier).
 * Returns number of blocks deduplicated.
 */
int  fossil_ai_jellyfish_deduplicate_chain(fossil_ai_jellyfish_chain_t *chain);

/**
 * Compress chain: normalizes whitespace, retokenizes, marks compressed.
 * Returns number of blocks changed.
 */
int  fossil_ai_jellyfish_compress_chain(fossil_ai_jellyfish_chain_t *chain);

/* ------------------------------- Hash / Search ------------------------------ */

/**
 * Select highest-confidence valid block.
 * Returns pointer to block or NULL.
 */
const fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_best_memory(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Knowledge coverage: normalized density of active knowledge.
 * Returns [0.0, 1.0].
 */
float fossil_ai_jellyfish_knowledge_coverage(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Detect conflict (same input, differing output).
 * Returns count of conflicts, or -1 on error.
 */
int   fossil_ai_jellyfish_detect_conflict(const fossil_ai_jellyfish_chain_t *chain,
                                       const char *input, const char *output);

/**
 * Hash lookup (read-only, valid blocks only).
 * Returns pointer to block or NULL.
 */
const fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_find_by_hash(const fossil_ai_jellyfish_chain_t *chain,
                                                              const uint8_t *hash);

/**
 * Direct index accessor (valid type only).
 * Returns pointer to block or NULL.
 */
fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_get(fossil_ai_jellyfish_chain_t *chain, size_t index);

/* --------------------------- Block Attribute Ops --------------------------- */

/**
 * Mark block immutable (locks future mutation, may promote type/trust).
 */
void fossil_ai_jellyfish_mark_immutable(fossil_ai_jellyfish_block_t *block);

/**
 * Redact sensitive IO fields (emails, UUIDs, hex, digits).
 * Retokenizes and rehashes. Returns number of redactions.
 */
int  fossil_ai_jellyfish_redact_block(fossil_ai_jellyfish_block_t *block);

/**
 * Set commit message (bounded copy, confidence/type heuristics).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int  fossil_ai_jellyfish_block_set_message(fossil_ai_jellyfish_block_t *block, const char *message);

/**
 * Change block type (promotes/demotes, applies heuristics).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int  fossil_ai_jellyfish_block_set_type(fossil_ai_jellyfish_block_t *block,
                                     fossil_ai_jellyfish_commit_type_t type);

/* --------------------------- Classification Helpers ------------------------ */

/**
 * Append tag to block classification (lowercase, no whitespace).
 * Returns 1 if added, 0 if duplicate, negative on error.
 */
int fossil_ai_jellyfish_block_add_tag(fossil_ai_jellyfish_block_t *block, const char *tag);

/**
 * Set human-readable classification reason (trims/collapses whitespace).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int fossil_ai_jellyfish_block_set_reason(fossil_ai_jellyfish_block_t *block, const char *reason);

/**
 * Update similarity score (flags hallucinated/contradicted).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int fossil_ai_jellyfish_block_set_similarity(fossil_ai_jellyfish_block_t *block, float similarity);

/**
 * Link forward reference (derivation chain).
 * Returns 1 if added, 0 if duplicate, negative on error.
 */
int fossil_ai_jellyfish_block_link_forward(fossil_ai_jellyfish_block_t *block, uint32_t target_index);

/**
 * Link cross reference (semantic lateral link).
 * Returns 1 if added, 0 if duplicate, negative on error.
 */
int fossil_ai_jellyfish_block_link_cross(fossil_ai_jellyfish_block_t *block, uint32_t target_index);

/* ------------------------------ Git-Chain Ops ------------------------------ */

/**
 * Add a generic commit with explicit parents and type.
 * Computes hash with parent mixing and sets commit message.
 * Returns pointer to new block or NULL.
 */
fossil_ai_jellyfish_block_t *fossil_ai_jellyfish_add_commit(
    fossil_ai_jellyfish_chain_t *chain,
    const char *input,
    const char *output,
    fossil_ai_jellyfish_commit_type_t type,
    const uint8_t parent_hashes[][FOSSIL_JELLYFISH_HASH_SIZE],
    size_t parent_count,
    const char *message);

/**
 * Set parent hashes on an existing block (validates, recomputes hash).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int fossil_ai_jellyfish_commit_set_parents(fossil_ai_jellyfish_block_t *block,
                                        const uint8_t parent_hashes[][FOSSIL_JELLYFISH_HASH_SIZE],
                                        size_t parent_count);

/**
 * Return human-readable commit type string.
 */
const char *fossil_ai_jellyfish_commit_type_name(fossil_ai_jellyfish_commit_type_t type);

/* Branch management */

/**
 * Create branch if capacity available (validates name).
 * Returns branch index or negative on error.
 */
int fossil_ai_jellyfish_branch_create(fossil_ai_jellyfish_chain_t *chain, const char *name);

/**
 * Checkout branch (updates active branch name).
 * Returns branch index or negative on error.
 */
int fossil_ai_jellyfish_branch_checkout(fossil_ai_jellyfish_chain_t *chain, const char *name);

/**
 * Find branch index by name.
 * Returns index or negative if not found.
 */
int fossil_ai_jellyfish_branch_find(const fossil_ai_jellyfish_chain_t *chain, const char *name);

/**
 * Retrieve active branch name (default branch).
 * Returns pointer to name or NULL.
 */
const char *fossil_ai_jellyfish_branch_active(const fossil_ai_jellyfish_chain_t *chain);

/**
 * Update head hash of active branch.
 * Returns branch index or negative on error.
 */
int fossil_ai_jellyfish_branch_head_update(fossil_ai_jellyfish_chain_t *chain, const uint8_t *new_head_hash);

/* Merge / rebase / cherry-pick */

/**
 * Merge source into target, creates MERGE commit with dual parents.
 * Returns commit index or negative on error.
 */
int fossil_ai_jellyfish_merge(fossil_ai_jellyfish_chain_t *chain,
                           const char *source_branch,
                           const char *target_branch,
                           const char *message);

/**
 * Rebase branch onto another (creates REBASE commit).
 * Returns commit index or negative on error.
 */
int fossil_ai_jellyfish_rebase(fossil_ai_jellyfish_chain_t *chain,
                            const char *branch,
                            const char *onto_branch);

/**
 * Cherry-pick single commit by hash onto current branch.
 * Returns commit index or negative on error.
 */
int fossil_ai_jellyfish_cherry_pick(fossil_ai_jellyfish_chain_t *chain, const uint8_t *commit_hash);

/* Tagging */

/**
 * Add tag to block (delegates to classification tags).
 * Returns 1 if added, 0 if duplicate, negative on error.
 */
int fossil_ai_jellyfish_tag_block(fossil_ai_jellyfish_block_t *block, const char *tag);

/* ---------------------------- Cryptographic Ops --------------------------- */

/**
 * Sign block (deterministic hash-based signature, not cryptographic).
 * Returns 1 if changed, 0 if unchanged, negative on error.
 */
int  fossil_ai_jellyfish_block_sign(fossil_ai_jellyfish_block_t *block, const uint8_t *priv_key);

/**
 * Verify signature (deterministic hash-based, not cryptographic).
 * Returns true if valid.
 */
bool fossil_ai_jellyfish_block_verify_signature(const fossil_ai_jellyfish_block_t *block,
                                             const uint8_t *pub_key);

/* ------------------------------ Tokenization ------------------------------- */

/**
 * Tokenize input into lowercase alphanumeric tokens.
 * Returns token count.
 */
size_t fossil_ai_jellyfish_tokenize(const char *input,
                                 char (*tokens)[FOSSIL_JELLYFISH_TOKEN_SIZE],
                                 size_t max_tokens);

/* ------------------------------- Cloning ----------------------------------- */

/**
 * Deep clone chain (including FSON deep copies).
 * Returns number of valid blocks cloned, negative on error.
 */
int fossil_ai_jellyfish_clone_chain(const fossil_ai_jellyfish_chain_t *src,
                                 fossil_ai_jellyfish_chain_t *dst);

#ifdef __cplusplus
}
#include <stdexcept>
#include <cstdint>
#include <vector>
#include <array>
#include <string>

namespace fossil {

    namespace ai {

        /**
        * @class Jellyfish
        * @brief C++ RAII wrapper for the C Jellyfish Chain API.
        *
        * - Owns a fossil_ai_jellyfish_chain_t instance.
        * - Provides type-safe, exception-free thin forwarding methods.
        * - Utility helpers for parent vector marshalling and FSON RAII.
        *
        * Lifetime:
        * - Default constructor initializes the chain.
        * - Copy/move constructors and assignment perform deep/shallow clone as appropriate.
        *
        * Thread-safety: Not internally synchronized; external synchronization required.
        */
        class Jellyfish {
        public:
            /**
            * @brief Construct and initialize an empty chain.
            */
            Jellyfish() { ::fossil_ai_jellyfish_init(&chain_); }

            /**
            * @brief Destructor (no explicit teardown needed).
            */
            ~Jellyfish() { /* optional: cleanup if implementation allocs */ }

            /**
            * @brief Deep copy from another Jellyfish (clones chain & FSON).
            */
            Jellyfish(const Jellyfish& other) { ::fossil_ai_jellyfish_clone_chain(&other.chain_, &chain_); }

            /**
            * @brief Copy-assign via deep clone. Self-safe.
            */
            Jellyfish& operator=(const Jellyfish& other) {
                if (this != &other) {
                    ::fossil_ai_jellyfish_clone_chain(&other.chain_, &chain_);
                }
                return *this;
            }

            /**
            * @brief Move construct (bitwise move of POD chain_).
            */
            Jellyfish(Jellyfish&& other) noexcept { chain_ = other.chain_; }

            /**
            * @brief Move assign (bitwise move of POD chain_). Self-safe.
            */
            Jellyfish& operator=(Jellyfish&& other) noexcept {
                if (this != &other) chain_ = other.chain_;
                return *this;
            }

            // ---- Core API ----

            /**
            * @brief Static: compute deterministic hash for input/output pair.
            */
            static void hash(const char* input, const char* output, uint8_t* hash_out) {
                ::fossil_ai_jellyfish_hash(input, output, hash_out);
            }

            /**
            * @brief Reinitialize the internal chain (destructive reset).
            */
            void init() { ::fossil_ai_jellyfish_init(&chain_); }

            /**
            * @brief Append a learning (INFER) commit with given IO pair.
            */
            void learn(const char* input, const char* output) {
                ::fossil_ai_jellyfish_learn(&chain_, input, output);
            }

            /**
            * @brief Logically remove (invalidate) block by index.
            */
            void remove(size_t index) { ::fossil_ai_jellyfish_remove(&chain_, index); }

            /**
            * @brief Find mutable block by hash.
            */
            fossil_ai_jellyfish_block_t* find(const uint8_t* hash) {
                return ::fossil_ai_jellyfish_find(&chain_, hash);
            }

            /**
            * @brief Get mutable block by index.
            */
            fossil_ai_jellyfish_block_t* get(size_t index) {
                return ::fossil_ai_jellyfish_get(&chain_, index);
            }

            /**
            * @brief Update block IO and rehash.
            */
            void update(size_t index, const char* input, const char* output) {
                ::fossil_ai_jellyfish_update(&chain_, index, input, output);
            }

            /**
            * @brief Persist chain to binary file.
            */
            int save(const char* filepath) const { return ::fossil_ai_jellyfish_save(&chain_, filepath); }

            /**
            * @brief Load chain from binary file.
            */
            int load(const char* filepath) { return ::fossil_ai_jellyfish_load(&chain_, filepath); }

            // ---- Maintenance / metrics ----

            /**
            * @brief Perform hygiene cleanup pass.
            */
            void cleanup() { ::fossil_ai_jellyfish_cleanup(&chain_); }

            /**
            * @brief Run audit returning anomaly count.
            */
            int audit() const { return ::fossil_ai_jellyfish_audit(&chain_); }

            /**
            * @brief Prune low-confidence / expired blocks.
            */
            int prune(float min_confidence) { return ::fossil_ai_jellyfish_prune(&chain_, min_confidence); }

            // ---- Reasoning ----

            /**
            * @brief Retrieve best output for input.
            */
            const char* reason(const char* input) { return ::fossil_ai_jellyfish_reason(&chain_, input); }

            /**
            * @brief Verbose reasoning (output + confidence + block reference).
            */
            bool reason_verbose(const char* input, char* out_output, float* out_confidence,
                                const fossil_ai_jellyfish_block_t** out_block) const {
                return ::fossil_ai_jellyfish_reason_verbose(&chain_, input, out_output, out_confidence, out_block);
            }

            /**
            * @brief Get pointer to best approximate matching block.
            */
            const fossil_ai_jellyfish_block_t* best_match(const char* input) const {
                return ::fossil_ai_jellyfish_best_match(&chain_, input);
            }

            // ---- Diagnostics ----

            /**
            * @brief Debug dump to stdout.
            */
            void dump() const { ::fossil_ai_jellyfish_dump(&chain_); }

            /**
            * @brief Print reflective summary stats.
            */
            void reflect() const { ::fossil_ai_jellyfish_reflect(&chain_); }

            /**
            * @brief Emit validation report to stdout.
            */
            void validation_report() const { ::fossil_ai_jellyfish_validation_report(&chain_); }

            /**
            * @brief Verify chain integrity.
            */
            bool verify_chain() const { return ::fossil_ai_jellyfish_verify_chain(&chain_); }

            /**
            * @brief Verify single block integrity.
            */
            static bool verify_block(const fossil_ai_jellyfish_block_t* block) {
                return ::fossil_ai_jellyfish_verify_block(block);
            }

            /**
            * @brief Aggregate trust metric for chain.
            */
            float chain_trust_score() const { return ::fossil_ai_jellyfish_chain_trust_score(&chain_); }

            /**
            * @brief Compute repository-wide fingerprint hash.
            */
            void chain_fingerprint(uint8_t* out_hash) const {
                ::fossil_ai_jellyfish_chain_fingerprint(&chain_, out_hash);
            }

            /**
            * @brief Collect categorized stats arrays.
            */
            void chain_stats(size_t out_valid_count[5], float out_avg_confidence[5],
                            float out_immutable_ratio[5]) const {
                ::fossil_ai_jellyfish_chain_stats(&chain_, out_valid_count, out_avg_confidence, out_immutable_ratio);
            }

            /**
            * @brief Compare with another chain instance.
            */
            int compare_chains(const Jellyfish& other) const {
                return ::fossil_ai_jellyfish_compare_chains(&chain_, &other.chain_);
            }

            // ---- Optimization ----

            /**
            * @brief Apply exponential decay to all confidences.
            */
            void decay_confidence(float decay_rate) {
                ::fossil_ai_jellyfish_decay_confidence(&chain_, decay_rate);
            }

            /**
            * @brief Trim to at most max_blocks.
            */
            int trim(size_t max_blocks) { return ::fossil_ai_jellyfish_trim(&chain_, max_blocks); }

            /**
            * @brief Compact invalidated gaps.
            */
            int chain_compact() { return ::fossil_ai_jellyfish_chain_compact(&chain_); }

            /**
            * @brief Deduplicate equivalent IO pairs.
            */
            int deduplicate_chain() { return ::fossil_ai_jellyfish_deduplicate_chain(&chain_); }

            /**
            * @brief Compress (lightweight text normalization).
            */
            int compress_chain() { return ::fossil_ai_jellyfish_compress_chain(&chain_); }

            // ---- Hash / search helpers ----

            /**
            * @brief Select highest-confidence valid block.
            */
            const fossil_ai_jellyfish_block_t* best_memory() const {
                return ::fossil_ai_jellyfish_best_memory(&chain_);
            }

            /**
            * @brief Approximate knowledge coverage metric.
            */
            float knowledge_coverage() const {
                return ::fossil_ai_jellyfish_knowledge_coverage(&chain_);
            }

            /**
            * @brief Detect conflict (same input, differing output).
            */
            int detect_conflict(const char* input, const char* output) const {
                return ::fossil_ai_jellyfish_detect_conflict(&chain_, input, output);
            }

            /**
            * @brief Read-only hash lookup.
            */
            const fossil_ai_jellyfish_block_t* find_by_hash(const uint8_t* hash) const {
                return ::fossil_ai_jellyfish_find_by_hash(&chain_, hash);
            }

            // ---- Block attribute / classification ops ----

            /**
            * @brief Mark block immutable.
            */
            static void mark_immutable(fossil_ai_jellyfish_block_t* block) {
                ::fossil_ai_jellyfish_mark_immutable(block);
            }

            /**
            * @brief Redact sensitive IO fields.
            */
            static int redact_block(fossil_ai_jellyfish_block_t* block) {
                return ::fossil_ai_jellyfish_redact_block(block);
            }

            /**
            * @brief Set commit message text.
            */
            static int block_set_message(fossil_ai_jellyfish_block_t* block, const char* msg) {
                return ::fossil_ai_jellyfish_block_set_message(block, msg);
            }

            /**
            * @brief Change block commit type.
            */
            static int block_set_type(fossil_ai_jellyfish_block_t* block, fossil_ai_jellyfish_commit_type_t type) {
                return ::fossil_ai_jellyfish_block_set_type(block, type);
            }

            /**
            * @brief Append tag to block classification.
            */
            static int block_add_tag(fossil_ai_jellyfish_block_t* block, const char* tag) {
                return ::fossil_ai_jellyfish_block_add_tag(block, tag);
            }

            /**
            * @brief Set classification reason string.
            */
            static int block_set_reason(fossil_ai_jellyfish_block_t* block, const char* reason) {
                return ::fossil_ai_jellyfish_block_set_reason(block, reason);
            }

            /**
            * @brief Update similarity score.
            */
            static int block_set_similarity(fossil_ai_jellyfish_block_t* block, float sim) {
                return ::fossil_ai_jellyfish_block_set_similarity(block, sim);
            }

            /**
            * @brief Link forward reference index.
            */
            static int block_link_forward(fossil_ai_jellyfish_block_t* block, uint32_t idx) {
                return ::fossil_ai_jellyfish_block_link_forward(block, idx);
            }

            /**
            * @brief Link cross reference index.
            */
            static int block_link_cross(fossil_ai_jellyfish_block_t* block, uint32_t idx) {
                return ::fossil_ai_jellyfish_block_link_cross(block, idx);
            }

            /**
            * @brief Add tag alias (delegates to classification).
            */
            static int tag_block(fossil_ai_jellyfish_block_t* block, const char* tag) {
                return ::fossil_ai_jellyfish_tag_block(block, tag);
            }

            // ---- Git-chain style commit ops ----

            /**
            * @brief Add generic commit with explicit parents.
            * @param parents vector (max 4 used).
            */
            fossil_ai_jellyfish_block_t* add_commit(const char* input,
                                                    const char* output,
                                                    fossil_ai_jellyfish_commit_type_t type,
                                                    const std::vector<std::array<uint8_t, FOSSIL_JELLYFISH_HASH_SIZE>>& parents,
                                                    const char* message) {
                uint8_t parent_buf[4][FOSSIL_JELLYFISH_HASH_SIZE] = {{0}};
                size_t pc = parents.size() > 4 ? 4 : parents.size();
                for (size_t i = 0; i < pc; ++i)
                    memcpy(parent_buf[i], parents[i].data(), FOSSIL_JELLYFISH_HASH_SIZE);
                return ::fossil_ai_jellyfish_add_commit(&chain_, input, output, type, parent_buf, pc, message);
            }

            /**
            * @brief Set parents on an existing block.
            */
            static int commit_set_parents(fossil_ai_jellyfish_block_t* block,
                                        const std::vector<std::array<uint8_t, FOSSIL_JELLYFISH_HASH_SIZE>>& parents) {
                uint8_t parent_buf[4][FOSSIL_JELLYFISH_HASH_SIZE] = {{0}};
                size_t pc = parents.size() > 4 ? 4 : parents.size();
                for (size_t i = 0; i < pc; ++i)
                    memcpy(parent_buf[i], parents[i].data(), FOSSIL_JELLYFISH_HASH_SIZE);
                return ::fossil_ai_jellyfish_commit_set_parents(block, parent_buf, pc);
            }

            /**
            * @brief Human-readable commit type name.
            */
            static const char* commit_type_name(fossil_ai_jellyfish_commit_type_t type) {
                return ::fossil_ai_jellyfish_commit_type_name(type);
            }

            // ---- Branch management ----

            /**
            * @brief Create new branch.
            */
            int branch_create(const char* name) { return ::fossil_ai_jellyfish_branch_create(&chain_, name); }

            /**
            * @brief Checkout existing branch.
            */
            int branch_checkout(const char* name) { return ::fossil_ai_jellyfish_branch_checkout(&chain_, name); }

            /**
            * @brief Find branch index by name.
            */
            int branch_find(const char* name) const { return ::fossil_ai_jellyfish_branch_find(&chain_, name); }

            /**
            * @brief Active branch name.
            */
            const char* branch_active() const { return ::fossil_ai_jellyfish_branch_active(&chain_); }

            /**
            * @brief Update HEAD hash of active branch.
            */
            int branch_head_update(const uint8_t* new_head) {
                return ::fossil_ai_jellyfish_branch_head_update(&chain_, new_head);
            }

            // ---- Merge / rebase / cherry-pick ----

            /**
            * @brief Merge source branch into target branch.
            */
            int merge(const char* source_branch, const char* target_branch, const char* message) {
                return ::fossil_ai_jellyfish_merge(&chain_, source_branch, target_branch, message);
            }

            /**
            * @brief Rebase branch onto another.
            */
            int rebase(const char* branch, const char* onto) {
                return ::fossil_ai_jellyfish_rebase(&chain_, branch, onto);
            }

            /**
            * @brief Cherry-pick commit onto current branch.
            */
            int cherry_pick(const uint8_t* commit_hash) {
                return ::fossil_ai_jellyfish_cherry_pick(&chain_, commit_hash);
            }

    } // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
