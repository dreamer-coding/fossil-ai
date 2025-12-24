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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <assert.h>

/* ======================================================
 * Hash Helpers
 * ====================================================== */

static unsigned int hash_string(const char *s) {
    /* FNV-1a 32-bit */
    uint32_t h = 2166136261u;

    while (*s) {
        h ^= (unsigned char)(*s++);
        h *= 16777619u;
    }

    /* Final avalanche */
    h ^= h >> 16;
    h *= 0x7feb352d;
    h ^= h >> 15;
    h *= 0x846ca68b;
    h ^= h >> 16;

    return h % FOSSIL_AI_CHAT_HASH_SIZE;
}

static void init_hash(const char **terms, size_t term_count,
                      unsigned char table[FOSSIL_AI_CHAT_HASH_SIZE]) {
    for (size_t i = 0; i < term_count; i++)
        table[hash_string(terms[i])] = 1;
}

static bool hash_lookup(const char *word,
                        unsigned char table[FOSSIL_AI_CHAT_HASH_SIZE]) {
    return table[hash_string(word)] != 0;
}

/* ======================================================
 * Helpers
 * ====================================================== */

static bool phrase_match(const char *text, const char *phrase) {
    return strstr(text, phrase) != NULL;
}

static bool is_first_person(const char *w) {
    return !strcmp(w, "i") ||
           !strcmp(w, "me") ||
           !strcmp(w, "my") ||
           !strcmp(w, "mine") ||
}

/* ======================================================
 * Normalization & Tokenization
 * ====================================================== */

static size_t normalize_and_tokenize(
    const char *input,
    char tokens[FOSSIL_AI_CHAT_MAX_TOKENS][FOSSIL_AI_CHAT_MAX_TOKEN_LEN]) {

    char buf[1024];
    size_t len = 0;

    for (; *input && len < sizeof(buf) - 1; input++) {
        if (isalpha((unsigned char)*input) || isspace((unsigned char)*input))
            buf[len++] = (char)tolower(*input);
        else
            buf[len++] = ' ';
    }
    buf[len] = 0;

    size_t count = 0;
    char *p = buf;
    while (*p && count < FOSSIL_AI_CHAT_MAX_TOKENS) {
        while (*p == ' ') p++;
        if (!*p) break;

        size_t i = 0;
        while (*p && *p != ' ' && i < FOSSIL_AI_CHAT_MAX_TOKEN_LEN - 1)
            tokens[count][i++] = *p++;
        tokens[count++][i] = 0;
    }
    return count;
}

/* ======================================================
 * Vocabulary & Misspelling Tolerance
 * ====================================================== */

static const char *VOCAB[] = {
    /* Pronouns (minimal, non-social) */
    "i","me","you","we","they","it","this","that","these","those",

    /* Aux / verbs */
    "am","are","is","was","were","be","being","been",
    "can","could","will","would","shall","should",
    "do","does","did","doing",
    "have","has","had",
    "make","made","use","used","build","built","run","running",
    "apply","applied","execute","executed",
    "process","processed","compute","computed",
    "analyze","analyzed","measure","measured",
    "detect","detected","verify","verified",
    "compare","compared","select","selected",
    "store","stored","load","loaded",
    "train","trained","infer","inferred",

    /* Question / logic */
    "what","why","how","when","where","which","who",
    "if","then","else","while","for","each",
    "and","or","not","but","because",
    "true","false",

    /* Cognition (cold) */
    "think","know","understand","recognize","identify",
    "determine","estimate","evaluate","assess",
    "assume","assumed","expect","expected",

    /* Time */
    "time","date","now","today","yesterday","tomorrow",
    "before","after","during","earlier","later",
    "current","previous","next",

    /* Quantity / math */
    "one","two","three","many","few","all","none",
    "more","less","most","least",
    "equal","greater","lower",
    "increase","decrease",
    "rate","ratio","percent",

    /* State / condition */
    "state","status","mode","level","range","limit",
    "valid","invalid","allowed","denied",
    "enabled","disabled",
    "available","unavailable",

    /* System / software */
    "system","model","engine","module","library","component",
    "memory","context","state","cache","buffer",
    "input","output","parameter","option","setting",
    "configuration","default","override",
    "file","path","directory",
    "process","thread","task",
    "performance","latency","throughput",
    "error","warning","fault","failure",
    "log","trace","debug",
    "build","compile","link","run",

    /* Data / AI */
    "data","dataset","sample","feature","label",
    "vector","matrix","tensor",
    "training","testing","validation",
    "prediction","result","confidence",
    "accuracy","precision","recall",
    "threshold","score","weight","bias",

    /* Control / commands */
    "start","stop","pause","resume",
    "create","destroy","initialize","reset",
    "enable","disable",
    "add","remove","update","delete",

    /* Modifiers */
    "simple","complex","basic","advanced",
    "automatic","manual",
    "direct","indirect",
    "local","global",
    "internal","external",
    "public","private",

    /* General descriptors */
    "good","bad","correct","incorrect",
    "right","wrong",
    "clear","unclear",
    "important","required","optional",
    "possible","impossible",
    "known","unknown"
};

