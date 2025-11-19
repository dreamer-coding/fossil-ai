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

static char current_context_name[64] = {0};
static time_t session_start_time = 0;
static uint64_t session_id = 0;
static FILE *session_log_file = NULL;

// Helper: hex encode hash
static void hash_to_hex(const uint8_t *hash, char *hex, size_t hex_size) {
    static const char *digits = "0123456789abcdef";
    size_t need = FOSSIL_JELLYFISH_HASH_SIZE * 2 + 1;
    if (hex_size < need) {
        if (hex_size) hex[0] = 0;
        return;
    }
    for (size_t i = 0; i < FOSSIL_JELLYFISH_HASH_SIZE; ++i) {
        hex[i * 2]     = digits[(hash[i] >> 4) & 0xF];
        hex[i * 2 + 1] = digits[hash[i] & 0xF];
    }
    hex[FOSSIL_JELLYFISH_HASH_SIZE * 2] = 0;
}

// Helper: Format timestamp string
static void format_timestamp(time_t t, char *buf, size_t size) {
    struct tm *tm_info = localtime(&t);
    if (tm_info)
        strftime(buf, size, "%Y-%m-%d %H:%M:%S", tm_info);
    else if (size)
        buf[0] = 0;
}

// Helper: Open session log file for appending
static int open_session_log(uint64_t id) {
    char filename[128];
    snprintf(filename, sizeof(filename), "session_%llu.log", (unsigned long long)id);
    session_log_file = fopen(filename, "a");
    return session_log_file ? 0 : -1;
}

// Helper: Close session log file
static void close_session_log(void) {
    if (session_log_file) {
        fclose(session_log_file);
        session_log_file = NULL;
    }
}

// Logs a line to the session log (if open)
static void log_session_line(const char *line) {
    if (session_log_file) {
        fprintf(session_log_file, "%s\n", line);
        fflush(session_log_file);
    }
}

// Helper: log line with hash of input/output
static void log_hashed_event(const char *prefix, const char *input, const char *output) {
    if (!prefix) return;
    uint8_t hash[FOSSIL_JELLYFISH_HASH_SIZE];
    fossil_ai_jellyfish_hash(input ? input : "", output ? output : "", hash);
    char hex[FOSSIL_JELLYFISH_HASH_SIZE * 2 + 1];
    hash_to_hex(hash, hex, sizeof(hex));
    char line[512];
    snprintf(line, sizeof(line), "%s hash=%s in=\"%s\" out=\"%s\"",
             prefix, hex, input ? input : "", output ? output : "");
    log_session_line(line);
}

// Add a system block to the chain (immutable)
static void record_system_block(fossil_ai_jellyfish_chain_t *chain, const char *msg) {
    if (!chain || !msg) return;
    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) return;

    fossil_ai_jellyfish_learn(chain, "[system]", msg);

    if (chain->count > 0) {
        fossil_ai_jellyfish_block_t *b = fossil_ai_jellyfish_get(chain, chain->count - 1);
        fossil_ai_jellyfish_mark_immutable(b);
        fossil_ai_jellyfish_block_set_message(b, "system-context");
        fossil_ai_jellyfish_block_set_type(b, (chain->count == 1) ? JELLY_COMMIT_INIT : JELLY_COMMIT_TAG);
    }
    log_hashed_event("[system-block]", "[system]", msg);
}

// --- Slash Command Support ---

