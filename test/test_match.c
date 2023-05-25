#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include <CBoil.h>
#include <CBoil/printutils.h>

int main() {
    TESTS(
        TEST(
            Capture* res = PARSE(inputLine, "1");
            ASSERT(res, "Failed to match\n");
            ASSERT(strcmp(res->name, "expression") == 0, "Expected capture named 'expression', got: %s\n", res->name);
            CaptureKVList* terms = get(res, "term");
            ASSERT(terms, "Expected 'term' in 'expression', but not found\n");
            Capture* term = terms->captures;
            CaptureKVList* factors = get(term, "factor");
            ASSERT(factors, "Expected 'factor' in 'term', but not found\n");
            Capture* factor = factors->captures;
            CaptureKVList* numbers = get(factor, "number");
            ASSERT(numbers, "Expected 'number' in 'factor', but not found\n");
            Capture* number = numbers->captures;
            ASSERT(atoi(number->firstCap->str) == 1, "Expected to match '1', but got %s\n", number->firstCap->str);
            clear(res);
        ),
        TEST(
            Capture* res = PARSE(inputLine, "1+2");
            ASSERT(res, "Failed to match\n");
            ASSERT(strcmp(res->name, "expression") == 0, "Expected capture named 'expression', got: %s\n", res->name);
            CaptureKVList* terms = get(res, "term");
            ASSERT(terms, "Expected 'term' in 'expression', but not found\n");
            Capture* term = terms->captures;
            CaptureKVList* factors = get(term, "factor");
            ASSERT(factors, "Expected 'factor' in 'term', but not found\n");
            Capture* factor = factors->captures;
            CaptureKVList* numbers = get(factor, "number");
            ASSERT(numbers, "Expected 'number' in 'factor', but not found\n");
            Capture* number = numbers->captures;
            ASSERT(atoi(number->firstCap->str) == 1, "Expected to match '1', but got %s\n", number->firstCap->str);
            ASSERT(res->firstCap, "Expected token '+', not found\n");
            Token* first = res->firstCap;
            ASSERT(strcmp(first->str, "+") == 0, "Expected token '+', got '%s'\n", first->str);
            Capture* term2 = terms->captures + 1;
            CaptureKVList* factors2 = get(term2, "factor");
            ASSERT(factors, "Expected 'factor' in 'term', but not found\n");
            Capture* factor2 = factors2->captures;
            CaptureKVList* numbers2 = get(factor2, "number");
            ASSERT(numbers2, "Expected 'number' in 'factor', but not found\n");
            Capture* number2 = numbers2->captures;
            ASSERT(atoi(number2->firstCap->str) == 2, "Expected to match '1', but got %s\n", number->firstCap->str);
            clear(res);
        )
    );
    return 0;
}