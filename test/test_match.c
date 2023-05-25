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
        ),
        TEST(
            Capture* res = parse(capture("pass", "pass"), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("pass", "pass"), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", ANY), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            ASSERT(res->firstCap->size == 1 && res->firstCap->str[0] == 'p', "Expected to match 'p'\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", ANY), "");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", anyof("pass")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", anyof("pass", "succeed")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", anyof("pass", "succeed")), "succeed");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", anyof("pass", "succeed")), "pas");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", charrange("a", "z")), "p");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", charrange("g", "z")), "f");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", END), "");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", END), "f");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", firstof("fail", "pass", "succeed")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", firstof("pass", "succeed")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", ignorecase("pAss")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("pass", ignorecase("pass")), "pAsS");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", ignorecase("pass")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", noneof("f", "l", "y")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", noneof("f", "l", "y")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", oneormore("pass")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("pass", oneormore("pass")), "passpass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("pass", oneormore("pass")), "passpassfail");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", oneormore("pass")), "failfail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", sequence("p", "a", "s", "s")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", sequence("f", "a", "i", "s")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", test("pass")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            ASSERT(res->firstCap == NULL, "Expected empty match");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", test("pass")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        ),
        TEST(
            Capture* res = parse(capture("pass", testnot("fail")), "pass");
            ASSERT(res != NULL, "Expected string to pass\n");
            ASSERT(res->firstCap == NULL, "Expected empty match");
            clear(res);
        ),
        TEST(
            Capture* res = parse(capture("fail", testnot("fail")), "fail");
            ASSERT(res == NULL, "Expected string to fail\n");
        )
    );
    return 0;
}