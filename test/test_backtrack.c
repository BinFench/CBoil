#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include <CBoil.h>

// Mirrors a grammar where one alternative shares a prefix with the next, and nested captures fail late
PARSER(
    RULES(fallback,
        RULE(item, capture("item", firstof(subrule(range), subrule(text)))),
        RULE(range, capture("range", sequence(capture("start", subrule(ch)), "-", capture("end", subrule(ch))))),
        RULE(text, capture("text", oneormore(charrange("a", "z")))),
        RULE(ch, capture("ch", charrange("a", "z")))
    )
);

static bool textIs(Capture* cap, const char* text) {
    return cap->firstCap && cap->firstCap->str && strcmp(cap->firstCap->str, text) == 0;
}

// Text can span several Tokens once a capture has children, and firstCap may not be set
static bool allTextIs(Capture* cap, const char* text) {
    Token* token = cap->lastCap;
    while (token && token->prev) token = token->prev;

    size_t len = 0;
    for (; token; token = token->next) {
        if (!token->str) continue;
        if (strncmp(text + len, token->str, strlen(token->str)) != 0) return false;
        len += strlen(token->str);
    }
    return text[len] == '\0';
}

static int matchesOf(Capture* cap, const char* name) {
    CaptureKVList* list = cboil_get(cap, name);
    return list ? list->matches : 0;
}

static bool childTextIs(Capture* cap, const char* name, const char* text) {
    CaptureKVList* list = cboil_get(cap, name);
    return list && textIs(list->captures, text);
}

