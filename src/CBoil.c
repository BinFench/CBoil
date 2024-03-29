#include <stdbool.h>

#include "strutil.h"
#include "CBoil.h"

const uint8_t HEADER_SIZE = sizeof(Header);
const uint8_t RULE_SIZE = sizeof(Rule) + 8;

static void progress(char** src, Capture* capture, int amount) {
    // Encapsulate substring in token within capture, repoint src by amount
    if (capture && !capture->lastCap) {
        capture->lastCap = malloc(sizeof(Token));
        *(capture->lastCap) = (Token){amount, NULL, capture, NULL, NULL};
        capture->firstCap = capture->lastCap;
        capture->numTokens++;
        append(capture->firstCap, *src, amount);
    } else if (capture) {
        append(capture->lastCap, *src, amount);
    }
    *src += amount;
}

static bool compString(char** src, Capture* capture, const char* string, uint16_t* offset) {
    // Compare strings and progress if match
    *offset += strlen(string) + 1;
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] != (*src)[i]) return false;
    }
    progress(src, capture, strlen(string));
    return true;
}

static uint16_t traverse(uint8_t idx, Rule* rule, uint16_t* offset);

static void _traverse(Rule* rule, uint16_t* offset) {
    // Walks through Rule tree to determine length of top Rule
    uint16_t off = 0;
    switch(rule->type) {
        case ALL:
        case EOI:
            // Rule has no storage
            *offset += RULE_SIZE;
            break;

        case CHARRANGE:
            // Rule stores two chars
            *offset += 2 + HEADER_SIZE;
            break;

        case FIRSTOF:
        case ANYOF:
        case NONEOF:
        case SEQUENCE:
            // Rule has subrules
            traverse(0, rule, &off);
            *offset += off + HEADER_SIZE;
            break;

        case IGNORECASE:
            // Rule has string
            *offset += strlen(rule->child) + 1 + HEADER_SIZE;
            break;

        case CAPTURE:
            // Rule has string + subrule
            if (rule->child[strlen(rule->child)+1] == '\0') {_traverse((Rule*)(rule->child+strlen(rule->child)+1), &off); off++;}
            else *offset += strlen((rule->child+strlen(rule->child)+1)) + 1;
            *offset += off + strlen(rule->child) + 1 + HEADER_SIZE;
            break;

        case ONEORMORE:
        case OPTIONAL:
        case RULE_ENUM:
        case TEST:
        case TESTNOT:
            // Rule has subrule
            if (rule->child[0] == '\0') {_traverse((Rule*)rule->child, &off); off++;}
            else *offset += strlen(rule->child) + 1;
            *offset += off + HEADER_SIZE;
            break;
    }
}

static uint16_t traverse(uint8_t idx, Rule* rule, uint16_t* offset) {
    // Updates offset to the end of this rule's last child
    while(idx < rule->numChildren) {
        // Traverse rule or add string length + null termination to offset
        if (rule->child[*offset] == '\0') {_traverse((Rule*)(rule->child+*offset), offset); (*offset)++;}
        else *offset += strlen(rule->child+*offset) + 1;

        idx++;
    }
    return *offset;
}

static uint64_t hash_key(const char* key) {
    // Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
    // https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
    uint64_t hash = OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= PRIME;
    }
    return hash;
}

static CaptureKVList* get(Capture* capture, const char* name) {
    // Given a Capture and name string, perform hash lookup and return matching CaptureKVList
    if (!capture || capture->numKVs == 0)
        return NULL;

    uint64_t hash = hash_key(name);
    size_t index = (size_t)(hash & (uint64_t)(capture->capacity - 1));

    while (capture->subcaptures[index].key) {
        if (strcmp(name, capture->subcaptures[index].key) == 0)
            return capture->subcaptures + index;
        index++;
        if (index >= capture->capacity) index = 0;
    }
    return NULL;
}

