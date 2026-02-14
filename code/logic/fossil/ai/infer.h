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
#ifndef FOSSIL_AI_INFER_H
#define FOSSIL_AI_INFER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int fossil_ai_infer_predict(void* model,const void* X,size_t rows,size_t cols,void* out,const char* type);
int fossil_ai_infer_score(void* model,const void* X,size_t rows,size_t cols,double* out);
int fossil_ai_infer_rank(void* model,const void* X,size_t rows,size_t cols,size_t* order);

int fossil_ai_infer_reason_step(void* model,const void* input,void* output);
int fossil_ai_infer_explain(void* model,const void* input,void* explanation);

int fossil_ai_infer_batch(void* model,const void* X,size_t rows,size_t cols,void* out);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil::ai {

class Infer {
public:
    static int predict(void* m,const void* X,size_t r,size_t c,void* o,const char* t){
        return fossil_ai_infer_predict(m,X,r,c,o,t);
    }
    static int score(void* m,const void* X,size_t r,size_t c,double* o){
        return fossil_ai_infer_score(m,X,r,c,o);
    }
    static int rank(void* m,const void* X,size_t r,size_t c,size_t* o){
        return fossil_ai_infer_rank(m,X,r,c,o);
    }

    static int reason_step(void* m,const void* i,void* o){
        return fossil_ai_infer_reason_step(m,i,o);
    }
    static int explain(void* m,const void* i,void* e){
        return fossil_ai_infer_explain(m,i,e);
    }

    static int batch(void* m,const void* X,size_t r,size_t c,void* o){
        return fossil_ai_infer_batch(m,X,r,c,o);
    }
};

}
#endif

#endif
