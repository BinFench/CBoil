#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
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
            dumpCapture(res);
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parse(&calculator, "inputLine", "1+2");
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
            ASSERT(res->firstCap, "Expected token '+', not found\n");
            Token* first = res->firstCap;
            ASSERT(strcmp(first->str, "+") == 0, "Expected token '+', got '%s'\n", first->str);
            Capture* term2 = terms->captures + 1;
            CaptureKVList* factors2 = CBoil.get(term2, "factor");
            ASSERT(factors, "Expected 'factor' in 'term', but not found\n");
            Capture* factor2 = factors2->captures;
            CaptureKVList* numbers2 = CBoil.get(factor2, "number");
            ASSERT(numbers2, "Expected 'number' in 'factor', but not found\n");
            Capture* number2 = numbers2->captures;
            ASSERT(atoi(number2->firstCap->str) == 2, "Expected to match '1', but got %s\n", number->firstCap->str);
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", "pass"), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", "pass"), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", ANY), "pass");
            ASSERT(res, "Expected string to pass\n");
            ASSERT(res->firstCap->size == 1 && res->firstCap->str[0] == 'p', "Expected to match 'p'\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", ANY), "");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", anyof("pass")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", anyof("pass", "succeed")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", anyof("pass", "succeed")), "succeed");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", anyof("pass", "succeed")), "pas");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", charrange("a", "z")), "p");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", charrange("g", "z")), "f");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", END), "");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", END), "f");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", firstof("fail", "pass", "succeed")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", firstof("pass", "succeed")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", ignorecase("pAss")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", ignorecase("pass")), "pAsS");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", ignorecase("pass")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", noneof("f", "l", "y")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", noneof("f", "l", "y")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", oneormore("pass")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", oneormore("pass")), "passpass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", oneormore("pass")), "passpassfail");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", oneormore("pass")), "failfail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", sequence("p", "a", "s", "s")), "pass");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", sequence("f", "a", "i", "s")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", test("pass")), "pass");
            ASSERT(res, "Expected string to pass\n");
            ASSERT(!res->firstCap, "Expected empty match");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", test("pass")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", testnot("fail")), "pass");
            ASSERT(res, "Expected string to pass\n");
            ASSERT(!res->firstCap, "Expected empty match");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("fail", testnot("fail")), "fail");
            ASSERT(!res, "Expected string to fail\n");
        ),
        CALCULATE("3+1", 4),
        CALCULATE("(7*2+6)/10", 2),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", sequence(optional(sequence("+", "/", "-")), charrange("0", "9"))), "0");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", sequence(optional(sequence(subrule(kv), subrule(w))), zeroormore(sequence(",", subrule(w), subrule(kv), subrule(w))), ",")), ",");
            ASSERT(res, "Expected string to pass\n");
            ASSERT(res->numTokens == 1, "Expected single match\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", sequence("\"", zeroormore(sequence(testnot("\""), ANY)), "\"")), "\"Hello World!\"");
            ASSERT(res, "Expected string to pass\n");
            ASSERT(strcmp(res->firstCap->str, "\"Hello World!\"") == 0, "Expected token '\"Hello World!\"', got '%s'\n", res->firstCap->str);
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("pass", sequence(oneormore(charrange("0","9")), optional("."), zeroormore(charrange("0","9")))), "1");
            ASSERT(res, "Expected string to pass\n");
            CBoil.clear(res);
        )
    );
    return 0;
}