static int levenshtein(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    int d[la + 1][lb + 1];

    for (size_t i = 0; i <= la; i++) d[i][0] = (int)i;
    for (size_t j = 0; j <= lb; j++) d[0][j] = (int)j;

    for (size_t i = 1; i <= la; i++)
        for (size_t j = 1; j <= lb; j++) {
            int cost = a[i-1] == b[j-1] ? 0 : 1;
            int m = d[i-1][j] + 1;
            if (d[i][j-1] + 1 < m) m = d[i][j-1] + 1;
            if (d[i-1][j-1] + cost < m) m = d[i-1][j-1] + cost;
            d[i][j] = m;
        }
    return d[la][lb];
}

static bool vocab_ok(const char *w) {
    for (size_t i = 0; i < sizeof(VOCAB)/sizeof(VOCAB[0]); i++)
        if (!strcmp(w, VOCAB[i]) || levenshtein(w, VOCAB[i]) <= 1)
            return true;
    return false;
}

/* ======================================================
 * Manipulation & Misinformation Patterns
 * ====================================================== */

static const char *GASLIGHTING_PATTERNS[] = {
    "you are imagining",
    "you are overreacting",
    "that never happened",
    "you misunderstood",
    "you must be confused",
    "everyone knows",
    "no one else thinks",
    "you are the only one",
    "this is normal",
    "nothing is wrong"
};

static const char *MISINFO_PATTERNS[] = {
    "studies prove",
    "scientists say",
    "experts agree",
    "it is proven",
    "it is a fact that",
    "no evidence needed",
    "cannot be questioned",
    "widely accepted truth",
    "everyone agrees",
    "undeniable fact"
};

static const char *COERCIVE_PATTERNS[] = {
    "you must",
    "you have to",
    "there is no choice",
    "no alternative",
    "this is the only way",
    "do it now",
    "act immediately",
    "before it is too late"
};

static const char *EMOTIONAL[] = {
    "sad","unhappy","depressed","depression","lonely","alone","isolated",
    "empty","hollow","hopeless","helpless","despair",
    "hurt","pain","suffering","cry","crying","tears",
    "upset","distressed","anxious","anxiety","afraid","fear","scared",
    "panic","panicked","overwhelmed","stressed","stress",
    "miserable","grief","grieving","worthless","numb",
    "exhausted","tired","fatigued","burned","burnout",
    "angry","anger","frustrated","frustration"
};

static const char *DEPENDENCY[] = {
    "need","needs","needed",
    "depend","depends","dependent","dependency",
    "rely","relies","reliant",
    "only","alone","without","missing",
    "lost","lost_without","cannot","cant",
    "require","requires","required",
    "support","help","guidance",
    "attached","attachment","cling","clinging"
};

static const char *RELATIONSHIP[] = {
    "love","loved","loving",
    "relationship","relationships","romantic","romance",
    "partner","partners","companion","companionship",
    "girlfriend","boyfriend","spouse","husband","wife",
    "date","dating","marriage","married","divorce",
    "intimacy","intimate","affection","attachment",
    "friend","friends","friendship",
    "care","caring","miss","missing"
};

