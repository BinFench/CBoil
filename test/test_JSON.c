#include <stdlib.h>
#include <string.h>

#include "test.h"
#include <CSON.h>
#include <CBoil.h>
#include <CBoil/printutils.h>

int main() {
    TESTS(
        TEST(
            Capture* res = CBoil.parse(&calculator, "inputLine", "1");
            ASSERT(res, "Failed to match\n");
            ASSERT(strcmp(res->name, "expression") == 0, "Expected capture named 'expression', got: %s\n", res->name);
            CaptureKVList* terms = CBoil.get(res, "term");
            ASSERT(terms, "Expected 'term' in 'expression', but not found\n");
            Capture* term = terms->captures;
            CaptureKVList* factors = CBoil.get(term, "factor");
            ASSERT(factors, "Expected 'factor' in 'term', but not found\n");
            Capture* factor = factors->captures;
            CaptureKVList* numbers = CBoil.get(factor, "number");
            ASSERT(numbers, "Expected 'number' in 'factor', but not found\n");
            Capture* number = numbers->captures;
            ASSERT(atoi(number->firstCap->str) == 1, "Expected to match '1', but got %s\n", number->firstCap->str);
            JSONObject* cap = captureToJSON(res);
            ASSERT(cap, "Expected capture to represent as JSON\n");
            JSONObject* expression = CSON.getObject(cap, "expression");
            ASSERT(expression, "Expected 'expression' in top JSON\n");
            JSONObject* captures = CSON.getObject(expression, "captures");
            ASSERT(captures, "Expected 'captures' in 'expression'\n");
            JSONObject* trm = CSON.getObject(captures, "term");
            ASSERT(trm, "Expected 'term' in 'captures'\n");
            captures = CSON.getObject(trm, "captures");
            ASSERT(captures, "Expected 'captures' in 'term'\n");
            JSONObject* fact = CSON.getObject(captures, "factor");
            ASSERT(fact, "Expected 'factor' in 'captures'\n");
            captures = CSON.getObject(fact, "captures");
            ASSERT(captures, "Expected 'captures' in 'factor'\n");
            JSONObject* num = CSON.getObject(captures, "number");
            ASSERT(num, "Expected 'number' in 'captures'\n");
            JSONArray* tokens = CSON.getArray(num, "tokens");
            ASSERT(tokens, "Expected 'tokens' in 'number'\n")
            ASSERT(strcmp(CSON.getStringFromArray(tokens, 0), "1") == 0, "Expected token: '1'\n");
            CSON.clear(cap);
            CBoil.clear(res);
        ),
        TEST(
            JSONObject* term;
            for (int i = 0; i < calculator.size; i++) {
                if (strcmp("term", calculator.nrps[i].name) == 0) {
                    term = ruleToJSON(calculator.nrps[i].rule);
                    break;
                }
            }

            ASSERT(term, "Expected rule to represent as JSON\n");
            JSONObject* rule = CSON.getObject(term, "rule");
            ASSERT(strcmp(CSON.getString(rule, "type"), "CAPTURE") == 0, "Expected term['type'] == 'CAPTURE'\n");
            ASSERT(CSON.getInt(rule, "numChildren") == 2, "Expected capture to have 2 children\n");

            JSONArray* termRules = CSON.getArray(rule, "rules");
            ASSERT(termRules, "Expected term['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termRules, 0), "term") == 0, "Expected term['rules'][0] == 'term'\n");

            JSONObject* termSeq = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termSeq, "Expected Rule in term['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");

            termRules = CSON.getArray(termSeq, "rules");
            ASSERT(termRules, "Expected termSeq['rules']\n");

            JSONObject* factor = CSON.getObjectFromArray(termRules, 0);
            ASSERT(factor, "Expected Rule in termSeq['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            JSONArray* factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");

            JSONObject* termOpt = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termOpt, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(termOpt, "type"), "OPTIONAL") == 0, "Expected factor['type'] == 'OPTIONAL'\n");
            ASSERT(CSON.getInt(termOpt, "numChildren") == 1, "Expected optional to have 1 child\n");

            termRules = CSON.getArray(termOpt, "rules");
            ASSERT(termRules, "Expected termOpt['rules']\n");

            JSONObject* termOOM = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termOOM, "Expected Rule in termOpt['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termOOM, "type"), "ONEORMORE") == 0, "Expected termOpt['type'] == 'ONEORMORE'\n");
            ASSERT(CSON.getInt(termOOM, "numChildren") == 1, "Expected oneormore to have 1 child\n");

            termRules = CSON.getArray(termOOM, "rules");
            ASSERT(termRules, "Expected termOOM['rules']\n");

            JSONObject* termFO = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termFO, "Expected Rule in termOOM['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termFO, "type"), "FIRSTOF") == 0, "Expected termFO['type'] == 'FIRSTOF'\n");
            ASSERT(CSON.getInt(termFO, "numChildren") == 2, "Expected firstof to have 2 children\n");

            termRules = CSON.getArray(termFO, "rules");
            ASSERT(termRules, "Expected termFO['rules']\n");

            termSeq = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termSeq, "Expected Rule in term['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");
            
            JSONArray* termSeqRules = CSON.getArray(termSeq, "rules");
            ASSERT(termSeqRules, "Expected termSeq['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termSeqRules, 0), "*") == 0, "Expected termSeq['rules'][0] == '*'\n");

            factor = CSON.getObjectFromArray(termSeqRules, 1);
            ASSERT(factor, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");

            termSeq = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termSeq, "Expected Rule in term['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");
            
            termSeqRules = CSON.getArray(termSeq, "rules");
            ASSERT(termSeqRules, "Expected termSeq['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termSeqRules, 0), "/") == 0, "Expected termSeq['rules'][0] == '/'\n");

            factor = CSON.getObjectFromArray(termSeqRules, 1);
            ASSERT(factor, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");
            
            CSON.clear(term);
        ),
        TEST(
            dumpRules(dump, calculator);
            JSONObject* rules = CSON.parse(dump);
            ASSERT(rules, "Expected RuleSet to represent as JSON\n");
            
            JSONObject* term = CSON.getObject(rules, "term");
            ASSERT(term, "Expected rules['term']\n");
            ASSERT(strcmp(CSON.getString(term, "type"), "CAPTURE") == 0, "Expected term['type'] == 'CAPTURE'\n");
            ASSERT(CSON.getInt(term, "numChildren") == 2, "Expected capture to have 2 children\n");

            JSONArray* termRules = CSON.getArray(term, "rules");
            ASSERT(termRules, "Expected term['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termRules, 0), "term") == 0, "Expected term['rules'][0] == 'term'\n");

            JSONObject* termSeq = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termSeq, "Expected Rule in term['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");

            termRules = CSON.getArray(termSeq, "rules");
            ASSERT(termRules, "Expected termSeq['rules']\n");

            JSONObject* factor = CSON.getObjectFromArray(termRules, 0);
            ASSERT(factor, "Expected Rule in termSeq['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            JSONArray* factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");

            JSONObject* termOpt = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termOpt, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(termOpt, "type"), "OPTIONAL") == 0, "Expected factor['type'] == 'OPTIONAL'\n");
            ASSERT(CSON.getInt(termOpt, "numChildren") == 1, "Expected optional to have 1 child\n");

            termRules = CSON.getArray(termOpt, "rules");
            ASSERT(termRules, "Expected termOpt['rules']\n");

            JSONObject* termOOM = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termOOM, "Expected Rule in termOpt['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termOOM, "type"), "ONEORMORE") == 0, "Expected termOpt['type'] == 'ONEORMORE'\n");
            ASSERT(CSON.getInt(termOOM, "numChildren") == 1, "Expected oneormore to have 1 child\n");

            termRules = CSON.getArray(termOOM, "rules");
            ASSERT(termRules, "Expected termOOM['rules']\n");

            JSONObject* termFO = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termFO, "Expected Rule in termOOM['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termFO, "type"), "FIRSTOF") == 0, "Expected termFO['type'] == 'FIRSTOF'\n");
            ASSERT(CSON.getInt(termFO, "numChildren") == 2, "Expected firstof to have 2 children\n");

            termRules = CSON.getArray(termFO, "rules");
            ASSERT(termRules, "Expected termFO['rules']\n");

            termSeq = CSON.getObjectFromArray(termRules, 0);
            ASSERT(termSeq, "Expected Rule in term['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");
            
            JSONArray* termSeqRules = CSON.getArray(termSeq, "rules");
            ASSERT(termSeqRules, "Expected termSeq['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termSeqRules, 0), "*") == 0, "Expected termSeq['rules'][0] == '*'\n");

            factor = CSON.getObjectFromArray(termSeqRules, 1);
            ASSERT(factor, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");

            termSeq = CSON.getObjectFromArray(termRules, 1);
            ASSERT(termSeq, "Expected Rule in term['rules'][0]\n");
            ASSERT(strcmp(CSON.getString(termSeq, "type"), "SEQUENCE") == 0, "Expected termSeq['type'] == 'SEQUENCE'\n");
            ASSERT(CSON.getInt(termSeq, "numChildren") == 2, "Expected sequence to have 2 children\n");
            
            termSeqRules = CSON.getArray(termSeq, "rules");
            ASSERT(termSeqRules, "Expected termSeq['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(termSeqRules, 0), "/") == 0, "Expected termSeq['rules'][0] == '/'\n");

            factor = CSON.getObjectFromArray(termSeqRules, 1);
            ASSERT(factor, "Expected Rule in termSeq['rules'][1]\n");
            ASSERT(strcmp(CSON.getString(factor, "type"), "SUBRULE") == 0, "Expected factor['type'] == 'SUBRULE'\n");
            ASSERT(CSON.getInt(factor, "numChildren") == 1, "Expected subrule to have 1 child\n");

            factorRules = CSON.getArray(factor, "rules");
            ASSERT(factorRules, "Expected factor['rules']\n");
            ASSERT(strcmp(CSON.getStringFromArray(factorRules, 0), "factor") == 0, "Expected factor['rules'][0] == 'factor'\n");
            
            CSON.clear(term);
        )
    );
    return 0;
}