static int handle_slash_command(const char *input, char *output, size_t size, fossil_ai_jellyfish_chain_t *chain) {
    if (!input || input[0] != '/') return 0; // Not a slash command

    // /summary
    if (strncmp(input, "/summary", 8) == 0) {
        if (fossil_ai_iochat_summarize_session(chain, output, size) == 0) {
            return 1;
        } else {
            strncpy(output, "No summary available.", size - 1);
            output[size - 1] = '\0';
            return 1;
        }
    }
    // /turns
    if (strncmp(input, "/turns", 6) == 0) {
        int turns = fossil_ai_iochat_turn_count(chain);
        snprintf(output, size, "Turn count: %d", turns);
        return 1;
    }
    // /trust
    if (strncmp(input, "/trust", 6) == 0) {
        float trust = fossil_ai_jellyfish_chain_trust_score(chain);
        snprintf(output, size, "Chain trust score: %.3f", trust);
        return 1;
    }
    // /end
    if (strncmp(input, "/end", 4) == 0) {
        strncpy(output, "Session end requested.", size - 1);
        output[size - 1] = '\0';
        return 2; // Special code for session end
    }
    // /help
    if (strncmp(input, "/help", 5) == 0) {
        strncpy(output,
            "Slash commands:\n"
            "/summary - Summarize session\n"
            "/turns - Show turn count\n"
            "/trust - Show chain trust score\n"
            "/export <file> - Export session history\n"
            "/import <file> - Import context\n"
            "/end - End session\n"
            "/help - Show this help\n",
            size - 1);
        output[size - 1] = '\0';
        return 1;
    }
    // /export <filepath>
    if (strncmp(input, "/export ", 8) == 0) {
        const char *filepath = input + 8;
        if (strlen(filepath) > 0) {
            int res = fossil_ai_iochat_export_history(chain, filepath);
            if (res == 0) {
                snprintf(output, size, "Session history exported to: %s", filepath);
            } else {
                snprintf(output, size, "Failed to export session history to: %s", filepath);
            }
        } else {
            strncpy(output, "Usage: /export <filepath>", size - 1);
            output[size - 1] = '\0';
        }
        return 1;
    }
    // /import <filepath>
    if (strncmp(input, "/import ", 8) == 0) {
        const char *filepath = input + 8;
        if (strlen(filepath) > 0) {
            int res = fossil_ai_iochat_import_context(chain, filepath);
            if (res == 0) {
                snprintf(output, size, "Context imported from: %s", filepath);
            } else {
                snprintf(output, size, "Failed to import context from: %s", filepath);
            }
        } else {
            strncpy(output, "Usage: /import <filepath>", size - 1);
            output[size - 1] = '\0';
        }
        return 1;
    }
    // /inject <message>
    if (strncmp(input, "/inject ", 8) == 0) {
        const char *msg = input + 8;
        if (strlen(msg) > 0) {
            int res = fossil_ai_iochat_inject_system_message(chain, msg);
            if (res == 0) {
                snprintf(output, size, "Injected system message: %s", msg);
            } else {
                snprintf(output, size, "Failed to inject system message.");
            }
        } else {
            strncpy(output, "Usage: /inject <message>", size - 1);
            output[size - 1] = '\0';
        }
        return 1;
    }
    // /learn <input>|<output>
    if (strncmp(input, "/learn ", 7) == 0) {
        const char *args = input + 7;
        const char *sep = strchr(args, '|');
        if (sep && sep != args && *(sep + 1)) {
            char inbuf[128], outbuf[128];
            size_t inlen = (size_t)(sep - args);
            size_t outlen = strlen(sep + 1);
            if (inlen < sizeof(inbuf) && outlen < sizeof(outbuf)) {
                strncpy(inbuf, args, inlen);
                inbuf[inlen] = '\0';
                strncpy(outbuf, sep + 1, sizeof(outbuf) - 1);
                outbuf[sizeof(outbuf) - 1] = '\0';
                int res = fossil_ai_iochat_learn_response(chain, inbuf, outbuf);
                if (res == 0) {
                    snprintf(output, size, "Learned response for \"%s\"", inbuf);
                } else {
                    snprintf(output, size, "Failed to learn response.");
                }
            } else {
                strncpy(output, "Input/output too long.", size - 1);
                output[size - 1] = '\0';
            }
        } else {
            strncpy(output, "Usage: /learn <input>|<output>", size - 1);
            output[size - 1] = '\0';
        }
        return 1;
    }
    // Unknown command
    snprintf(output, size, "Unknown slash command: %s", input);
    return 1;
}

// --- API Functions ---

