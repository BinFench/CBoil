#ifndef STRUTIL
#define STRUTIL

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "CBoil.h"

void append(Token* token, char* str, uint16_t len) {
    // Append str[0:len] to dest string, fails if out of memory
    uint16_t size = (token->str) ? strlen(token->str) : 0;
    token->str = (char*)realloc(token->str, size + len + 1);
    assert(token->str);
    memcpy((token->str + size), str, len);
    token->str[size + len] = '\0';
    token->size = size + len;
}

bool isLower(char ch) {
    return (97 <= ch && ch <= 122);
}

bool isUpper(char ch) {
    return (65 <= ch && ch <= 90);
}

bool isAlpha(char ch) {
    return isLower(ch) || isUpper(ch);
}

#endif