int main() {
    CBOIL_INIT();
    TESTS(
        // FIRSTOF: a failed alternative must not consume input
        TEST(
            Capture* res = CBoil.parseRule(capture("r", firstof(sequence("x", "-"), "x")), "x");
            ASSERT(res, "Expected second alternative to match after first failed partway\n");
            ASSERT(textIs(res, "x"), "Expected text 'x', got '%s'\n", res->firstCap ? res->firstCap->str : "(none)");
            CBoil.clear(res);
        ),
        // FIRSTOF: a failed alternative must not leave its text behind
        TEST(
            Capture* res = CBoil.parseRule(capture("r", firstof(sequence("a", "b"), "ac")), "ac");
            ASSERT(res, "Failed to match\n");
            ASSERT(textIs(res, "ac"), "Expected text 'ac', got '%s'\n", res->firstCap ? res->firstCap->str : "(none)");
            CBoil.clear(res);
        ),
        // FIRSTOF: text matched before the FIRSTOF must survive a failed alternative
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence("p", firstof(sequence("a", "-"), "ab"))), "pab");
            ASSERT(res, "Failed to match\n");
            ASSERT(textIs(res, "pab"), "Expected text 'pab', got '%s'\n", res->firstCap ? res->firstCap->str : "(none)");
            CBoil.clear(res);
        ),
        // FIRSTOF: a failed alternative must not leave its captures behind
        TEST(
            Capture* res = CBoil.parseRule(capture("r", firstof(sequence(capture("a", "x"), "-"), capture("b", "x"))), "x");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "a") == 0, "Expected no 'a' from the failed alternative, got %d\n", matchesOf(res, "a"));
            ASSERT(matchesOf(res, "b") == 1, "Expected 1 'b', got %d\n", matchesOf(res, "b"));
            CBoil.clear(res);
        ),
        // FIRSTOF: matches of a key from before the FIRSTOF must survive, only the failed one is dropped
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(capture("a", "x"), firstof(sequence(capture("a", "y"), "-"), "y"))), "xy");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "a") == 1, "Expected 1 'a', got %d\n", matchesOf(res, "a"));
            ASSERT(childTextIs(res, "a", "x"), "Expected the surviving 'a' to be 'x'\n");
            CBoil.clear(res);
        ),
        // FIRSTOF: when every alternative fails, nothing matches and nothing leaks
        TEST(
            Capture* res = CBoil.parseRule(capture("r", firstof(sequence(capture("a", "x"), "-"), sequence(capture("b", "x"), "+"))), "x");
            ASSERT(!res, "Expected no match\n");
        ),
        // FIRSTOF through named rules, where the failed alternative holds nested captures
        TEST(
            Capture* res = CBoil.parse(&fallback, "item", "abc");
            ASSERT(res, "Expected 'text' after 'range' failed partway\n");
            ASSERT(matchesOf(res, "range") == 0, "Expected no 'range', got %d\n", matchesOf(res, "range"));
            ASSERT(childTextIs(res, "text", "abc"), "Expected 'text' to be 'abc'\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parse(&fallback, "item", "a-b");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "range") == 1, "Expected 1 'range', got %d\n", matchesOf(res, "range"));
            ASSERT(matchesOf(res, "text") == 0, "Expected no 'text', got %d\n", matchesOf(res, "text"));
            CBoil.clear(res);
        ),
        // ONEORMORE: a failed last iteration must not consume input
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(oneormore(sequence("a", ",")), "a")), "a,a,a");
            ASSERT(res, "Expected the failed iteration to give its 'a' back\n");
            ASSERT(textIs(res, "a,a,a"), "Expected text 'a,a,a', got '%s'\n", res->firstCap ? res->firstCap->str : "(none)");
            CBoil.clear(res);
        ),
        // ONEORMORE: a failed last iteration must not leave its captures behind
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(oneormore(sequence(capture("i", "a"), ",")), capture("z", "a"))), "a,a,a");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "i") == 2, "Expected 2 'i', got %d\n", matchesOf(res, "i"));
            ASSERT(matchesOf(res, "z") == 1, "Expected 1 'z', got %d\n", matchesOf(res, "z"));
            CBoil.clear(res);
        ),
        // ONEORMORE: still fails when no iteration matches, and clears the partial one
        TEST(
            Capture* res = CBoil.parseRule(capture("r", oneormore(sequence(capture("i", "a"), ","))), "a");
            ASSERT(!res, "Expected no match\n");
        ),
        // OPTIONAL: a failed subrule must not leave its text behind
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(optional(sequence("x", "-")), "xy")), "xy");
            ASSERT(res, "Failed to match\n");
            ASSERT(textIs(res, "xy"), "Expected text 'xy', got '%s'\n", res->firstCap ? res->firstCap->str : "(none)");
            CBoil.clear(res);
        ),
        // OPTIONAL: a failed subrule must not leave its captures behind
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(optional(sequence(capture("a", "x"), "-")), "x")), "x");
            ASSERT(res, "Failed to match\n");
            ASSERT(matchesOf(res, "a") == 0, "Expected no 'a' from the failed optional, got %d\n", matchesOf(res, "a"));
            CBoil.clear(res);
        ),
        // Rollback when the capture already holds Tokens from a child, keeping text from before and after
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(capture("a", "x"), "p", firstof(sequence("q", capture("b", "r"), "!"), "qr"), capture("c", "s"), "t")), "xpqrst");
            ASSERT(res, "Failed to match\n");
            ASSERT(allTextIs(res, "pqrt"), "Expected text 'pqrt' after the rollback\n");
            ASSERT(matchesOf(res, "b") == 0, "Expected no 'b', got %d\n", matchesOf(res, "b"));
            ASSERT(childTextIs(res, "c", "s"), "Expected 'c' to be 's'\n");
            CBoil.clear(res);
        ),
        // Every key rolled back, so the table itself must be released and be usable again
        TEST(
            Capture* res = CBoil.parseRule(capture("r", firstof(sequence(capture("a", "x"), capture("b", "y"), "!"), sequence(capture("c", "x"), capture("d", "y")))), "xy");
            ASSERT(res, "Failed to match\n");
            ASSERT(res->numKVs == 2, "Expected 2 keys, got %d\n", res->numKVs);
            ASSERT(childTextIs(res, "c", "x") && childTextIs(res, "d", "y"), "Expected 'c' and 'd' from the second alternative\n");
            CBoil.clear(res);
        ),
        // get: keys inserted before the table grew must still be found
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(capture("start", "a"), capture("end", "b"))), "ab");
            ASSERT(res, "Failed to match\n");
            ASSERT(childTextIs(res, "start", "a"), "Expected to find 'start' after the table grew\n");
            ASSERT(childTextIs(res, "end", "b"), "Expected to find 'end'\n");
            CBoil.clear(res);
        ),
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(
                capture("k0", "a"), capture("k1", "b"), capture("k2", "c"), capture("k3", "d"),
                capture("k4", "e"), capture("k5", "f"), capture("k6", "g"), capture("k7", "h"),
                capture("k8", "i"), capture("k9", "j"), capture("k10", "k"), capture("k11", "l")
            )), "abcdefghijkl");
            ASSERT(res, "Failed to match\n");
            ASSERT(res->numKVs == 12, "Expected 12 keys, got %d\n", res->numKVs);
            ASSERT(childTextIs(res, "k0", "a") && childTextIs(res, "k1", "b") && childTextIs(res, "k2", "c"), "Lost one of k0-k2\n");
            ASSERT(childTextIs(res, "k3", "d") && childTextIs(res, "k4", "e") && childTextIs(res, "k5", "f"), "Lost one of k3-k5\n");
            ASSERT(childTextIs(res, "k6", "g") && childTextIs(res, "k7", "h") && childTextIs(res, "k8", "i"), "Lost one of k6-k8\n");
            ASSERT(childTextIs(res, "k9", "j") && childTextIs(res, "k10", "k") && childTextIs(res, "k11", "l"), "Lost one of k9-k11\n");
            CBoil.clear(res);
        ),
        // get: keys that survive a rollback must still be found after the others are removed
        TEST(
            Capture* res = CBoil.parseRule(capture("r", sequence(
                capture("k1", "a"), capture("k2", "b"), capture("k3", "c"),
                firstof(sequence(capture("k4", "d"), capture("k5", "e"), "!"), "de"),
                capture("k6", "f")
            )), "abcdef");
            ASSERT(res, "Failed to match\n");
            ASSERT(res->numKVs == 4, "Expected 4 keys, got %d\n", res->numKVs);
            ASSERT(matchesOf(res, "k4") == 0 && matchesOf(res, "k5") == 0, "Expected k4 and k5 rolled back\n");
            ASSERT(childTextIs(res, "k1", "a") && childTextIs(res, "k2", "b"), "Lost k1 or k2\n");
            ASSERT(childTextIs(res, "k3", "c") && childTextIs(res, "k6", "f"), "Lost k3 or k6\n");
            ASSERT(allTextIs(res, "de"), "Expected only 'de' as text after the rollback\n");
            CBoil.clear(res);
        )
    );
    return 0;
}
