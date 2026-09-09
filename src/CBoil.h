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
    void (*setMemFuncs)(void (*freeFunc)(void*), void* (*mallocFunc)(size_t), void* (*reallocFunc)(void*, size_t));
} CBoilLib;

// Forward declarations - these need to be non-static and exported from CBoil.c
Capture* cboil_parse(CBoilMemLib* cml, RuleSet* ruleSet, const char* ruleName, char* src);
Capture* cboil_parseRule(CBoilMemLib* cml, const char* rule, char* src);
CaptureKVList* cboil_get(Capture* capture, const char* name);
void cboil_clear(CBoilMemLib* cml, Capture* capture);

#define CBOIL_INIT() \
    CBoilMemLib _cboil_memlib = {free, malloc, realloc}; \
    \
    Capture* _cboil_parse(RuleSet* rs, const char* rn, char* src) { return cboil_parse(&_cboil_memlib, rs, rn, src); } \
    Capture* _cboil_parseRule(const char* rule, char* src) { return cboil_parseRule(&_cboil_memlib, rule, src); } \
    CaptureKVList* _cboil_get(Capture* cap, const char* name) { return cboil_get(cap, name); } \
    void _cboil_clear(Capture* cap) { cboil_clear(&_cboil_memlib, cap); } \
    void _cboil_freeFunc(void* ptr) { _cboil_memlib.freeFunc(ptr); } \
    void* _cboil_mallocFunc(size_t sz) { return _cboil_memlib.mallocFunc(sz); } \
    void* _cboil_reallocFunc(void* ptr, size_t sz) { return _cboil_memlib.reallocFunc(ptr, sz); } \
    void _cboil_setMemFuncs(void (*ff)(void*), void* (*mf)(size_t), void* (*rf)(void*, size_t)) { \
        _cboil_memlib.freeFunc = ff; _cboil_memlib.mallocFunc = mf; _cboil_memlib.reallocFunc = rf; \
    } \
    \
    CBoilLib CBoil = { \
        .parse = _cboil_parse, \
        .parseRule = _cboil_parseRule, \
        .get = _cboil_get, \
        .clear = _cboil_clear, \
        .freeFunc = _cboil_freeFunc, \
        .mallocFunc = _cboil_mallocFunc, \
        .reallocFunc = _cboil_reallocFunc, \
        .setMemFuncs = _cboil_setMemFuncs, \
    }

#define INITIAL_CAPACITY 2  // must not be zero
#define OFFSET 14695981039346656037UL
#define PRIME 1099511628211UL

#endif
