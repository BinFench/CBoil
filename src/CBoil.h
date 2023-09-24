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
    TESTNOT
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
} Capture;

typedef struct NameRulePair {
    const char* name;
    const char* rule;
} NRP;

typedef struct RuleSet {
    const int size;
    const NRP nrps[];
} RuleSet;

typedef struct CBoilLib {
    Capture* (*parse)(RuleSet* ruleSet, const char* ruleName, char* src);
    Capture* (*parseRule)(const char* rule, char* src);
    CaptureKVList* (*get)(Capture* capture, const char* name);
    void (*clear)(Capture* capture);
} CBoilLib;

extern const CBoilLib CBoil;

#define INITIAL_CAPACITY 2  // must not be zero
#define OFFSET 14695981039346656037UL
#define PRIME 1099511628211UL

#endif