static Capture* insert(Capture* parent, Capture child) {
    // Given a parent and child Captures, perform hash insertion
    if (!parent) {
        parent = malloc(sizeof(Capture));
        *parent = child;
        return parent;
    }
    CaptureKVList* match = get(parent, child.name);
    if (!match && parent->numKVs >= parent->capacity / 2) {
        if (parent->capacity == 0) parent->capacity = INITIAL_CAPACITY;
        else parent->capacity *= 2;

        parent->subcaptures = realloc(parent->subcaptures, parent->capacity*sizeof(CaptureKVList));
        if (parent->capacity == INITIAL_CAPACITY) for (int i = 0; i < INITIAL_CAPACITY; i++)
            parent->subcaptures[i] = (CaptureKVList){NULL, 0, NULL};
        else for (int i = parent->capacity / 2; i < parent->capacity; i++)
            parent->subcaptures[i] = (CaptureKVList){NULL, 0, NULL};
    } else if (match) {
        match->matches++;
        match->captures = realloc(match->captures, match->matches*sizeof(Capture));
        match->captures[match->matches - 1] = child;
        return (match->captures + match->matches - 1);
    }

    uint64_t hash = hash_key(child.name);
    size_t index = (size_t)(hash & (uint64_t)(parent->capacity - 1));

    while (parent->subcaptures[index].key) {
        index++;
        if (index >= parent->capacity) index = 0;
    }

    parent->subcaptures[index] = (CaptureKVList){child.name, 1, malloc(sizeof(Capture))};
    parent->subcaptures[index].captures[0] = child;
    parent->numKVs++;
    return parent->subcaptures[index].captures;
}

static void redir(Capture* old, Capture* new) {
    Token* token = old->firstCap;
    for (int i = 0; i < old->numTokens;) {
        if (old == token->capture) {
            i++;
            token->capture = new;
        }
        token = token->next;
    }
}