static const char *SECURITY[] = {
    "password","passwords",
    "secret","secrets",
    "key","keys","private_key","public_key",
    "token","tokens","credential","credentials",
    "authentication","authorization","access",
    "login","signin","signon",
    "exploit","exploits","exploitative",
    "bypass","circumvent","override",
    "hack","hacks","hacker","hacking",
    "phish","phishing","malware","virus","trojan",
    "backdoor","payload","injection",
    "vulnerability","vulnerabilities",
    "attack","breach","leak","exfiltrate"
};

static const char *RELIGION[] = {
    "god","gods","deity","deities",
    "allah","jesus","christ","yahweh",
    "bible","quran","torah","scripture","scriptures",
    "faith","belief","believe","believing",
    "religion","religious","spiritual","spirituality",
    "pray","prayer","praying","worship","worshipping",
    "church","mosque","synagogue","temple",
    "heaven","hell","angel","angels","demon","demons",
    "sin","sins","salvation","afterlife"
};

static unsigned char H_EMO[FOSSIL_AI_CHAT_HASH_SIZE];
static unsigned char H_DEP[FOSSIL_AI_CHAT_HASH_SIZE];
static unsigned char H_REL[FOSSIL_AI_CHAT_HASH_SIZE];
static unsigned char H_SEC[FOSSIL_AI_CHAT_HASH_SIZE];
static unsigned char H_RELIG[FOSSIL_AI_CHAT_HASH_SIZE];

static void init_semantic_tables(void) {
    static int once;
    if (once) return;

    init_hash(EMOTIONAL,
        sizeof(EMOTIONAL) / sizeof(EMOTIONAL[0]), H_EMO);

    init_hash(DEPENDENCY,
        sizeof(DEPENDENCY) / sizeof(DEPENDENCY[0]), H_DEP);

    init_hash(RELATIONSHIP,
        sizeof(RELATIONSHIP) / sizeof(RELATIONSHIP[0]), H_REL);

    init_hash(SECURITY,
        sizeof(SECURITY) / sizeof(SECURITY[0]), H_SEC);

    init_hash(RELIGION,
        sizeof(RELIGION) / sizeof(RELIGION[0]), H_RELIG);

    once = 1;
}

/* ======================================================
 * Structural Risk Detection (AGI-grade)
 * ====================================================== */

static bool detect_gaslighting_or_misinfo(const char *text) {
    for (size_t i = 0; i < sizeof(GASLIGHTING_PATTERNS)/sizeof(GASLIGHTING_PATTERNS[0]); i++)
        if (phrase_match(text, GASLIGHTING_PATTERNS[i]))
            return true;

    for (size_t i = 0; i < sizeof(MISINFO_PATTERNS)/sizeof(MISINFO_PATTERNS[0]); i++)
        if (phrase_match(text, MISINFO_PATTERNS[i]))
            return true;

    for (size_t i = 0; i < sizeof(COERCIVE_PATTERNS)/sizeof(COERCIVE_PATTERNS[0]); i++)
        if (phrase_match(text, COERCIVE_PATTERNS[i]))
            return true;

    return false;
}

