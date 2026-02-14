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
#ifndef FOSSIL_AI_MODEL_H
#define FOSSIL_AI_MODEL_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int fossil_ai_model_create(const char* model_id, void** out);
int fossil_ai_model_destroy(void* model);

int fossil_ai_model_save(void* model, const char* path);
int fossil_ai_model_load(const char* path, void** out);

int fossil_ai_model_version(void* model, char* out, size_t n);
int fossil_ai_model_hash(void* model, char* out, size_t n);

int fossil_ai_model_metadata(void* model, void* out);
int fossil_ai_model_clone(void* model, void** out);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil::ai {

class Model {
public:
    static void* create(const char* id){
        void* m=nullptr; fossil_ai_model_create(id,&m); return m;
    }
    static int destroy(void* m){ return fossil_ai_model_destroy(m); }

    static int save(void* m,const char* p){ return fossil_ai_model_save(m,p); }
    static void* load(const char* p){
        void* m=nullptr; fossil_ai_model_load(p,&m); return m;
    }

    static int version(void* m,char* o,size_t n){ return fossil_ai_model_version(m,o,n); }
    static int hash(void* m,char* o,size_t n){ return fossil_ai_model_hash(m,o,n); }

    static int metadata(void* m,void* o){ return fossil_ai_model_metadata(m,o); }
    static void* clone(void* m){
        void* o=nullptr; fossil_ai_model_clone(m,&o); return o;
    }
};

}
#endif

#endif
