#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include <CBoil.h>

// A Capture's Tokens are the strings of text it matched itself, split wherever a subcapture
// occurred.  firstCap is the first string, lastCap is the last, and they are linked in order.
PARSER(
    RULES(tokens,
        RULE(call, capture("call", sequence(subrule(name), "(", subrule(name), ")"))),
        RULE(name, capture("name", oneormore(charrange("a", "z"))))
    )
);

static bool tokensAre(Capture* cap, int count, ...) {
    va_list expected;
    va_start(expected, count);
    bool same = cap && cap->numTokens == count;

    Token* prev = NULL;
    Token* token = cap ? cap->firstCap : NULL;
    for (int i = 0; same && i < count; i++) {
        const char* text = va_arg(expected, const char*);
        same = token && token->str && strcmp(token->str, text) == 0 && token->size == strlen(text)
            && token->prev == prev && token->capture == cap;
        prev = token;
        token = same ? token->next : NULL;
    }
    va_end(expected);

    return same && !token && cap->lastCap == prev;
}

static Capture* child(Capture* cap, const char* name, int index) {
    CaptureKVList* list = cboil_get(cap, name);
    return (list && index < list->matches) ? list->captures + index : NULL;
}

static int matchesOf(Capture* cap, const char* name) {
    CaptureKVList* list = cboil_get(cap, name);
    return list ? list->matches : 0;
}

