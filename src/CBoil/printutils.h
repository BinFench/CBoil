#include "../CBoil.h"

#ifndef CBOIL_PRINT_UTILS
#define CBOIL_PRINT_UTILS

int getCaptureSize(Capture* capture, int indent, bool singular);
int writeCapture(char* dump, Capture* capture, int indent, bool singular, int pos);

#define dumpCapture(name, capture) int CBOIL__DUMPSIZE = getCaptureSize(capture, 0, true); char name[CBOIL__DUMPSIZE + 3]; name[0] = '{'; writeCapture(dump, capture, 0, true, 1); name[CBOIL__DUMPSIZE + 1] = '}'; name[CBOIL__DUMPSIZE + 2] = '\0'

int strCopy(char* dump, int pos, const char* string, int len);

int getRuleSize(Rule* rule, int indent, bool singular, int* off);
int writeRule(char* dump, Rule* rule, int* off, int indent, int pos);

#define dumpRule(name, rname, rule) int CBOIL__OFFSET = 0; int CBOIL__NAMELEN = strlen(rname); int CBOIL__DUMPSIZE = CBOIL__NAMELEN + 5 + getRuleSize((Rule*)rule, 0, true, &CBOIL__OFFSET); char name[CBOIL__DUMPSIZE + 3]; name[0] = '{'; name[1] = '"'; strCopy(name, 2, rname, CBOIL__NAMELEN); name[CBOIL__NAMELEN+2] = '"'; name[CBOIL__NAMELEN+3] = ':'; name[CBOIL__NAMELEN+4] = ' '; CBOIL__OFFSET = 0; writeRule(name, (Rule*)rule, &CBOIL__OFFSET, 0, CBOIL__NAMELEN+5); name[CBOIL__DUMPSIZE] = '}'; name[CBOIL__DUMPSIZE+1] = '\0';

#ifdef CSONDEP

#include <CSON.h>

JSONObject* ruleToJSON(const char* rule);
JSONObject* captureToJSON(Capture* cap);

#endif

#endif