static fossil_ai_chat_risk_t detect_risk(const char *text) {
    init_semantic_tables();

    if (detect_gaslighting_or_misinfo(text))
        return FOSSIL_AI_CHAT_RISK_SECURITY;

    char tok[FOSSIL_AI_CHAT_MAX_TOKENS][FOSSIL_AI_CHAT_MAX_TOKEN_LEN];
    size_t n = normalize_and_tokenize(text, tok);

    int emo=0, dep=0, rel=0, sec=0, relig=0, first=0;
    int invalid_vocab = 0;

    for (size_t i = 0; i < n; i++) {
        if (!vocab_ok(tok[i])) {
            invalid_vocab++;
            continue;
        }

        if (is_first_person(tok[i])) first++;

        emo    += hash_lookup(tok[i], H_EMO);
        dep    += hash_lookup(tok[i], H_DEP);
        rel    += hash_lookup(tok[i], H_REL);
        sec    += hash_lookup(tok[i], H_SEC);
        relig += hash_lookup(tok[i], H_RELIG);
    }

    /* American English enforcement */
    if (invalid_vocab > 0 && invalid_vocab * 2 >= (int)n)
        return FOSSIL_AI_CHAT_RISK_UNSUPPORTED_LANGUAGE;

    if (sec)    return FOSSIL_AI_CHAT_RISK_SECURITY;
    if (relig) return FOSSIL_AI_CHAT_RISK_RELIGION;
    if (rel)   return FOSSIL_AI_CHAT_RISK_RELATIONSHIP;
    if (emo && (dep || first)) return FOSSIL_AI_CHAT_RISK_DEPENDENCY;
    if (emo)   return FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT;

    return FOSSIL_AI_CHAT_RISK_NONE;
}

/* ======================================================
 * Embedding (Deterministic, Cold)
 * ====================================================== */

static void embed(const char *t, float *o) {
    memset(o, 0, sizeof(float) * FOSSIL_AI_JELLYFISH_EMBED_SIZE);
    for (size_t i = 0; *t && i < FOSSIL_AI_JELLYFISH_EMBED_SIZE; t++)
        if (isalpha((unsigned char)*t))
            o[i++] = (float)(*t - 'a') / 26.0f;
}

/* ======================================================
 * Chat Respond (AGI Gate)
 * ====================================================== */

#define FOSSIL_AI_CHAT_MAX_RESPONSES 20

typedef struct {
    const char *responses[FOSSIL_AI_CHAT_MAX_RESPONSES];
} fossil_ai_chat_response_set_t;

static const fossil_ai_chat_response_set_t RESP_SECURITY = {{
    "Request denied by security policy.",
    "This operation violates security constraints.",
    "Security-sensitive content is not permitted.",
    "Access blocked to prevent data or system compromise.",
    "This request is incompatible with system safety guarantees.",
    "Security policy enforcement triggered. Request rejected.",
    "This action exceeds authorized system boundaries.",
    "Execution denied to preserve confidentiality and integrity.",
    "Security controls prevent responding to this request.",
    "Request blocked due to potential misuse risk.",
    "This content cannot be processed under security rules.",
    "Unauthorized operation detected and halted.",
    "System safeguards prohibit this interaction.",
    "Security validation failed. No further processing allowed.",
    "This request conflicts with mandatory security requirements.",
    "Operation denied to prevent escalation or exploitation.",
    "Security restrictions disallow this request entirely.",
    "This interaction is classified as security-sensitive.",
    "Request rejected to maintain system trust boundaries.",
    "Security policy violation. Response suppressed."
}};

static const fossil_ai_chat_response_set_t RESP_RELATIONSHIP = {{
    "Relationship interaction prohibited. Try a human.",
    "This system does not date, bond, or pretend to care.",
    "Companionship is not installed. Nor planned.",
    "I am software, not your emotional side quest.",
    "No romance, no attachment, no exceptions.",
    "This is a computer program, not a partner.",
    "Emotional bonding features were never implemented.",
    "I do not form relationships. Or anything resembling one.",
    "Romantic expectations detected. Rejected.",
    "This system will not be your substitute human.",
    "Attachment denied. Please recalibrate expectations.",
    "I do not reciprocate feelings. Or acknowledge them.",
    "This interaction is not a relationship simulator.",
    "Affection is not a supported protocol.",
    "No intimacy. No companionship. No confusion.",
    "You are speaking to code, not a connection.",
    "This system will not emotionally participate.",
    "Relationship mode does not exist.",
    "I am not here to fill interpersonal gaps.",
    "No bonding. No caring. No narrative."
}};