int main() {
    CBOIL_INIT();
    TESTS(
        // One contiguous string: firstCap and lastCap are the same Token
        TEST(
            Capture* res = CBoil.parseRule(capture("r", "abc"), "abc");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "abc"), "Expected tokens [abc]\n");
            ASSERT(res->firstCap == res->lastCap, "Expected firstCap == lastCap\n");
            CBoil.clear(res);
        ),
        // Text with no subcapture between it stays one string, however it was matched
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", "b", oneormore(charrange("c", "e")), optional("f"))), "abcdef");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "abcdef"), "Expected tokens [abcdef]\n");
            CBoil.clear(res);
        ),
        // A capture with only subcaptures has no Tokens
        TEST(
            Capture* res = CBoil.parseRule(capture("r", capture("k", "a")), "a");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 0), "Expected no tokens in 'r'\n");
            ASSERT(!res->firstCap && !res->lastCap, "Expected firstCap and lastCap to be NULL\n");
            ASSERT(tokensAre(child(res, "k", 0), 1, "a"), "Expected tokens [a] in 'k'\n");
            CBoil.clear(res);
        ),
        // Text after a subcapture, with none before it, is still one contiguous string
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(capture("k", "a"), "de")), "ade");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "de"), "Expected tokens [de]\n");
            ASSERT(res->firstCap == res->lastCap, "Expected firstCap == lastCap\n");
            CBoil.clear(res);
        ),
        // Text before a subcapture, with none after it
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("de", capture("k", "a"))), "dea");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "de"), "Expected tokens [de]\n");
            ASSERT(res->firstCap == res->lastCap, "Expected firstCap == lastCap\n");
            CBoil.clear(res);
        ),
        // A subcapture splits the text, when it wraps a plain string
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", "x"), "b")), "axb");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "b"), "Expected tokens [a, b]\n");
            ASSERT(tokensAre(child(res, "k", 0), 1, "x"), "Expected tokens [x] in 'k'\n");
            CBoil.clear(res);
        ),
        // A subcapture splits the text, when it wraps a nested rule
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", charrange("x", "z")), "b")), "ayb");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "b"), "Expected tokens [a, b]\n");
            ASSERT(tokensAre(child(res, "k", 0), 1, "y"), "Expected tokens [y] in 'k'\n");
            CBoil.clear(res);
        ),
        // A subcapture splits the text, when it is reached through a named rule
        TEST(
            Capture* res = CBoil.parse(&tokens, "call", "f(x)");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "(", ")"), "Expected tokens [(, )]\n");
            ASSERT(tokensAre(child(res, "name", 0), 1, "f"), "Expected tokens [f] in name 0\n");
            ASSERT(tokensAre(child(res, "name", 1), 1, "x"), "Expected tokens [x] in name 1\n");
            CBoil.clear(res);
        ),
        // Each string between subcaptures is its own Token
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", "x"), "b", capture("k", "y"), "c")), "axbyc");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 3, "a", "b", "c"), "Expected tokens [a, b, c]\n");
            ASSERT(matchesOf(res, "k") == 2, "Expected 2 'k'\n");
            CBoil.clear(res);
        ),
        // Adjacent subcaptures have no string between them, so no empty Token
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", "x"), capture("j", charrange("y", "y")), "b")), "axyb");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "b"), "Expected tokens [a, b]\n");
            CBoil.clear(res);
        ),
        // A trailing subcapture leaves lastCap on the final string, not an empty Token
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", "x"), "b", capture("j", "y"))), "axby");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "b"), "Expected tokens [a, b]\n");
            ASSERT(strcmp(res->lastCap->str, "b") == 0, "Expected lastCap to be 'b'\n");
            CBoil.clear(res);
        ),
        // Text belongs only to the capture that directly matched it
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", sequence("b", capture("j", "c"), "d")), "e")), "abcde");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "e"), "Expected tokens [a, e] in 'r'\n");
            Capture* k = child(res, "k", 0);
            ASSERT(tokensAre(k, 2, "b", "d"), "Expected tokens [b, d] in 'k'\n");
            ASSERT(tokensAre(child(k, "j", 0), 1, "c"), "Expected tokens [c] in 'j'\n");
            CBoil.clear(res);
        ),
        // Only text matched within a capture is captured
        TEST(
            Capture* res = CBoil.parseRule(sequence("a", capture("k", "b"), "c"), "abc");
            ASSERT(res, "Failed to match\n");
            ASSERT(strcmp(res->name, "k") == 0, "Expected capture 'k'\n");
            ASSERT(tokensAre(res, 1, "b"), "Expected tokens [b]\n");
            CBoil.clear(res);
        ),
        // A rolled back subcapture never happened, so it does not split the text
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("p", firstof(sequence(capture("x", "q"), "!"), "q"), "r")), "pqr");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "pqr"), "Expected tokens [pqr]\n");
            ASSERT(matchesOf(res, "x") == 0, "Expected no 'x'\n");
            CBoil.clear(res);
        ),
        // A split from before the rollback is kept, the Tokens added after it are dropped
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", capture("k", "x"), firstof(sequence("b", capture("y", "c"), "d", "!"), "bcd"), "e")), "axbcde");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 2, "a", "bcde"), "Expected tokens [a, bcde]\n");
            CBoil.clear(res);
        ),
        // A failed ONEORMORE iteration drops only its own Tokens
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(oneormore(sequence("a", capture("i", "b"), "c")), "ab")), "abcabcab");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 3, "a", "ca", "cab"), "Expected tokens [a, ca, cab]\n");
            ASSERT(matchesOf(res, "i") == 2, "Expected 2 'i'\n");
            CBoil.clear(res);
        ),
        // A failed OPTIONAL does not split the text
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("a", optional(sequence(capture("x", "b"), "!")), "b")), "ab");
            ASSERT(res, "Failed to match\n");
            ASSERT(tokensAre(res, 1, "ab"), "Expected tokens [ab]\n");
            CBoil.clear(res);
        ),
        // Tokens still point at their Capture after sibling Captures are reallocated
        TEST(
            Capture* res = CBoil.parseRule(capture("r", oneormore(capture("i", sequence("a", capture("j", "b"), "c")))), "abcabcabcabcabcabc");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "i") == 6, "Expected 6 'i', got %d\n", matchesOf(res, "i"));
            ASSERT(tokensAre(child(res, "i", 0), 2, "a", "c"), "Expected tokens [a, c] in i 0\n");
            ASSERT(tokensAre(child(res, "i", 3), 2, "a", "c"), "Expected tokens [a, c] in i 3\n");
            ASSERT(tokensAre(child(res, "i", 5), 2, "a", "c"), "Expected tokens [a, c] in i 5\n");
            CBoil.clear(res);
        ),
        // Walking Tokens by owner works for terms that were reallocated by later terms
        CALCULATE("1*2*3+4", 10),
        CALCULATE("2*3*4+5*6*7+8*9*2", 378)
    );
    return 0;
}
