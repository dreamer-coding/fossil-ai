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
#ifndef FOSSIL_AI_AUDIT_H
#define FOSSIL_AI_AUDIT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int fossil_ai_audit_begin(void** ctx);
int fossil_ai_audit_record(void* ctx,const char* key,const void* data,size_t size);
int fossil_ai_audit_end(void* ctx);

int fossil_ai_audit_export(void* ctx,const char* path);
int fossil_ai_audit_verify(const char* path);

int fossil_ai_audit_diff(const char* a,const char* b,void* out);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil::ai {

class Audit {
public:
    static void* begin(){ void* c=nullptr; fossil_ai_audit_begin(&c); return c; }
    static int record(void* c,const char* k,const void* d,size_t s){
        return fossil_ai_audit_record(c,k,d,s);
    }
    static int end(void* c){ return fossil_ai_audit_end(c); }

    static int export_log(void* c,const char* p){ return fossil_ai_audit_export(c,p); }
    static int verify(const char* p){ return fossil_ai_audit_verify(p); }

    static int diff(const char* a,const char* b,void* o){
        return fossil_ai_audit_diff(a,b,o);
    }
};

}
#endif

#endif
