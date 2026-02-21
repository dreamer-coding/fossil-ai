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
#include "fossil/ai/kernal.h"

#include <stdlib.h>
#include <string.h>

/* =========================================================
 * Internal State
 * ========================================================= */

typedef struct fossil_ai_model_node {
    void* model;
    struct fossil_ai_model_node* next;
} fossil_ai_model_node_t;

static struct {
    int initialized;
    fossil_ai_model_node_t* models;
    size_t model_count;
    size_t steps_executed;
} g_kernel = {0};


/* =========================================================
 * Helpers
 * ========================================================= */

static fossil_ai_model_node_t* find_model(void* model, fossil_ai_model_node_t** prev)
{
    fossil_ai_model_node_t* p = g_kernel.models;
    fossil_ai_model_node_t* last = NULL;

    while (p) {
        if (p->model == model) {
            if (prev) *prev = last;
            return p;
        }
        last = p;
        p = p->next;
    }
    return NULL;
}


/* =========================================================
 * Lifecycle
 * ========================================================= */

int fossil_ai_kernel_init(void)
{
    if (g_kernel.initialized)
        return 0;

    memset(&g_kernel, 0, sizeof(g_kernel));
    g_kernel.initialized = 1;
    return 0;
}

int fossil_ai_kernel_shutdown(void)
{
    if (!g_kernel.initialized)
        return -1;

    fossil_ai_model_node_t* p = g_kernel.models;
    while (p) {
        fossil_ai_model_node_t* next = p->next;
        free(p);
        p = next;
    }

    memset(&g_kernel, 0, sizeof(g_kernel));
    return 0;
}


/* =========================================================
 * Model Management
 * ========================================================= */

int fossil_ai_kernel_register_model(void* model)
{
    if (!g_kernel.initialized || !model)
        return -1;

    if (find_model(model, NULL))
        return 1; /* already registered */

    fossil_ai_model_node_t* node =
        (fossil_ai_model_node_t*)malloc(sizeof(*node));
    if (!node)
        return -2;

    node->model = model;
    node->next = g_kernel.models;
    g_kernel.models = node;
    g_kernel.model_count++;

    return 0;
}

int fossil_ai_kernel_unregister_model(void* model)
{
    if (!g_kernel.initialized || !model)
        return -1;

    fossil_ai_model_node_t* prev = NULL;
    fossil_ai_model_node_t* node = find_model(model, &prev);
    if (!node)
        return 1;

    if (prev)
        prev->next = node->next;
    else
        g_kernel.models = node->next;

    free(node);
    g_kernel.model_count--;
    return 0;
}


/* =========================================================
 * Execution
 * ========================================================= */

int fossil_ai_kernel_run(void* task)
{
    if (!g_kernel.initialized)
        return -1;

    /* Placeholder execution logic
       Later you could dispatch this to schedulers,
       models, or pipelines */
    (void)task;

    /* For now just perform a step */
    return fossil_ai_kernel_step();
}

int fossil_ai_kernel_step(void)
{
    if (!g_kernel.initialized)
        return -1;

    /* In a real kernel:
       iterate models and update them */
    g_kernel.steps_executed++;
    return 0;
}


/* =========================================================
 * Audit & Introspection
 * ========================================================= */

typedef struct fossil_ai_kernel_snapshot {
    size_t model_count;
    size_t steps_executed;
    int initialized;
} fossil_ai_kernel_snapshot_t;

int fossil_ai_kernel_audit_snapshot(void* out)
{
    if (!g_kernel.initialized || !out)
        return -1;

    fossil_ai_kernel_snapshot_t* snap =
        (fossil_ai_kernel_snapshot_t*)out;

    snap->model_count = g_kernel.model_count;
    snap->steps_executed = g_kernel.steps_executed;
    snap->initialized = g_kernel.initialized;

    return 0;
}

int fossil_ai_kernel_introspect(void* out)
{
    /* For now this just mirrors snapshot.
       Later this could expose queues,
       scheduler info, memory stats, etc. */
    return fossil_ai_kernel_audit_snapshot(out);
}