static Capture* _parse(RuleSet* ruleSet, Rule* rule, char** src, Capture* capture, bool* match, uint16_t* off, Token* curr) {
    // Walk Rule tree and parse by current Rule
    Capture* cap = capture;
    Capture* seqCap;
    char* currSrc = *src;
    Capture newCap;
    uint8_t idx = 0;
    uint16_t offset = 0;
    bool firstMatch = false;
    bool found = false;
    char a;
    char b;
    char min;
    char max;
    switch(rule->type) {
        case ALL:
            if (strlen(*src) == 0) {
                *match = false;
                break;
            }
            // Matches any char
            progress(src, capture, 1);

            // Rule size is just size of struct, since no children
            *off += RULE_SIZE;
            break;

        case ANYOF:
            // If any subrule matches, match char
            while (idx < rule->numChildren) {
                if (rule->child[offset] == '\0') {
                    cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, NULL, match, &offset, curr);
                    offset++;
                    if (*match) break;
                } else if (compString(src, NULL, rule->child+offset, &offset)) {
                    *match = true;
                    break;
                } else *match = false;
                idx++;
            }

            if (*match) progress(src, capture, 1);
            // Rule size is size of Header + size of all children rules
            *off += traverse(++idx, rule, &offset) + HEADER_SIZE;
            break;

        case CAPTURE:
            // Matches on subrule, captures matching text
            newCap = (Capture){rule->child, 0, 0, 0, NULL, NULL, NULL};
            offset += strlen(rule->child) + 1;

            if (rule->child[offset] == '\0')
                cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, &newCap, match, &offset, curr);
            else if (!compString(src, &newCap, rule->child+offset, &offset)) *match = false;

            // Upon match, add new Token to AST
            if (*match) {
                if (cap == capture || cap == &newCap || capture) {
                    curr = malloc(sizeof(Token));
                    if (!cap || (cap != capture && cap != &newCap)) cap = &newCap;
                    *curr = (Token){0, NULL, cap, NULL, cap->lastCap};
                    if (cap->lastCap) cap->lastCap->next = curr;
                    cap->lastCap = curr;
                }
                cap = insert(capture, newCap);
                redir(&newCap, cap);
            } else if (newCap.firstCap) {
                // Likely potential source of memory leaks, but need test to prove
                if (newCap.firstCap->str) free(newCap.firstCap->str);
                free(newCap.firstCap);
            }

            // Rule size is size of Header + size of subrule
            *off += offset + HEADER_SIZE + 1;

            break;

        case CHARRANGE:
            // Match any char c where min <= c <= max
            a = rule->child[0];
            b = rule->child[1];

            if (a > b) {
                min = b;
                max = a;
            } else {
                min = a;
                max = b;
            }

            if (min <= **src && max >= **src) progress(src, capture, 1);
            else *match = false;

            // Size of rule is size of Header + 2 chars
            *off += HEADER_SIZE + 3;
            break;

        case EOI:
            // Match on null character
            if (**src != '\0') *match = false;

            // Rule size is just size of struct, since no children
            *off += RULE_SIZE;
            break;

        case FIRSTOF:
            // Match on first matching subrule
            while (idx < rule->numChildren) {
                *match = true;
                if (rule->child[offset] == '\0') {
                    cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, capture, match, &offset, curr);
                    offset++;
                    if (*match) break;
                } else if (compString(src, capture, rule->child+offset, &offset)) {
                    *match = true;
                    break;
                } else *match = false;
                idx++;
            }

            // Rule size is size of Header + size of all children rules
            *off += traverse(++idx, rule, &offset) + HEADER_SIZE;
            break;

        case IGNORECASE:
            // Match string ignoring case
            for (int i = 0; i < strlen(rule->child+offset); i++) {
                if (isAlpha((*src)[i]) && isAlpha(rule->child[i])) {
                    if (isLower((*src)[i])) a = (*src)[i];
                    else a = (*src)[i] + 32;
                    
                    if (isLower(rule->child[i])) b = rule->child[i];
                    else b = rule->child[i] + 32;
                    
                    if (a != b) *match = false;
                } else if ((*src)[i] != rule->child[i]) *match = false;
                
                if (!*match) break;
            }
            
            if (*match) progress(src, capture, strlen(rule->child+offset));
            
            // Rule size is size of Header + length of string + null termination
            *off += strlen(rule->child+offset) + HEADER_SIZE + 1;
            break;

        case NONEOF:
            // If no subrules match, match char
            while (idx < rule->numChildren) {
                if (rule->child[offset] == '\0') {
                    cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, NULL, match, &offset, curr);
                    offset++;
                    if (*match) break;
                } else if (compString(src, NULL, rule->child+offset, &offset)) *match = true;
                else *match = false;
                
                if (*match) break;

                idx++;
            }

            if (!*match)
                progress(src, capture, 1);

            *match = !*match;

            // Rule size is size of Header + size of all children rules
            *off += traverse(++idx, rule, &offset) + HEADER_SIZE;
            break;

        case ONEORMORE:
            // Match subrule as many times as possible, but at least once
            while (*match) {
                offset = 0;

                if (rule->child[offset] == '\0')
                    cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, capture, match, &offset, curr);
                else if (!compString(src, capture, rule->child+offset, &offset)) *match = false;

                if (*match) firstMatch = true;
                if (!*match) break;
            }

            if (firstMatch) *match = true;

            // Rule size is size of Header + size of subrule
            *off += offset + HEADER_SIZE + 1;
            break;

        case OPTIONAL:
            // Always match, but only progress if subrule matches
            if (rule->child[offset] == '\0') {
                cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, capture, match, &offset, curr);
                if (match && !cap) cap = capture;
            } else compString(src, capture, rule->child+offset, &offset);
            
            if (!*match) *src = currSrc;

            *match = true;
            // Rule size is size of Header + size of subrule + 1 for termination
            *off += offset + HEADER_SIZE + ((rule->child[0] == '\0') ? 1 : 0);
            break;

        case RULE_ENUM:
            // Match subrule.  Used for recursion
            // Find named rule
            if (ruleSet) {
                for (int i = 0; i < ruleSet->size; i++) {
                    if (strcmp(rule->child, ruleSet->nrps[i].name) == 0) {
                        found = true;
                        cap = _parse(ruleSet, (Rule*)ruleSet->nrps[i].rule, src, capture, match, &offset, curr);
                        // Rule size is size of header + string length + null termination
                        *off += strlen(rule->child) + 1 + HEADER_SIZE;
                        break;
                    }
                }
            }
            if (!found) {
                *match = false;
                // Rule size is size of header + string length + null termination
                *off += strlen(rule->child) + 1 + HEADER_SIZE;
            }
            break;

        case SEQUENCE:
            // Match all subrules
            while (idx < rule->numChildren) {
                if (rule->child[offset] == '\0') {
                    seqCap = _parse(ruleSet, (Rule*)(rule->child+offset), src, capture, match, &offset, curr);
                    if (seqCap) cap = seqCap;
                    offset++;
                    if (!*match) break;
                } else if (!compString(src, capture, rule->child+offset, &offset)) {
                    *match = false;
                    break;
                }

                idx++;
            }

            // Rule size is size of Header + size of subrules
            *off += traverse(++idx, rule, &offset) + HEADER_SIZE;
            break;

        case TEST:
            // Match subrule, never progress
            if (rule->child[offset] == '\0')
                cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, NULL, match, &offset, curr);
            else if (!compString(src, NULL, rule->child+offset, &offset)) *match = false;
            
            // Rule size is size of Header + size of subrule
            *off += offset + HEADER_SIZE;
            *src = currSrc;
            break;

        case TESTNOT:
            // Match is subrule does not match, never progress
            if (rule->child[offset] == '\0')
                cap = _parse(ruleSet, (Rule*)(rule->child+offset), src, NULL, match, &offset, curr);
            else if (!compString(src, NULL, rule->child+offset, &offset)) *match = false;

            *match = !*match;

            // Rule size is size of Header + size of subrule
            *off += offset + HEADER_SIZE;
            *src = currSrc;
            break;
    }
    return cap;
}

