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

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
   Jellyfish Core Types
   ============================================================ */

typedef struct fossil_ai_jellyfish_core fossil_ai_jellyfish_core_t;
typedef struct fossil_ai_jellyfish_model fossil_ai_jellyfish_model_t;
typedef struct fossil_ai_jellyfish_context fossil_ai_jellyfish_context_t;
typedef struct fossil_ai_jellyfish_audit fossil_ai_jellyfish_audit_t;

/* All IDs are UTF-8 strings, caller-owned */
typedef const char* fossil_ai_jellyfish_id_t;

/* Binary blobs for multimodal payloads */
typedef struct {
    const void* data;
    unsigned long size;
    const char* media_type; /* e.g. "text/plain", "image/png", "audio/wav" */
} fossil_ai_jellyfish_blob_t;

/* Hash structure (algorithm-agnostic) */
typedef struct {
    const char* algorithm; /* e.g. "sha256" */
    const unsigned char* bytes;
    unsigned long length;
} fossil_ai_jellyfish_hash_t;

/* ============================================================
   Core Lifecycle
   ============================================================ */

fossil_ai_jellyfish_core_t*
fossil_ai_jellyfish_create(const fossil_ai_jellyfish_id_t core_id);

void
fossil_ai_jellyfish_destroy(fossil_ai_jellyfish_core_t* core);

/* ============================================================
   Model Management
   ============================================================ */

fossil_ai_jellyfish_model_t*
fossil_ai_jellyfish_model_create(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_id_t model_id,
    fossil_ai_jellyfish_id_t model_type /* e.g. "llm", "vision", "audio" */
);

void
fossil_ai_jellyfish_model_destroy(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model
);

/* ============================================================
   Persistence
   ============================================================ */

int
fossil_ai_jellyfish_save(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    const char* path
);

fossil_ai_jellyfish_model_t*
fossil_ai_jellyfish_load(
    fossil_ai_jellyfish_core_t* core,
    const char* path
);

/* ============================================================
   Context Handling (Explicit, Immutable)
   ============================================================ */

fossil_ai_jellyfish_context_t*
fossil_ai_jellyfish_context_create(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_id_t context_id
);

int
fossil_ai_jellyfish_context_add(
    fossil_ai_jellyfish_context_t* context,
    fossil_ai_jellyfish_blob_t blob
);

fossil_ai_jellyfish_hash_t
fossil_ai_jellyfish_context_hash(
    fossil_ai_jellyfish_context_t* context
);

void
fossil_ai_jellyfish_context_destroy(
    fossil_ai_jellyfish_context_t* context
);

/* ============================================================
   Training Control Plane
   ============================================================ */

int
fossil_ai_jellyfish_train(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_id_t dataset_id
);

int
fossil_ai_jellyfish_retrain(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_id_t dataset_id
);

int
fossil_ai_jellyfish_untrain(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_id_t dataset_id
);

int
fossil_ai_jellyfish_erase(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_id_t dataset_id
);

/* ============================================================
   Inference
   ============================================================ */

int
fossil_ai_jellyfish_infer(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_context_t* context,
    fossil_ai_jellyfish_blob_t* output
);

/* Auto-detect integrity violations, drift, poisoning */
int
fossil_ai_jellyfish_auto_detect(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model
);

/* ============================================================
   Query & Summarization
   ============================================================ */

int
fossil_ai_jellyfish_ask(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_context_t* context,
    const char* question,
    fossil_ai_jellyfish_blob_t* answer
);

int
fossil_ai_jellyfish_summary(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_model_t* model,
    fossil_ai_jellyfish_context_t* context,
    fossil_ai_jellyfish_blob_t* summary
);

/* ============================================================
   Audit & Provenance
   ============================================================ */

fossil_ai_jellyfish_audit_t*
fossil_ai_jellyfish_audit(
    fossil_ai_jellyfish_core_t* core,
    fossil_ai_jellyfish_id_t target_id
);

fossil_ai_jellyfish_hash_t
fossil_ai_jellyfish_audit_hash(
    fossil_ai_jellyfish_audit_t* audit
);

void
fossil_ai_jellyfish_audit_destroy(
    fossil_ai_jellyfish_audit_t* audit
);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <stdexcept>

namespace fossil {

namespace ai {

class Jellyfish {
public:
    // ============================================================
    // Core Lifecycle
    // ============================================================
    explicit Jellyfish(const std::string& core_id) {
        core_ = fossil_ai_jellyfish_create(core_id.c_str());
        if (!core_) throw std::runtime_error("Failed to create Jellyfish core");
    }

    ~Jellyfish() {
        if (core_) fossil_ai_jellyfish_destroy(core_);
    }

    // Non-copyable
    Jellyfish(const Jellyfish&) = delete;
    Jellyfish& operator=(const Jellyfish&) = delete;

    // ============================================================
    // Model Management
    // ============================================================
    struct Model {
        fossil_ai_jellyfish_model_t* ptr = nullptr;
        std::string id;
        std::string type;

        ~Model() {
            if (ptr) ptr = nullptr; // Actual destroy handled by Jellyfish wrapper
        }
    };

    Model model_create(const std::string& model_id, const std::string& model_type) {
        Model m;
        m.ptr = fossil_ai_jellyfish_model_create(core_, model_id.c_str(), model_type.c_str());
        if (!m.ptr) throw std::runtime_error("Failed to create model");
        m.id = model_id;
        m.type = model_type;
        return m;
    }

