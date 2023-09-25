#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "printutils.h"

int getCaptureSize(Capture* capture, int indent, bool singular) {
    /*
                      indent     "    length of name  ": {\n
                      indent + 1 "tokens": [\n
    */

    int captureSize = indent;
    if (singular)
        captureSize += 1 + strlen(capture->name) + 3;

    captureSize += 2 + indent + 13;

    Token* token = capture->firstCap;
    for (int i = 0; i < capture->numTokens;) {
        if (!token) break;
        if (capture == token->capture) {
            i++;
            captureSize += indent + 3 + token->size + 2;
            if (i < capture->numTokens) captureSize += 1;
        }
        token = token->next;
    }

    //             indent + 1 ],\n

    captureSize += indent + 4;
    
    //             indent + 1 "captures": {\n
    captureSize += indent + 1 + 14;

    if (capture->numKVs != 0) {
        bool first = true;
        for (int i = 0; i < capture->capacity; i++) {
            CaptureKVList* captures = &capture->subcaptures[i];
            if (captures && captures->matches > 0) {
                if (!first) {
                    captureSize += 2;
                }
                first = false;
                if (captures->matches > 1) {
                    //             indent + 2 " key name " [\n
                    captureSize += indent + 3 + strlen(captures->key) + 4;
                    for (int j = 0; j < captures->matches; j++) {
                        captureSize += getCaptureSize(&captures->captures[j], indent + 3, false) + 1;
                        if (j < captures->matches - 1)
                            captureSize += 1;
                    }
                    //             indent + 2 ]
                    captureSize += indent + 4;
                } else {
                    captureSize += getCaptureSize(captures->captures, indent + 2, true) + 1;
                }
            }
        }
    }

    //             indent + 1 }\n
    captureSize += indent + 3;
    //             indent }
    captureSize += indent + 1;

    return captureSize;
}

int strCopy(char* dump, int pos, const char* string, int len) {
    int strLen = len;
    if (len < 0) strLen = strlen(string);

    for (int i = 0; i < strLen; i++) dump[pos + i] =  string[i];

    return strLen;
}

int writeCapture(char* dump, Capture* capture, int indent, bool singular, int pos) {
    int newPos = pos + indent;
    for (int i = 0; i < indent; i++) {
        dump[pos + i] = '\t';
    }
    
    if (singular) {
        dump[newPos] = '"';
        newPos++;
        newPos += strCopy(dump, newPos, capture->name, -1);
        dump[newPos] = '"';
        newPos++;
        dump[newPos] = ':';
        newPos++;
        dump[newPos] = ' ';
        newPos++;
    }

    dump[newPos] = '{';
    newPos++;
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;

    newPos += strCopy(dump, newPos, "\"tokens\": [\n", -1);

    Token* token = capture->firstCap;
    for (int i = 0; i < capture->numTokens;) {
        if (!token) break;
        if (capture == token->capture) {
            i++;
            for (int i = 0; i < indent+2; i++) {
                dump[newPos + i] = '\t';
            }
            newPos += indent+2;
            dump[newPos] = '"';
            newPos++;
            newPos += strCopy(dump, newPos, token->str, token->size);
            dump[newPos] = '"';
            newPos++;
            if (i < capture->numTokens) {
                dump[newPos] = ',';
                newPos++;
            }
            dump[newPos] = '\n';
            newPos++;
        }
        token = token->next;
    }

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;
    dump[newPos] = ']';
    newPos++;
    dump[newPos] = ',';
    newPos++;
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;
    newPos += strCopy(dump, newPos, "\"captures\": {\n", -1);

    if (capture->numKVs != 0) {
        bool first = true;
        for (int i = 0; i < capture->capacity; i++) {
            CaptureKVList* captures = &capture->subcaptures[i];
            if (captures && captures->matches > 0) {
                if (!first) {
                    dump[newPos] = ',';
                    newPos++;
                    dump[newPos] = '\n';
                    newPos++;
                }
                first = false;
                if (captures->matches > 1) {
                    for (int i = 0; i < indent+2; i++) {
                        dump[newPos + i] = '\t';
                    }
                    newPos += indent+2;
                    dump[newPos] = '"';
                    newPos++;
                    newPos += strCopy(dump, newPos, captures->key, -1);
                    dump[newPos] = '"';
                    newPos++;
                    dump[newPos] = ' ';
                    newPos++;
                    dump[newPos] = '[';
                    newPos++;
                    dump[newPos] = '\n';
                    newPos++;
                    for (int j = 0; j < captures->matches; j++) {
                        newPos = writeCapture(dump, &captures->captures[j], indent + 3, false, newPos);
                        if (j < captures->matches - 1) {
                            dump[newPos] = ',';
                            newPos++;
                        }
                        dump[newPos] = '\n';
                        newPos++;
                    }

                    for (int j = 0; j < indent+2; j++) {
                        dump[newPos + j] = '\t';
                    }
                    newPos += indent+2;
                    dump[newPos] = ']';
                    newPos++;
                } else {
                    newPos = writeCapture(dump, captures->captures, indent + 2, true, newPos);
                }
                dump[newPos] = '\n';
                newPos++;
            }
        }
    }

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;
    dump[newPos] = '}';
    newPos++;
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent;
    dump[newPos] = '}';
    newPos++;

    return newPos;
}

