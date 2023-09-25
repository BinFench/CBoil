#include "../CBoil.h"

#ifndef CBOIL_PRINT_UTILS
#define CBOIL_PRINT_UTILS

void dumpCapture(Capture* capture);
void dumpRule(const char* name, const char* rule);
void dumpRuleSet(RuleSet* ruleSet);

#ifdef CSONDEP

#include <CSON.h>

JSONObject* ruleToJSON(const char* rule);
JSONObject* captureToJSON(Capture* cap);

#endif

#endif