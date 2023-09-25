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
        )
    );
    return 0;
}