int fossil_ai_iochat_start(const char *context_name, fossil_ai_jellyfish_chain_t *chain) {
    if (context_name && strlen(context_name) < sizeof(current_context_name)) {
        strncpy(current_context_name, context_name, sizeof(current_context_name) - 1);
    } else {
        strncpy(current_context_name, "default", sizeof(current_context_name) - 1);
    }

    session_start_time = time(NULL);
    session_id = (uint64_t)session_start_time;

    if (open_session_log(session_id) != 0) {
        fprintf(stderr, "[fossil_ai_iochat] Warning: Could not open session log file.\n");
    }

    char ts[32];
    format_timestamp(session_start_time, ts, sizeof(ts));

    char log_line[256];
    snprintf(log_line, sizeof(log_line), "Session started: %s @ %s", current_context_name, ts);
    log_session_line(log_line);

    if (chain) {
        int anomalies = fossil_ai_jellyfish_audit(chain);
        char audit_line[128];
        snprintf(audit_line, sizeof(audit_line), "Initial chain audit anomalies=%d", anomalies);
        log_session_line(audit_line);

        char system_msg[200];
        snprintf(system_msg, sizeof(system_msg), "Session started with context \"%s\" at %s", current_context_name, ts);
        record_system_block(chain, system_msg);
    }

    return 0;
}

int fossil_ai_iochat_respond(fossil_ai_jellyfish_chain_t *chain, const char *input, char *output, size_t size) {
    if (!chain || !input || !output || size == 0) return -1;

    // Slash command support
    int slash_result = handle_slash_command(input, output, size, chain);
    if (slash_result == 1) {
        log_session_line(output);
        return 0;
    }
    if (slash_result == 2) {
        log_session_line(output);
        return 1; // Indicate session end requested
    }

    float confidence = 0.0f;
    const fossil_ai_jellyfish_block_t *matched_block = NULL;

    bool found = fossil_ai_jellyfish_reason_verbose(chain, input, output, &confidence, &matched_block);

    if (found && matched_block && confidence > 0.3f) {
        char log_line[256];
        snprintf(log_line, sizeof(log_line), "Input: \"%s\" → Output: \"%.*s\" (confidence: %.2f)",
                 input, (int)(size - 1), output, confidence);
        log_session_line(log_line);

        int conflict = fossil_ai_jellyfish_detect_conflict(chain, input, output);
        if (conflict == 0) {
            fossil_ai_jellyfish_learn(chain, input, output);
            log_hashed_event("[learn]", input, output);
        } else {
            char cbuf[160];
            snprintf(cbuf, sizeof(cbuf), "Conflict detected (skipped learn) input=\"%s\"", input);
            log_session_line(cbuf);
        }
        return 0;
    } else {
        const char *fallback = "I'm not sure how to respond to that yet.";
        strncpy(output, fallback, size - 1);
        output[size - 1] = '\0';

        char log_line[256];
        snprintf(log_line, sizeof(log_line), "Input: \"%s\" → Fallback response used", input);
        log_session_line(log_line);

        int conflict = fossil_ai_jellyfish_detect_conflict(chain, input, output);
        if (conflict == 0 && chain->count < FOSSIL_JELLYFISH_MAX_MEM) {
            fossil_ai_jellyfish_learn(chain, input, output);
            log_hashed_event("[learn-fallback]", input, output);
        }
        return -1;
    }
}

int fossil_ai_iochat_end(fossil_ai_jellyfish_chain_t *chain) {
    time_t now = time(NULL);
    double duration = difftime(now, session_start_time);

    char ts[32];
    format_timestamp(now, ts, sizeof(ts));

    char log_line[256];
    snprintf(log_line, sizeof(log_line), "Session \"%s\" ended after %.2f seconds @ %s", current_context_name, duration, ts);
    log_session_line(log_line);

    if (chain) {
        bool ok = fossil_ai_jellyfish_verify_chain(chain);
        float trust = fossil_ai_jellyfish_chain_trust_score(chain);
        char verify_line[160];
        snprintf(verify_line, sizeof(verify_line), "Final verify=%s trust=%.3f", ok ? "ok" : "FAIL", trust);
        log_session_line(verify_line);

        char system_msg[128];
        snprintf(system_msg, sizeof(system_msg), "Session ended after %.2f seconds at %s", duration, ts);
        record_system_block(chain, system_msg);
    }

    close_session_log();

    memset(current_context_name, 0, sizeof(current_context_name));
    session_start_time = 0;
    session_id = 0;

    return 0;
}

