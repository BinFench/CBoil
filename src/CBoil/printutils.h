#include "../CBoil.h"

#ifndef CBOIL_PRINT_UTILS
#define CBOIL_PRINT_UTILS

#define CONCAT(a, b) a##b

int getCaptureSize(Capture* capture, int indent, bool singular);
int writeCapture(char* dump, Capture* capture, int indent, bool singular, int pos);

#define _dumpCapture(name, capture, num)\
    int CONCAT(CBOIL__DUMPSIZE, num) = getCaptureSize(capture, 0, true);\
    char name[CONCAT(CBOIL__DUMPSIZE, num) + 3];\
    name[0] = '{';\
    writeCapture(name, capture, 0, true, 1);\
    name[CONCAT(CBOIL__DUMPSIZE, num) + 1] = '}';\
    name[CONCAT(CBOIL__DUMPSIZE, num) + 2] = '\0';
#define dumpCapture(name, capture) _dumpCapture(name, capture, __COUNTER__)

int strCopy(char* dump, int pos, const char* string, int len);

int getRuleSize(Rule* rule, int indent, bool singular, int* off);
int writeRule(char* dump, Rule* rule, int* off, int indent, int pos);

#define _dumpRule(name, rname, rule, num)\
    int CONCAT(CBOIL__OFFSET, num) = 0;\
    int CONCAT(CBOIL__NAMELEN, num) = strlen(rname);\
    int CONCAT(CBOIL__DUMPSIZE, num) = CONCAT(CBOIL__NAMELEN, num) + 5 + getRuleSize((Rule*)rule, 0, true, &CONCAT(CBOIL__OFFSET, num));\
    char name[CONCAT(CBOIL__DUMPSIZE, num) + 3];\
    name[0] = '{';\
    name[1] = '"';\
    strCopy(name, 2, rname, CONCAT(CBOIL__NAMELEN, num));\
    name[CONCAT(CBOIL__NAMELEN, num)+2] = '"';\
    name[CONCAT(CBOIL__NAMELEN, num)+3] = ':';\
    name[CONCAT(CBOIL__NAMELEN, num)+4] = ' ';\
    CONCAT(CBOIL__OFFSET, num) = 0;\
    writeRule(name, (Rule*)rule, &CONCAT(CBOIL__OFFSET, num), 0, CONCAT(CBOIL__NAMELEN, num)+5);\
    name[CONCAT(CBOIL__DUMPSIZE, num)] = '}';\
    name[CONCAT(CBOIL__DUMPSIZE, num)+1] = '\0';
#define dumpRule(name, rname, rule) _dumpRule(name, rname, rule, __COUNTER__);

#define _dumpRules(dumpname, rules, num)\
    int CONCAT(CBOIL__DUMPSIZE, num) = rules.size*2 + 1;\
    for (int CBOIL__i = 0; CBOIL__i < rules.size; CBOIL__i++) {\
        int CONCAT(CBOIL__OFFSET, num) = 0;\
        CONCAT(CBOIL__DUMPSIZE, num) += strlen(rules.nrps[CBOIL__i].name) + 5 + getRuleSize((Rule*)rules.nrps[CBOIL__i].rule, 0, true, &CONCAT(CBOIL__OFFSET, num));}\
    int CONCAT(CBOIL__OFFSET, num) = 2;\
    char dumpname[CONCAT(CBOIL__DUMPSIZE, num)+3];\
    dumpname[0] = '{';\
    dumpname[1] = '\n';\
    for (int CBOIL__i = 0; CBOIL__i < rules.size; CBOIL__i++) {\
        int CONCAT(CBOIL__NAMELEN, num) = strlen(rules.nrps[CBOIL__i].name);\
        dumpname[CONCAT(CBOIL__OFFSET, num)] = '"';\
        CONCAT(CBOIL__OFFSET, num)++;\
        strCopy(dumpname, CONCAT(CBOIL__OFFSET, num), rules.nrps[CBOIL__i].name, CONCAT(CBOIL__NAMELEN, num));\
        CONCAT(CBOIL__OFFSET, num) += CONCAT(CBOIL__NAMELEN, num);\
        dumpname[CONCAT(CBOIL__OFFSET, num)] = '"';\
        dumpname[CONCAT(CBOIL__OFFSET, num)+1] = ':';\
        dumpname[CONCAT(CBOIL__OFFSET, num)+2] = ' ';\
        CONCAT(CBOIL__OFFSET, num) += 3;\
        int CONCAT(CBOIL__TMP, num) = 0;\
        writeRule(dumpname, (Rule*)rules.nrps[CBOIL__i].rule, &CONCAT(CBOIL__TMP, num), 0, CONCAT(CBOIL__OFFSET, num));\
        CONCAT(CBOIL__TMP, num) = 0;\
        CONCAT(CBOIL__OFFSET, num) += getRuleSize((Rule*)rules.nrps[CBOIL__i].rule, 0, true, &CONCAT(CBOIL__TMP, num));\
        dumpname[CONCAT(CBOIL__OFFSET, num)] = ',';\
        dumpname[CONCAT(CBOIL__OFFSET, num)+1] = '\n';\
        CONCAT(CBOIL__OFFSET, num) += 2;}\
        /* TODO: These index offsets are weird, possible bug*/\
    dumpname[CONCAT(CBOIL__OFFSET, num)-4] = '}';\
    dumpname[CONCAT(CBOIL__OFFSET, num)-3] = '\0';
#define dumpRules(dumpname, rules) _dumpRules(dumpname, rules, __COUNTER__);

#ifdef CSONDEP

#include <CSON.h>

JSONObject* ruleToJSON(const char* rule);
JSONObject* captureToJSON(Capture* cap);

#endif

#endif