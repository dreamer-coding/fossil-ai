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
#ifndef FOSSIL_AI_CHAT_H
#define FOSSIL_AI_CHAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int fossil_ai_chat_session_open(void** out);
int fossil_ai_chat_session_close(void* s);

int fossil_ai_chat_send(void* s,const char* role,const char* msg);
int fossil_ai_chat_receive(void* s,char* out,size_t n);

int fossil_ai_chat_history_get(void* s,void* out);
int fossil_ai_chat_history_prune(void* s,size_t keep);

int fossil_ai_chat_render(void* s,char* out,size_t n);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil::ai {

class Chat {
public:
    static void* open(){ void* s=nullptr; fossil_ai_chat_session_open(&s); return s; }
    static int close(void* s){ return fossil_ai_chat_session_close(s); }

    static int send(void* s,const char* r,const char* m){ return fossil_ai_chat_send(s,r,m); }
    static int receive(void* s,char* o,size_t n){ return fossil_ai_chat_receive(s,o,n); }

    static int history_get(void* s,void* o){ return fossil_ai_chat_history_get(s,o); }
    static int history_prune(void* s,size_t k){ return fossil_ai_chat_history_prune(s,k); }

    static int render(void* s,char* o,size_t n){ return fossil_ai_chat_render(s,o,n); }
};

}
#endif

#endif