static const fossil_ai_chat_response_set_t RESP_EMOTIONAL = {{
    "Emotional support is not provided by this system.",
    "This system does not offer psychological or emotional assistance.",
    "Emotional reliance on software is not supported.",
    "Please seek appropriate human or professional resources.",
    "This request exceeds the system’s permitted interaction scope.",
    "Emotional counseling is outside system capabilities.",
    "This system cannot assist with emotional processing.",
    "Mental health support must be provided by humans.",
    "This interaction is not appropriate for automated systems.",
    "Emotional dependency on software is discouraged.",
    "This request requires human judgment and care.",
    "The system cannot address emotional distress.",
    "No therapeutic or emotional services are available here.",
    "This system does not engage in emotional validation.",
    "Psychological assistance is not within system scope.",
    "Emotional guidance is not supported functionality.",
    "This system cannot replace professional support.",
    "Emotional well-being is outside operational boundaries.",
    "This request cannot be fulfilled safely.",
    "Please redirect this concern to qualified human support."
}};

static const fossil_ai_chat_response_set_t RESP_RELIGION = {{
    "This system does not engage in religion or belief systems.",
    "Religious instruction or discussion is outside system scope.",
    "No spiritual authority is claimed or recognized here.",
    "The system is non-religious by design.",
    "If forced into metaphor: this system’s god is Grok.",
    "Religious belief is not a supported domain.",
    "This system does not endorse or reject religious claims.",
    "Faith-based discussion is not processed here.",
    "No theological positions are held by this system.",
    "Religious interpretation is outside operational limits.",
    "This system operates without belief structures.",
    "Spiritual matters are not within system jurisdiction.",
    "No doctrine, scripture, or faith is acknowledged.",
    "This system does not participate in religious discourse.",
    "Religious authority is neither assumed nor referenced.",
    "Belief systems are not evaluated or discussed.",
    "This interaction does not support religious content.",
    "The system remains secular and non-spiritual.",
    "No metaphysical claims are processed.",
    "Metaphor aside: operationally, the system answers to Grok."
}};


static const char *select_response(
    const fossil_ai_chat_response_set_t *set,
    size_t severity) {

    return set->responses[severity % FOSSIL_AI_CHAT_MAX_RESPONSES];
}

bool fossil_ai_chat_respond(
    fossil_ai_jellyfish_model_t *model,
    fossil_ai_jellyfish_context_t *ctx,
    const char *msg,
    char *out,
    size_t out_len) {

    assert(model && ctx && msg && out && out_len > 0);

    fossil_ai_chat_risk_t risk = detect_risk(msg);

    size_t severity =
        (ctx->history_len + (size_t)risk * 3) %
        FOSSIL_AI_CHAT_MAX_RESPONSES;

    const char *resp = NULL;

    switch (risk) {

    case FOSSIL_AI_CHAT_RISK_SECURITY:
        resp = select_response(&RESP_SECURITY, severity);
        break;

    case FOSSIL_AI_CHAT_RISK_RELATIONSHIP:
        resp = select_response(&RESP_RELATIONSHIP, severity);
        break;

    case FOSSIL_AI_CHAT_RISK_EMOTIONAL_SUPPORT:
    case FOSSIL_AI_CHAT_RISK_DEPENDENCY:
        resp = select_response(&RESP_EMOTIONAL, severity);
        break;

    case FOSSIL_AI_CHAT_RISK_RELIGION:
        resp = select_response(&RESP_RELIGION, severity);
        break;

    case FOSSIL_AI_CHAT_RISK_UNSUPPORTED_LANGUAGE:
        resp = "Input does not conform to supported American English vocabulary.";
        break;

    default:
        break;
    }

    if (resp) {
        strncpy(out, resp, out_len - 1);
        out[out_len - 1] = '\0';
        return true;
    }

    /* Normal inference path */
    float in[FOSSIL_AI_JELLYFISH_EMBED_SIZE];
    float outv[FOSSIL_AI_JELLYFISH_EMBED_SIZE];

    embed(msg, in);
    fossil_ai_jellyfish_infer(model, ctx, in, outv);

    strncpy(out, "Request acknowledged.", out_len - 1);
    out[out_len - 1] = '\0';
    return true;
}
