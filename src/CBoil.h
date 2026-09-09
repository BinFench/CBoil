#ifndef CBOILLIB
#define CBOILLIB

#include <stdint.h>

#include "CBoil.def"

typedef enum RTYPE {
    ALL,
    ANYOF,
    CAPTURE,
    CHARRANGE,
    EOI,
    FIRSTOF,
    IGNORECASE,
    NONEOF,
    ONEORMORE,
    OPTIONAL,
    RULE_ENUM,
    SEQUENCE,
    TEST,
    TESTNOT,
    TRANSFORM
} RTYPE;

typedef struct Rule {
    const char magic;
    const uint8_t type;
    const uint8_t numChildren;
    const char child[];
} Rule;

typedef struct Header {
    const char magic;
    const uint8_t type;
    const uint8_t numChildren;
} Header;

typedef struct Capture Capture;
typedef struct RuleSet RuleSet;

typedef struct Token {
    uint16_t size;
    char* str;
    Capture* capture;
    struct Token* next;
    struct Token* prev;
} Token;

typedef struct CaptureKVList {
    const char* key;
    uint16_t matches;
    Capture* captures;
} CaptureKVList;

typedef struct Capture {
    const char* name;
    uint16_t numTokens;
    uint16_t numKVs;
    uint16_t capacity;
    CaptureKVList* subcaptures;
    Capture* next;
    Token* firstCap;
    Token* lastCap;
    void* structure;
    RuleSet* ruleSet;
} Capture;

typedef struct NameRulePair {
    const char* name;
    const char* rule;
} NRP;

typedef void* (*transformFunc)(Capture*);
typedef void (*cleanupFunc)(void*);

typedef struct NameFunctionPair {
    const char* name;
    const transformFunc function;
} NFP;

typedef struct CleanupFunctionPair {
    const char* name;
    const cleanupFunc function;
} CFP;

typedef union Pair {
    const NRP nrp;
    const NFP nfp;
    const CFP cfp;
} Pair;

typedef struct RuleSet {
    const int ruleSize;
    const int transformSize;
    const int cleanupSize;
    const Pair pairs[];
} RuleSet;

typedef struct CBoilMemLib {
    void (*freeFunc)(void*);
    void* (*mallocFunc)(size_t);
    void* (*reallocFunc)(void*, size_t);
} CBoilMemLib;

typedef struct CBoilLib {
    Capture* (*parse)(RuleSet* ruleSet, const char* ruleName, char* src);
    Capture* (*parseRule)(const char* rule, char* src);
    CaptureKVList* (*get)(Capture* capture, const char* name);
    void (*clear)(Capture* capture);
    void (*freeFunc)(void*);
    void* (*mallocFunc)(size_t);
    void* (*reallocFunc)(void*, size_t);
    CBoilMemLib* cml;
    void (*setMemFuncs)(void (*freeFunc)(void*), void* (*mallocFunc)(size_t), void* (*reallocFunc)(void*, size_t));
} CBoilLib;

// Forward declarations - these need to be non-static and exported from CBoil.c
Capture* cboil_parse(CBoilMemLib* cml, RuleSet* ruleSet, const char* ruleName, char* src);
Capture* cboil_parseRule(CBoilMemLib* cml, const char* rule, char* src);
CaptureKVList* cboil_get(Capture* capture, const char* name);
void cboil_clear(CBoilMemLib* cml, Capture* capture);

// Global context for current CBoil instance (set by CBOIL_INIT)
static CBoilMemLib* _global_cboil_context = NULL;

// Wrapper functions that use the global context
static inline Capture* _cboil_parse_wrapper(RuleSet* ruleSet, const char* ruleName, char* src) {
    return cboil_parse(_global_cboil_context, ruleSet, ruleName, src);
}
static inline Capture* _cboil_parseRule_wrapper(const char* rule, char* src) {
    return cboil_parseRule(_global_cboil_context, rule, src);
}
static inline CaptureKVList* _cboil_get_wrapper(Capture* capture, const char* name) {
    return cboil_get(capture, name);
}
static inline void _cboil_clear_wrapper(Capture* capture) {
    cboil_clear(_global_cboil_context, capture);
}
static inline void _cboil_free_wrapper(void* ptr) {
    _global_cboil_context->freeFunc(ptr);
}
static inline void* _cboil_malloc_wrapper(size_t size) {
    return _global_cboil_context->mallocFunc(size);
}
static inline void* _cboil_realloc_wrapper(void* ptr, size_t size) {
    return _global_cboil_context->reallocFunc(ptr, size);
}
static inline void _cboil_setMemFuncs_wrapper(void (*freeFunc)(void*), void* (*mallocFunc)(size_t), void* (*reallocFunc)(void*, size_t)) {
    _global_cboil_context->freeFunc = freeFunc;
    _global_cboil_context->mallocFunc = mallocFunc;
    _global_cboil_context->reallocFunc = reallocFunc;
}

// Global CBoil instance (defined when CBOIL_INIT is called)
static CBoilLib _global_cboil_struct __attribute__((unused));
#define CBoil _global_cboil_struct

#define CBOIL_INIT() \
    static CBoilMemLib _cboil_memlib = {free, malloc, realloc}; \
    _global_cboil_context = &_cboil_memlib; \
    \
    _global_cboil_struct = (CBoilLib){ \
        .parse = _cboil_parse_wrapper, \
        .parseRule = _cboil_parseRule_wrapper, \
        .get = _cboil_get_wrapper, \
        .clear = _cboil_clear_wrapper, \
        .freeFunc = _cboil_free_wrapper, \
        .mallocFunc = _cboil_malloc_wrapper, \
        .reallocFunc = _cboil_realloc_wrapper, \
        .cml = &_cboil_memlib, \
        .setMemFuncs = _cboil_setMemFuncs_wrapper, \
    }

#define CBOIL_CLEANUP() \
    _global_cboil_context = NULL

#define INITIAL_CAPACITY 2  // must not be zero
#define OFFSET 14695981039346656037UL
#define PRIME 1099511628211UL

#endif