void dumpCapture(Capture* capture) {
    const int size = getCaptureSize(capture, 0, true);
    char dump[size + 1];
    for (int i = 0; i < size+1; i++)
        dump[i] = '\0';
    writeCapture(dump, capture, 0, true, 0);
    dump[size] = '\0';
    printf("%s\n", dump);
}

int numPlaces(int n) {
    if (n < 10) return 1;
    if (n < 100) return 2;
    return 3;
}

int getRuleSize(Rule* rule, int indent, bool singular, int* off) {
    // Assume name not attached to rule
    int size = indent + 2 + indent + 1 + 7 + 6;
    int offset = 0;
    int idx = 0;
    int typelens[] = {3, 5, 7, 9, 3, 7, 10, 6, 9, 8, 7, 8, 4, 7};
    size += typelens[rule->type];
    size += 3 + indent + 1 + 13 + numPlaces(rule->numChildren);

    if (rule->numChildren > 0) size += 2 + indent + 1 + 9;

    while (idx < rule->numChildren) {
        if (*(rule->child + offset) == '\0')
            size += getRuleSize((Rule*)(rule->child + offset), indent + 2, rule->numChildren == 1, &offset);
        else {
            size += indent + 3;
            int newOff = strlen(rule->child + offset);
            size += newOff + 1;
            offset += newOff + 1;
        }

        if (idx < rule->numChildren - 1) size += 1;
        size += 1;
        idx++;
    }

    if (rule->numChildren > 0) size += indent + 2;

    size += 1 + indent + 1;

    if (rule->numChildren == 0) *off += sizeof(Rule);
    else *off += sizeof(Header) + 1 + offset;

    return size;
}

