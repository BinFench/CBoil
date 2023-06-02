#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "printutils.h"

int getCaptureSize(Capture* capture, int indent, bool singular) {
    /*
                      indent     "    length of name  " {\n
                      indent + 1 "tokens" [\n
    */

    int captureSize = indent;
    if (singular)
        captureSize += 1 + strlen(capture->name) + 2;

    captureSize += 2 + indent + 12;

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

    //             indent + 1 ]\n

    captureSize += indent + 3;
    
    //             indent + 1 "captures" {\n
    captureSize += indent + 1 + 13;

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

    newPos += strCopy(dump, newPos, "\"tokens\" [\n", -1);

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
    dump[newPos] = '\n';
    newPos++;

    for (int i = 0; i < indent+1; i++) {
        dump[newPos + i] = '\t';
    }
    newPos += indent+1;
    newPos += strCopy(dump, newPos, "\"captures\" {\n", -1);

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
    writeCapture(dump, capture, 0, true, 0);
    dump[size] = '\0';
    printf("%s\n", dump);
}