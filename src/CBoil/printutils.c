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
    int newPos = pos;
    for (int i = 0; i < indent; i++)
        newPos += strCopy(dump, newPos, "\t", -1);
    
    if (singular) {
        newPos += strCopy(dump, newPos, "\"", -1);
        newPos += strCopy(dump, newPos, capture->name, -1);
        newPos += strCopy(dump, newPos, "\": ", -1);
    }

    newPos += strCopy(dump, newPos, "{\n", -1);

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);

    newPos += strCopy(dump, newPos, "\"tokens\": [\n", -1);

    Token* token = capture->firstCap;
    for (int i = 0; i < capture->numTokens;) {
        if (!token) break;
        if (capture == token->capture) {
            i++;
            for (int i = 0; i < indent+2; i++)
                newPos += strCopy(dump, newPos, "\t", -1);
            newPos += strCopy(dump, newPos, "\"", -1);
            newPos += strCopy(dump, newPos, token->str, token->size);
            newPos += strCopy(dump, newPos, "\"", -1);
            if (i < capture->numTokens)
                newPos += strCopy(dump, newPos, ",", -1);
            newPos += strCopy(dump, newPos, "\n", -1);
        }
        token = token->next;
    }

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);
    newPos += strCopy(dump, newPos, "],\n", -1);

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);
    newPos += strCopy(dump, newPos, "\"captures\": {\n", -1);

    if (capture->numKVs != 0) {
        bool first = true;
        for (int i = 0; i < capture->capacity; i++) {
            CaptureKVList* captures = &capture->subcaptures[i];
            if (captures && captures->matches > 0) {
                if (!first) 
                    newPos += strCopy(dump, newPos, ",\n", -1);
                first = false;
                if (captures->matches > 1) {
                    for (int i = 0; i < indent+2; i++)
                        newPos += strCopy(dump, newPos, "\t", -1);
                    newPos += strCopy(dump, newPos, "\"", -1);
                    newPos += strCopy(dump, newPos, captures->key, -1);
                    newPos += strCopy(dump, newPos, "\" [\n", -1);
                    for (int j = 0; j < captures->matches; j++) {
                        newPos = writeCapture(dump, &captures->captures[j], indent + 3, false, newPos);
                        if (j < captures->matches - 1) 
                            newPos += strCopy(dump, newPos, ",", -1);
                        newPos += strCopy(dump, newPos, "\n", -1);
                    }

                    for (int j = 0; j < indent+2; j++)
                        newPos += strCopy(dump, newPos, "\t", -1);
                    newPos += strCopy(dump, newPos, "]", -1);
                } else {
                    newPos = writeCapture(dump, captures->captures, indent + 2, true, newPos);
                }
                newPos += strCopy(dump, newPos, "\n", -1);
            }
        }
    }

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);
    newPos += strCopy(dump, newPos, "}\n", -1);

    for (int i = 0; i < indent; i++)
        newPos += strCopy(dump, newPos, "\t", -1);
    newPos += strCopy(dump, newPos, "}", -1);

    return newPos;
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
    int newPos = pos;
    for (int i = 0; i < indent; i++)
        newPos += strCopy(dump, newPos, "\t", -1);

    newPos += strCopy(dump, newPos, "{\n", -1);

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);

    newPos += strCopy(dump, newPos, "\"type\": \"", -1);

    switch(rule->type) {
        case ALL:
            newPos += strCopy(dump, newPos, "ALL", -1);
            break;
        case ANYOF:
            newPos += strCopy(dump, newPos, "ANYOF", -1);
            break;
        case CAPTURE:
            newPos += strCopy(dump, newPos, "CAPTURE", -1);
            break;
        case CHARRANGE:
            newPos += strCopy(dump, newPos, "CHARRANGE", -1);
            break;
        case EOI:
            newPos += strCopy(dump, newPos, "EOI", -1);
            break;
        case FIRSTOF:
            newPos += strCopy(dump, newPos, "FIRSTOF", -1);
            break;
        case IGNORECASE:
            newPos += strCopy(dump, newPos, "IGNORECASE", -1);
            break;
        case NONEOF:
            newPos += strCopy(dump, newPos, "NONEOF", -1);
            break;
        case ONEORMORE:
            newPos += strCopy(dump, newPos, "ONEORMORE", -1);
            break;
        case OPTIONAL:
            newPos += strCopy(dump, newPos, "OPTIONAL", -1);
            break;
        case RULE_ENUM:
            newPos += strCopy(dump, newPos, "SUBRULE", -1);
            break;
        case SEQUENCE:
            newPos += strCopy(dump, newPos, "SEQUENCE", -1);
            break;
        case TEST:
            newPos += strCopy(dump, newPos, "TEST", -1);
            break;
        case TESTNOT:
            newPos += strCopy(dump, newPos, "TESTNOT", -1);
            break;
    }

    newPos += strCopy(dump, newPos, "\",\n", -1);

    for (int i = 0; i < indent+1; i++)
        newPos += strCopy(dump, newPos, "\t", -1);

    newPos += strCopy(dump, newPos, "\"numChildren\": ", -1);

    int divider = 100;
    for (int i = 0; i < 3 - numPlaces(rule->numChildren); i++) divider /= 10;
    for (int i = numPlaces(rule->numChildren); i > 0; i--) {
        dump[newPos] = 48 + rule->numChildren / divider;
        newPos++;
    }

    if (rule->numChildren > 0) {
        newPos += strCopy(dump, newPos, ",\n", -1);

        for (int i = 0; i < indent+1; i++)
            newPos += strCopy(dump, newPos, "\t", -1);

        newPos += strCopy(dump, newPos, "\"rules\": [\n", -1);

        int offset = 0;
        for (int i = 0; i < rule->numChildren; i++) {
            if (*(rule->child + offset) == '\0')
                newPos = writeRule(dump, (Rule*)(rule->child + offset), &offset, indent + 2, newPos);
            else {
                for (int i = 0; i < indent+2; i++)
                    newPos += strCopy(dump, newPos, "\t", -1);
                newPos += strCopy(dump, newPos, "\"", -1);
                newPos += strCopy(dump, newPos, rule->child + offset, -1);
                newPos += strCopy(dump, newPos, "\"", -1);
                offset += strlen(rule->child + offset) + 1;
            }
            if (i < rule->numChildren - 1)
                newPos += strCopy(dump, newPos, ",", -1);
            newPos += strCopy(dump, newPos, "\n", -1);
        }

        *off += offset + sizeof(Header) + 1;

        for (int i = 0; i < indent+1; i++)
            newPos += strCopy(dump, newPos, "\t", -1);

        newPos += strCopy(dump, newPos, "]", -1);
    } else
        *off += sizeof(Rule);

    newPos += strCopy(dump, newPos, "\n", -1);

    for (int i = 0; i < indent; i++)
        newPos += strCopy(dump, newPos, "\t", -1);

    newPos += strCopy(dump, newPos, "}", -1);

    return newPos;
}

#ifdef CSONDEP
#include <CSON.h>

JSONObject* ruleToJSON(const char* rule) {
    char* name = "rule";
    dumpRule(dump, name, rule);

    return CSON.parse(dump);
}

JSONObject* captureToJSON(Capture* capture) {
    dumpCapture(dump, capture);

    return CSON.parse(dump);
}

#endif