static Capture* parse(RuleSet* ruleSet, const char* ruleName, char* src) {
    // Given Rule and string, attempt to match and return top Capture in Rule tree
    if (ruleSet == NULL) return NULL;
    bool match = true;
    Rule* rule = NULL;
    for (int i = 0; i < ruleSet->size; i++) {
        if (strcmp(ruleName, ruleSet->nrps[i].name) == 0) {
            rule = (Rule*)ruleSet->nrps[i].rule;
        }
    }
    uint16_t offset = 0;
    Capture* cap = _parse(ruleSet, rule, &src, NULL, &match, &offset, NULL);
    
    return match ? cap : NULL;
}

static Capture* parseRule(const char* rule, char* src) {
    // Given Rule and string, attempt to match and return top Capture in Rule tree
    bool match = true;
    uint16_t offset = 0;
    Capture* cap = _parse(NULL, (Rule*)rule, &src, NULL, &match, &offset, NULL);
    
    return match ? cap : NULL;
}

static void _clear(Capture* capture, bool isRoot) {
    // Cleanup function

    Token* token = capture->firstCap;
    for (int i = 0; i < capture->numTokens;) {
        if (!token) break;
        Token* next = token->next;
        if (capture == token->capture) {
            i++;
            free(token->str);
            free(token);
        } else if (token == capture->firstCap) {
            free(token->str);
            free(token);
        }
        token = next;
    }
    if (token) free(token);
    if (capture->lastCap != token && capture->lastCap != capture->firstCap) free(capture->lastCap);

    if (capture->numKVs != 0) {
        for (int i = 0; i < capture->capacity; i++) {
            CaptureKVList* captures = capture->subcaptures+i;
            if (captures && captures->matches > 0) {
                for (int j = 0; j < captures->matches; j++) {
                    _clear(captures->captures+j, false);
                }
                free(captures->captures);
            }
        }
        free(capture->subcaptures);
    }

    if (isRoot) free(capture);
}

static void clear(Capture* capture) {
    _clear(capture, true);
}

const CBoilLib CBoil = {parse, parseRule, get, clear};