    void model_destroy(Model& model) {
        if (model.ptr) {
            fossil_ai_jellyfish_model_destroy(core_, model.ptr);
            model.ptr = nullptr;
        }
    }

    // ============================================================
    // Persistence
    // ============================================================
    void save_model(Model& model, const std::string& path) {
        if (fossil_ai_jellyfish_save(core_, model.ptr, path.c_str()) != 0)
            throw std::runtime_error("Failed to save model");
    }

    Model load_model(const std::string& path) {
        Model m;
        m.ptr = fossil_ai_jellyfish_load(core_, path.c_str());
        if (!m.ptr) throw std::runtime_error("Failed to load model");
        return m;
    }

    // ============================================================
    // Context Handling
    // ============================================================
    struct Context {
        fossil_ai_jellyfish_context_t* ptr = nullptr;
        std::string id;

        ~Context() { if (ptr) ptr = nullptr; }
    };

    Context context_create(const std::string& context_id) {
        Context ctx;
        ctx.ptr = fossil_ai_jellyfish_context_create(core_, context_id.c_str());
        if (!ctx.ptr) throw std::runtime_error("Failed to create context");
        ctx.id = context_id;
        return ctx;
    }

    void context_add(Context& ctx, const void* data, size_t size, const std::string& media_type) {
        fossil_ai_jellyfish_blob_t blob{ data, static_cast<unsigned long>(size), media_type.c_str() };
        if (fossil_ai_jellyfish_context_add(ctx.ptr, blob) != 0)
            throw std::runtime_error("Failed to add blob to context");
    }

    std::vector<unsigned char> context_hash(const Context& ctx) {
        fossil_ai_jellyfish_hash_t h = fossil_ai_jellyfish_context_hash(ctx.ptr);
        return std::vector<unsigned char>(h.bytes, h.bytes + h.length);
    }

    void context_destroy(Context& ctx) {
        if (ctx.ptr) {
            fossil_ai_jellyfish_context_destroy(ctx.ptr);
            ctx.ptr = nullptr;
        }
    }

    // ============================================================
    // Training Control Plane
    // ============================================================
    void train(Model& model, const std::string& dataset_id) {
        if (fossil_ai_jellyfish_train(core_, model.ptr, dataset_id.c_str()) != 0)
            throw std::runtime_error("Training failed");
    }

    void retrain(Model& model, const std::string& dataset_id) {
        if (fossil_ai_jellyfish_retrain(core_, model.ptr, dataset_id.c_str()) != 0)
            throw std::runtime_error("Retraining failed");
    }

    void untrain(Model& model, const std::string& dataset_id) {
        if (fossil_ai_jellyfish_untrain(core_, model.ptr, dataset_id.c_str()) != 0)
            throw std::runtime_error("Untraining failed");
    }

    void erase(const std::string& dataset_id) {
        if (fossil_ai_jellyfish_erase(core_, dataset_id.c_str()) != 0)
            throw std::runtime_error("Erase failed");
    }

    // ============================================================
    // Inference
    // ============================================================
    std::vector<unsigned char> infer(Model& model, Context& ctx) {
        fossil_ai_jellyfish_blob_t output{};
        if (fossil_ai_jellyfish_infer(core_, model.ptr, ctx.ptr, &output) != 0)
            throw std::runtime_error("Inference failed");

        return std::vector<unsigned char>((unsigned char*)output.data, (unsigned char*)output.data + output.size);
    }

    void auto_detect(Model& model) {
        if (fossil_ai_jellyfish_auto_detect(core_, model.ptr) != 0)
            throw std::runtime_error("Auto-detect failed");
    }

    // ============================================================
    // Query & Summarization
    // ============================================================
    std::vector<unsigned char> ask(Model& model, Context& ctx, const std::string& question) {
        fossil_ai_jellyfish_blob_t answer{};
        if (fossil_ai_jellyfish_ask(core_, model.ptr, ctx.ptr, question.c_str(), &answer) != 0)
            throw std::runtime_error("Ask failed");

        return std::vector<unsigned char>((unsigned char*)answer.data, (unsigned char*)answer.data + answer.size);
    }

    std::vector<unsigned char> summary(Model& model, Context& ctx) {
        fossil_ai_jellyfish_blob_t summary{};
        if (fossil_ai_jellyfish_summary(core_, model.ptr, ctx.ptr, &summary) != 0)
            throw std::runtime_error("Summary failed");

        return std::vector<unsigned char>((unsigned char*)summary.data, (unsigned char*)summary.data + summary.size);
    }

    // ============================================================
    // Audit & Provenance
    // ============================================================
    struct Audit {
        fossil_ai_jellyfish_audit_t* ptr = nullptr;
        ~Audit() { if (ptr) ptr = nullptr; }
    };

    Audit audit(const std::string& target_id) {
        Audit a;
        a.ptr = fossil_ai_jellyfish_audit(core_, target_id.c_str());
        if (!a.ptr) throw std::runtime_error("Audit failed");
        return a;
    }

    std::vector<unsigned char> audit_hash(const Audit& a) {
        fossil_ai_jellyfish_hash_t h = fossil_ai_jellyfish_audit_hash(a.ptr);
        return std::vector<unsigned char>(h.bytes, h.bytes + h.length);
    }

    void audit_destroy(Audit& a) {
        if (a.ptr) {
            fossil_ai_jellyfish_audit_destroy(a.ptr);
            a.ptr = nullptr;
        }
    }

private:
    fossil_ai_jellyfish_core_t* core_ = nullptr;
};

} // namespace ai

} // namespace fossil

#endif

#endif /* fossil_fish_FRAMEWORK_H */