int fossil_ai_iochat_inject_system_message(fossil_ai_jellyfish_chain_t *chain, const char *message) {
    if (!chain || !message || strlen(message) == 0) {
        return -1;
    }

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fprintf(stderr, "[fossil_ai_iochat] Chain memory full, cannot inject system message.\n");
        return -1;
    }

    fossil_ai_jellyfish_learn(chain, "[system]", message);

    if (chain->count > 0) {
        fossil_ai_jellyfish_block_t *b = fossil_ai_jellyfish_get(chain, chain->count - 1);
        fossil_ai_jellyfish_mark_immutable(b);
        fossil_ai_jellyfish_block_set_message(b, "system-injected");
        fossil_ai_jellyfish_block_set_type(b, JELLY_COMMIT_PATCH);
    }

    log_hashed_event("[inject-system]", "[system]", message);
    printf("[fossil_ai_iochat] Injected system message: \"%s\"\n", message);

    return 0;
}

int fossil_ai_iochat_learn_response(fossil_ai_jellyfish_chain_t *chain, const char *input, const char *output) {
    if (!chain || !input || !output || strlen(input) == 0 || strlen(output) == 0) {
        return -1;
    }

    if (chain->count >= FOSSIL_JELLYFISH_MAX_MEM) {
        fprintf(stderr, "[fossil_ai_iochat] Chain memory full, cannot learn new response.\n");
        return -1;
    }

    int conflict = fossil_ai_jellyfish_detect_conflict(chain, input, output);
    if (conflict != 0) {
        fprintf(stderr, "[fossil_ai_iochat] Conflict detected for input \"%s\", learn skipped.\n", input);
        return -1;
    }

    fossil_ai_jellyfish_learn(chain, input, output);
    log_hashed_event("[manual-learn]", input, output);

    printf("[fossil_ai_iochat] Learned new response for input: \"%s\"\n", input);

    return 0;
}

int fossil_ai_iochat_turn_count(const fossil_ai_jellyfish_chain_t *chain) {
    if (!chain) return 0;

    int count = 0;
    for (size_t i = 0; i < chain->count; ++i) {
        const fossil_ai_jellyfish_block_t *b = &chain->commits[i];
        if (b->attributes.valid &&
            strncmp(b->io.input, "[system]", sizeof("[system]") - 1) != 0) {
            ++count;
        }
    }
    return count;
}

int fossil_ai_iochat_summarize_session(const fossil_ai_jellyfish_chain_t *chain, char *summary, size_t size) {
    if (!chain || !summary || size == 0) return -1;

    size_t pos = 0;
    for (size_t i = 0; i < chain->count && pos + 64 < size; ++i) {
        const fossil_ai_jellyfish_block_t *b = &chain->commits[i];
        if (!b->attributes.valid) continue;
        if (strncmp(b->io.input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        int written = snprintf(summary + pos, size - pos, "[%s] %s. ", b->io.input, b->io.output);
        if (written < 0) break;
        pos += (size_t)written;
    }

    return pos > 0 ? 0 : -1;
}

int fossil_ai_iochat_filter_recent(const fossil_ai_jellyfish_chain_t *chain, fossil_ai_jellyfish_chain_t *out_chain, int turn_count) {
    if (!chain || !out_chain || turn_count <= 0) return -1;

    fossil_ai_jellyfish_init(out_chain);
    int added = 0;

    for (int i = (int)chain->count - 1; i >= 0 && added < turn_count; --i) {
        const fossil_ai_jellyfish_block_t *b = &chain->commits[i];
        if (!b->attributes.valid) continue;
        if (strncmp(b->io.input, "[system]", FOSSIL_JELLYFISH_INPUT_SIZE) == 0) continue;

        out_chain->commits[turn_count - added - 1] = *b;
        out_chain->commits[turn_count - added - 1].attributes.valid = 1;
        added++;
    }

    out_chain->count = (size_t)added;
    return 0;
}

int fossil_ai_iochat_export_history(const fossil_ai_jellyfish_chain_t *chain, const char *filepath) {
    if (!chain || !filepath) return -1;

    // Use updated persistence API
    return fossil_ai_jellyfish_save(chain, filepath);
}

int fossil_ai_iochat_import_context(fossil_ai_jellyfish_chain_t *chain, const char *filepath) {
    if (!chain || !filepath) return -1;

    // Use updated persistence API
    return fossil_ai_jellyfish_load(chain, filepath);
}