int writeRule(char* dump, Rule* rule, int* off, int indent, int pos) {
    int newPos = pos + indent;
    for (int i = 0; i < indent; i++) {
        dump[pos + i] = '\t';
    }

    dump[newPos] = '{';
    newPos++;
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;

    dump[newPos] = '"';
    newPos++;
    dump[newPos] = 't';
    newPos++;
    dump[newPos] = 'y';
    newPos++;
    dump[newPos] = 'p';
    newPos++;
    dump[newPos] = 'e';
    newPos++;
    dump[newPos] = '"';
    newPos++;
    dump[newPos] = ':';
    newPos++;
    dump[newPos] = ' ';
    newPos++;
    dump[newPos] = '"';
    newPos++;

    switch(rule->type) {
        case ALL:
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'L';
            newPos++;
            dump[newPos] = 'L';
            newPos++;
            break;
        case ANYOF:
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'Y';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'F';
            newPos++;
            break;
        case CAPTURE:
            dump[newPos] = 'C';
            newPos++;
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'P';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'U';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case CHARRANGE:
            dump[newPos] = 'C';
            newPos++;
            dump[newPos] = 'H';
            newPos++;
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'G';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case EOI:
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'I';
            newPos++;
            break;
        case FIRSTOF:
            dump[newPos] = 'F';
            newPos++;
            dump[newPos] = 'I';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'F';
            newPos++;
            break;
        case IGNORECASE:
            dump[newPos] = 'I';
            newPos++;
            dump[newPos] = 'G';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'C';
            newPos++;
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case NONEOF:
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'F';
            newPos++;
            break;
        case ONEORMORE:
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'M';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case OPTIONAL:
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'P';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'I';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'A';
            newPos++;
            dump[newPos] = 'L';
            newPos++;
            break;
        case RULE_ENUM:
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'U';
            newPos++;
            dump[newPos] = 'B';
            newPos++;
            dump[newPos] = 'R';
            newPos++;
            dump[newPos] = 'U';
            newPos++;
            dump[newPos] = 'L';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case SEQUENCE:
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'Q';
            newPos++;
            dump[newPos] = 'U';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'C';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            break;
        case TEST:
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            break;
        case TESTNOT:
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'E';
            newPos++;
            dump[newPos] = 'S';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            dump[newPos] = 'N';
            newPos++;
            dump[newPos] = 'O';
            newPos++;
            dump[newPos] = 'T';
            newPos++;
            break;
    }

    dump[newPos] = '"';
    newPos++;
    dump[newPos] = ',';
    newPos++;
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;

    dump[newPos] = '"';
    newPos++;
    dump[newPos] = 'n';
    newPos++;
    dump[newPos] = 'u';
    newPos++;
    dump[newPos] = 'm';
    newPos++;
    dump[newPos] = 'C';
    newPos++;
    dump[newPos] = 'h';
    newPos++;
    dump[newPos] = 'i';
    newPos++;
    dump[newPos] = 'l';
    newPos++;
    dump[newPos] = 'd';
    newPos++;
    dump[newPos] = 'r';
    newPos++;
    dump[newPos] = 'e';
    newPos++;
    dump[newPos] = 'n';
    newPos++;
    dump[newPos] = '"';
    newPos++;
    dump[newPos] = ':';
    newPos++;
    dump[newPos] = ' ';
    newPos++;

    int divider = 100;
    for (int i = 0; i < 3 - numPlaces(rule->numChildren); i++) divider /= 10;
    for (int i = numPlaces(rule->numChildren); i > 0; i--) {
        dump[newPos] = 48 + rule->numChildren / divider;
        newPos++;
    }

    if (rule->numChildren > 0) {
        dump[newPos] = ',';
        newPos++;
        dump[newPos] = '\n';
        newPos++;

        for (int i = 0; i < indent+1; i++) {
            dump[newPos + i] = '\t';
        }
        newPos += indent+1;

        dump[newPos] = '"';
        newPos++;
        dump[newPos] = 'r';
        newPos++;
        dump[newPos] = 'u';
        newPos++;
        dump[newPos] = 'l';
        newPos++;
        dump[newPos] = 'e';
        newPos++;
        dump[newPos] = 's';
        newPos++;
        dump[newPos] = '"';
        newPos++;
        dump[newPos] = ':';
        newPos++;
        dump[newPos] = ' ';
        newPos++;
        dump[newPos] = '[';
        newPos++;
        dump[newPos] = '\n';
        newPos++;

        int offset = 0;
        for (int i = 0; i < rule->numChildren; i++) {
            if (*(rule->child + offset) == '\0')
                newPos = writeRule(dump, (Rule*)(rule->child + offset), &offset, indent + 2, newPos);
            else {
                for (int i = 0; i < indent+2; i++) {
                    dump[newPos + i] = '\t';
                }
                newPos += indent+2;
                dump[newPos] = '"';
                newPos++;
                newPos += strCopy(dump, newPos, rule->child + offset, -1);
                dump[newPos] = '"';
                newPos++;
                offset += strlen(rule->child + offset) + 1;
            }
            if (i < rule->numChildren - 1) {
                dump[newPos] = ',';
                newPos++;
            }
            dump[newPos] = '\n';
            newPos++;
        }

        *off += offset + sizeof(Header) + 1;

        for (int i = 0; i < indent+1; i++) {
            dump[newPos + i] = '\t';
        }
        newPos += indent+1;

        dump[newPos] = ']';
        newPos++;
    } else {
        *off += sizeof(Rule);
    }

    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent;

    dump[newPos] = '}';
    newPos++;

    return newPos;
}

void dumpRule(const char* name, const char* rule) {
    // Print a Rule as JSON string
    int offset = 0;
    int nameLen = strlen(name);
    const int size = getRuleSize((Rule*)rule, 0, true, &offset) + nameLen + 5;
    char dump[size + 1];
    for (int i = 0; i < size+1; i++)
        dump[i] = '\0';
    dump[0] = '"';
    strCopy(dump, 1, name, nameLen);
    dump[nameLen+1] = '"';
    dump[nameLen+2] = ':';
    dump[nameLen+3] = ' ';
    int off = 0;
    writeRule(dump, (Rule*)rule, &off, 0, nameLen + 4);
    dump[size] = '\0';
    printf("%s\n", dump);
}

void dumpRuleSet(RuleSet* ruleSet) {
    if (ruleSet) {
        for (int i = 0; i < ruleSet->size; i++) {
            dumpRule(ruleSet->nrps[i].name, ruleSet->nrps[i].rule);
        }
    }
}

#ifdef CSONDEP
#include <CSON.h>

JSONObject* ruleToJSON(const char* rule) {
    // Print a Rule as JSON string
    int offset = 0;
    const int size = getRuleSize((Rule*)rule, 0, true, &offset);
    char dump[size + 1];
    for (int i = 0; i < size+1; i++)
        dump[i] = '\0';
    int off = 0;
    writeRule(dump, (Rule*)rule, &off, 0, 0);
    dump[size] = '\0';

    return CSON.parse(dump);
}

JSONObject* captureToJSON(Capture* capture) {
    const int size = getCaptureSize(capture, 0, true) + 2;
    char dump[size + 1];
    for (int i = 0; i < size+1; i++)
        dump[i] = '\0';
    dump[0] = '{';
    dump[size-1] = '}';
    writeCapture(dump, capture, 0, true, 1);
    dump[size] = '\0';

    return CSON.parse(dump);
}

#endif