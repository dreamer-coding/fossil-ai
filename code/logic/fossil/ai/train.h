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
#ifndef FOSSIL_AI_TRAIN_H
#define FOSSIL_AI_TRAIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int fossil_ai_train_begin(void* model);
int fossil_ai_train_step(void* model,const void* batch,size_t rows);
int fossil_ai_train_finalize(void* model);

int fossil_ai_train_dataset_attach(void* model,const void* data,size_t rows);
int fossil_ai_train_validate(void* model);

int fossil_ai_train_checkpoint(void* model,const char* path);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil::ai {

class Train {
public:
    static int begin(void* m){ return fossil_ai_train_begin(m); }
    static int step(void* m,const void* b,size_t r){ return fossil_ai_train_step(m,b,r); }
    static int finalize(void* m){ return fossil_ai_train_finalize(m); }

    static int dataset_attach(void* m,const void* d,size_t r){
        return fossil_ai_train_dataset_attach(m,d,r);
    }

    static int validate(void* m){ return fossil_ai_train_validate(m); }
    static int checkpoint(void* m,const char* p){ return fossil_ai_train_checkpoint(m,p); }
};

}
#endif

#endif
