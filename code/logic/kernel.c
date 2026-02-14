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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

/* ---------------------------------------------------------
 * Internal Types
 * --------------------------------------------------------- */

typedef struct fossil_ai_task_s {
    void* data;
    struct fossil_ai_task_s* next;
} fossil_ai_task_t;

typedef struct fossil_ai_model_node_s {
    void* model;
    struct fossil_ai_model_node_s* next;
} fossil_ai_model_node_t;

typedef struct fossil_ai_kernel_ctx_s {
    fossil_ai_model_node_t* models;
    fossil_ai_task_t* task_head;
    fossil_ai_task_t* task_tail;
#if defined(_WIN32)
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} fossil_ai_kernel_ctx_t;

static fossil_ai_kernel_ctx_t g_kernel = {0};

/* ---------------------------------------------------------
 * Lock helpers
 * --------------------------------------------------------- */

static void kernel_lock() {
#if defined(_WIN32)
    EnterCriticalSection(&g_kernel.lock);
#else
    pthread_mutex_lock(&g_kernel.lock);
#endif
}

static void kernel_unlock() {
#if defined(_WIN32)
    LeaveCriticalSection(&g_kernel.lock);
#else
    pthread_mutex_unlock(&g_kernel.lock);
#endif
}

/* ---------------------------------------------------------
 * Kernel Init / Shutdown
 * --------------------------------------------------------- */

int fossil_ai_kernel_init(void) {
    memset(&g_kernel, 0, sizeof(g_kernel));
#if defined(_WIN32)
    InitializeCriticalSection(&g_kernel.lock);
#else
    if (pthread_mutex_init(&g_kernel.lock, NULL) != 0) return -1;
#endif
    return 0;
}

int fossil_ai_kernel_shutdown(void) {
    kernel_lock();

    /* Free all tasks */
    fossil_ai_task_t* t = g_kernel.task_head;
    while (t) {
        fossil_ai_task_t* next = t->next;
        free(t);
        t = next;
    }
    g_kernel.task_head = g_kernel.task_tail = NULL;

    /* Free all models */
    fossil_ai_model_node_t* m = g_kernel.models;
    while (m) {
        fossil_ai_model_node_t* next = m->next;
        free(m);
        m = next;
    }
    g_kernel.models = NULL;

    kernel_unlock();
#if defined(_WIN32)
    DeleteCriticalSection(&g_kernel.lock);
#else
    pthread_mutex_destroy(&g_kernel.lock);
#endif
    return 0;
}

/* ---------------------------------------------------------
 * Model Registration
 * --------------------------------------------------------- */

int fossil_ai_kernel_register_model(void* model) {
    if (!model) return -1;
    fossil_ai_model_node_t* node = (fossil_ai_model_node_t*)malloc(sizeof(fossil_ai_model_node_t));
    if (!node) return -2;
    node->model = model;
    node->next = NULL;

    kernel_lock();
    node->next = g_kernel.models;
    g_kernel.models = node;
    kernel_unlock();
    return 0;
}

int fossil_ai_kernel_unregister_model(void* model) {
    if (!model) return -1;
    kernel_lock();
    fossil_ai_model_node_t* prev = NULL;
    fossil_ai_model_node_t* curr = g_kernel.models;
    while (curr) {
        if (curr->model == model) {
            if (prev) prev->next = curr->next;
            else g_kernel.models = curr->next;
            free(curr);
            kernel_unlock();
            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    kernel_unlock();
    return -2; /* not found */
}

/* ---------------------------------------------------------
 * Task Queue
 * --------------------------------------------------------- */

int fossil_ai_kernel_run(void* task_data) {
    if (!task_data) return -1;
    fossil_ai_task_t* task = (fossil_ai_task_t*)malloc(sizeof(fossil_ai_task_t));
    if (!task) return -2;
    task->data = task_data;
    task->next = NULL;

    kernel_lock();
    if (g_kernel.task_tail) g_kernel.task_tail->next = task;
    else g_kernel.task_head = task;
    g_kernel.task_tail = task;
    kernel_unlock();
    return 0;
}

int fossil_ai_kernel_step(void) {
    kernel_lock();
    fossil_ai_task_t* task = g_kernel.task_head;
    if (!task) {
        kernel_unlock();
        return -1; /* no task */
    }
    g_kernel.task_head = task->next;
    if (!g_kernel.task_head) g_kernel.task_tail = NULL;
    kernel_unlock();

    /* Execute the task: user-defined pointer, cast as needed */
    // For now, assume task->data is a function pointer
    void (*fn)(void) = (void(*)(void))task->data;
    if (fn) fn();

    free(task);
    return 0;
}

/* ---------------------------------------------------------
 * Audit & Introspection
 * --------------------------------------------------------- */

int fossil_ai_kernel_audit_snapshot(void* out) {
    if (!out) return -1;
    kernel_lock();
    fossil_ai_model_node_t* m = g_kernel.models;
    size_t count = 0;
    while (m) { count++; m = m->next; }
    *((size_t*)out) = count;
    kernel_unlock();
    return 0;
}

int fossil_ai_kernel_introspect(void* out) {
    if (!out) return -1;
    kernel_lock();
    fossil_ai_task_t* t = g_kernel.task_head;
    size_t count = 0;
    while (t) { count++; t = t->next; }
    *((size_t*)out) = count;
    kernel_unlock();
    return 0;
}
