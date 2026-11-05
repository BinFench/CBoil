#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <CBoil.h>

// Exercises every rule primitive on its own, and every way the parser works out where the rule after
// it starts: after it matched, after it failed, and when it was skipped without being run.

static int checks = 0;
static int passed = 0;
static int tagCalls = 0;
static int untagCalls = 0;

static void* plain(Capture* cap) {
    return &checks;
}

static void* ta(Capture* cap) {
    return &checks;
}

static void* tag(Capture* cap) {
    tagCalls++;
    return malloc(1);
}

static void untag(void* structure) {
    untagCalls++;
    free(structure);
}

PARSER(
    RULES(rules,
        RULE(leaf, sequence("a")),
        RULE(subAfter, capture("r", sequence(subrule(leaf), "!", END))),
        RULE(subFail, capture("r", sequence(firstof(subrule(leaf), "q"), END))),
        RULE(subSkipped, capture("r", sequence(firstof("q", subrule(leaf)), "!", END))),
        RULE(subSkippedSeq, capture("r", sequence(firstof(sequence("n", subrule(leaf), "!"), "q"), END))),
        RULE(subMissing, capture("r", sequence(firstof(subrule(nosuch), "q"), END))),
        RULE(tagRoot, transform(tag, "a")),
        RULE(tagNested, transform(tag, sequence("a", "b"))),
        RULE(tagAfter, capture("r", sequence(transform(tag, "a"), "!", END))),
        RULE(tagNestedAfter, capture("r", sequence(transform(tag, sequence("a")), "!", END))),
        RULE(tagFail, capture("r", sequence(firstof(transform(tag, "z"), "q"), END))),
        RULE(tagSkipped, capture("r", sequence(firstof("q", transform(tag, "a")), "!", END))),
        RULE(tagNestedSkipped, capture("r", sequence(firstof("q", transform(tag, sequence("a"))), "!", END))),
        RULE(tagSkippedSeq, capture("r", sequence(firstof(sequence("n", transform(tag, "a"), "!"), "q"), END))),
        RULE(tagRolledBack, capture("r", firstof(sequence(transform(tag, "a"), "!"), "a"))),
        RULE(tagLookahead, capture("r", sequence(test(transform(tag, "a")), "a"))),
        RULE(plainRoot, transform(plain, "a")),
        RULE(prefixRoot, transform(ta, "a")),
        RULE(noFunction, transform(nosuch, "a")),
        RULE(noFunctionChild, capture("r", sequence(firstof(transform(nosuch, "a"), "q"), END)))
    ),
    TRANSFORM(plain),
    TRANSFORM(ta),
    TRANSFORM(tag, untag)
);

static char ANY_MATCH[] = "";

// All of the text a capture matched itself, across its Tokens
static bool textIs(Capture* cap, const char* text) {
    size_t len = 0;
    for (Token* token = cap->firstCap; token; token = token->next) {
        if (strncmp(text + len, token->str, strlen(token->str)) != 0) return false;
        len += strlen(token->str);
    }
    return text[len] == '\0';
}

static void report(const char* label, const char* src, const char* want, Capture* res) {
    checks++;
    bool ok = want ? (res && (want == ANY_MATCH || textIs(res, want))) : !res;
    if (ok) passed++;
    else printf("CHECK %d ERROR: %s on \"%s\": expected %s\n", checks, label, src,
        !want ? "no match" : want == ANY_MATCH ? "a match" : want);
}

static void checkRule(const char* label, const char* rule, char* src, const char* want) {
    CBOIL_INIT();
    Capture* res = CBoil.parseRule(rule, src);
    report(label, src, want, res);
    if (res) CBoil.clear(res);
}

static void checkNamed(const char* name, char* src, const char* want) {
    CBOIL_INIT();
    Capture* res = CBoil.parse(&rules, name, src);
    report(name, src, want, res);
    if (res) CBoil.clear(res);
}

static void assertThat(const char* label, bool ok) {
    checks++;
    if (ok) passed++;
    else printf("CHECK %d ERROR: %s\n", checks, label);
}

#define CHECK(rule, src, want) checkRule(#rule, capture("r", rule), src, want)
#define NO_MATCH NULL

// X must match the text m and must not match at "q".  Every shape ends in END, so a match
// proves the whole input was consumed and that the rule after X was read from the right place.
#define WHOLE(rule) capture("r", sequence(rule, END))
#define SKIPPED_(label, X) \
    checkRule(label " skipped by FIRSTOF", WHOLE(sequence(firstof("q", X), "!")), "q!", ANY_MATCH); \
    checkRule(label " skipped by SEQUENCE", WHOLE(firstof(sequence("n", X, "!"), "q")), "q", ANY_MATCH); \
    checkRule("OPTIONAL " label " skipped", WHOLE(sequence(firstof("q", optional(X)), "!")), "q!", ANY_MATCH); \
    checkRule("ONEORMORE " label " skipped", WHOLE(sequence(firstof("q", oneormore(X)), "!")), "q!", ANY_MATCH); \
    checkRule("TEST " label " skipped", WHOLE(sequence(firstof("q", test(X)), "!")), "q!", ANY_MATCH); \
    checkRule("TESTNOT " label " skipped", WHOLE(sequence(firstof("q", testnot(X)), "!")), "q!", ANY_MATCH); \
    checkRule("CAPTURE " label " skipped", WHOLE(sequence(firstof("q", capture("c", X)), "!")), "q!", ANY_MATCH);
#define FAILS_(label, X) \
    checkRule(label " fails, next alternative", WHOLE(firstof(X, "q")), "q", ANY_MATCH); \
    checkRule(label " fails, sequence fails", WHOLE(sequence(X, "!")), "q", NO_MATCH); \
    checkRule("TESTNOT " label " at a failure", WHOLE(sequence(testnot(X), "q")), "q", ANY_MATCH);
#define MATCHES_(label, X, m) \
    checkRule(label " then a rule", WHOLE(sequence(X, "!")), m "!", ANY_MATCH); \
    checkRule("OPTIONAL " label " then a rule", WHOLE(sequence(optional(X), "!")), m "!", ANY_MATCH); \
    checkRule("CAPTURE " label " then a rule", WHOLE(sequence(capture("c", X), "!")), m "!", ANY_MATCH); \
    checkRule("TEST " label " then a rule", WHOLE(sequence(test(X), m "!")), m "!", ANY_MATCH);
#define REPEATS_(label, X, m) \
    checkRule("ONEORMORE " label " then a rule", WHOLE(sequence(oneormore(X), "!")), m "!", ANY_MATCH);
#define PRIMITIVE(X, m) MATCHES_(#X, X, m) REPEATS_(#X, X, m) SKIPPED_(#X, X) FAILS_(#X, X)
#define CANNOT_FAIL(X, m) MATCHES_(#X, X, m) REPEATS_(#X, X, m) SKIPPED_(#X, X)

int main() {
    CBOIL_INIT();

    // Where the next rule starts, for every primitive with a string child and with a nested child
    PRIMITIVE(anyof("a"), "a")
    PRIMITIVE(anyof("x", "a"), "a")
    PRIMITIVE(anyof(charrange("a", "a")), "a")
    PRIMITIVE(anyof(charrange("x", "x"), charrange("a", "a")), "a")
    PRIMITIVE(anyof("x", charrange("a", "a")), "a")
    PRIMITIVE(capture("k", "a"), "a")
    PRIMITIVE(capture("k", charrange("a", "a")), "a")
    PRIMITIVE(capture("k", sequence("a", capture("j", "b"))), "ab")
    PRIMITIVE(charrange("a", "a"), "a")
    PRIMITIVE(firstof("x", "a"), "a")
    PRIMITIVE(firstof(charrange("x", "x"), charrange("a", "a")), "a")
    PRIMITIVE(ignorecase("A"), "a")
    PRIMITIVE(noneof("q", "!"), "a")
    PRIMITIVE(noneof(charrange("q", "q"), charrange("!", "!")), "a")
    PRIMITIVE(oneormore("a"), "aa")
    PRIMITIVE(oneormore(charrange("a", "a")), "aa")
    PRIMITIVE(sequence("a", "b"), "ab")
    PRIMITIVE(sequence(charrange("a", "a"), "b"), "ab")
    PRIMITIVE(test("!"), "")
    PRIMITIVE(test(charrange("!", "!")), "")
    PRIMITIVE(testnot("q"), "")
    PRIMITIVE(testnot(charrange("q", "q")), "")
    // Can not fail, so no FAILS
    CANNOT_FAIL(optional("a"), "a")
    CANNOT_FAIL(optional(charrange("a", "a")), "a")
    CANNOT_FAIL(zeroormore("a"), "aa")
    CANNOT_FAIL(zeroormore(charrange("a", "a")), "aa")
    // ANY repeated also consumes the rule after it, and END has nothing after it to match
    MATCHES_("ANY", ANY, "a") SKIPPED_("ANY", ANY)
    checkRule("ANY fails at end of input, next alternative", WHOLE(firstof(ANY, END)), "", ANY_MATCH);
    checkRule("ANY fails at end of input, next alternative", WHOLE(firstof(ANY, optional("x"))), "", ANY_MATCH);
    SKIPPED_("END", END)
    checkRule("END then a rule", WHOLE(sequence(END, optional("!"))), "", ANY_MATCH);
    checkRule("END fails, next alternative", WHOLE(firstof(END, "q")), "q", ANY_MATCH);

    // The same for the primitives that need a RuleSet
    checkNamed("subAfter", "a!", ANY_MATCH);
    checkNamed("subFail", "q", ANY_MATCH);
    checkNamed("subSkipped", "q!", ANY_MATCH);
    checkNamed("subSkippedSeq", "q", ANY_MATCH);
    checkNamed("subMissing", "q", ANY_MATCH);
    checkNamed("subMissing", "a", NO_MATCH);
    checkNamed("tagAfter", "a!", ANY_MATCH);
    checkNamed("tagNestedAfter", "a!", ANY_MATCH);
    checkNamed("tagFail", "q", ANY_MATCH);
    checkNamed("tagSkipped", "q!", ANY_MATCH);
    checkNamed("tagNestedSkipped", "q!", ANY_MATCH);
    checkNamed("tagSkippedSeq", "q", ANY_MATCH);
    checkNamed("noFunctionChild", "q", ANY_MATCH);
    checkNamed("noFunctionChild", "a", NO_MATCH);
    checkNamed("noFunction", "a", NO_MATCH);

    // ANY matches one char, and never the end of the input
    CHECK(ANY, "ab", "a");
    CHECK(ANY, "", NO_MATCH);
    CHECK(sequence(ANY, ANY), "a", NO_MATCH);

    // ANYOF matches one char, if any subrule matches here
    CHECK(anyof("a"), "abc", "a");
    CHECK(anyof("ab"), "abc", "a");
    CHECK(sequence(anyof("ab"), "bc"), "abc", "abc");
    CHECK(anyof("x", "ab"), "abc", "a");
    CHECK(anyof("x", "y"), "abc", NO_MATCH);
    CHECK(anyof(charrange("x", "z"), charrange("a", "c")), "b", "b");
    CHECK(anyof(charrange("x", "z"), "b"), "b", "b");
    CHECK(anyof("b", charrange("x", "z")), "b", "b");
    CHECK(anyof(charrange("x", "z"), charrange("p", "q")), "b", NO_MATCH);
    CHECK(sequence(anyof(charrange("a", "c")), "d"), "bd", "bd");
    CHECK(anyof("a"), "", NO_MATCH);
    CHECK(anyof(END), "", NO_MATCH);
    CHECK(anyof(optional("x")), "", NO_MATCH);
    CHECK(anyof(optional("x")), "a", "a");
    CHECK(anyof(capture("k", "a")), "a", "a");
    assertThat("ANYOF keeps no captures from its subrules", ({
        Capture* res = CBoil.parseRule(capture("r", anyof(capture("k", "a"))), "a");
        bool ok = res && !CBoil.get(res, "k");
        if (res) CBoil.clear(res);
        ok;
    }));

    // CAPTURE holds the text it matched
    CHECK(capture("k", "a"), "a", "");
    CHECK(sequence(capture("k", "a"), "b"), "ab", "b");
    CHECK(capture("k", "b"), "a", NO_MATCH);
    CHECK(capture("k", sequence("a", "b")), "ac", NO_MATCH);
    assertThat("CAPTURE with no parent is returned", ({
        Capture* res = CBoil.parseRule(capture("k", "a"), "a");
        bool ok = res && strcmp(res->name, "k") == 0;
        if (res) CBoil.clear(res);
        ok;
    }));
    assertThat("A failed CAPTURE with no parent is not released twice", !CBoil.parseRule(sequence(capture("k", sequence("a", "b")), "c"), "ax"));
    assertThat("A failed CAPTURE with no parent after another is released", !CBoil.parseRule(sequence(capture("j", "a"), capture("k", sequence("b", "c"))), "abx"));
    assertThat("The last CAPTURE with no parent is returned", ({
        Capture* res = CBoil.parseRule(sequence(capture("j", "a"), capture("k", "b")), "ab");
        bool ok = res && strcmp(res->name, "k") == 0;
        if (res) CBoil.clear(res);
        ok;
    }));

    // CHARRANGE matches one char in the inclusive range, in either order
    CHECK(charrange("a", "c"), "a", "a");
    CHECK(charrange("a", "c"), "c", "c");
    CHECK(charrange("c", "a"), "b", "b");
    CHECK(charrange("b", "c"), "a", NO_MATCH);
    CHECK(charrange("a", "b"), "c", NO_MATCH);
    CHECK(charrange("a", "z"), "", NO_MATCH);
    CHECK(charrange("\x80", "z"), "", NO_MATCH);

    // END matches only at the end of the input, and consumes nothing
    CHECK(END, "", "");
    CHECK(END, "a", NO_MATCH);
    CHECK(sequence("a", END), "a", "a");
    CHECK(sequence("a", END), "ab", NO_MATCH);
    CHECK(sequence(END, END), "", "");

    // FIRSTOF takes the first alternative that matches, in order
    CHECK(firstof("a", "ab"), "ab", "a");
    CHECK(firstof("ab", "a"), "ab", "ab");
    CHECK(firstof("x", "y"), "a", NO_MATCH);
    CHECK(firstof(charrange("x", "z"), "a"), "a", "a");
    CHECK(firstof(sequence("a", "x"), sequence("a", "y"), sequence("a", "z")), "az", "az");

    // IGNORECASE matches a string in any case, and only letters ignore case
    CHECK(ignorecase("aBc"), "AbC", "AbC");
    CHECK(ignorecase("abc"), "abd", NO_MATCH);
    CHECK(ignorecase("abc"), "ab", NO_MATCH);
    CHECK(ignorecase("a1"), "A1", "A1");
    CHECK(ignorecase("a1"), "A2", NO_MATCH);
    CHECK(ignorecase("@"), "`", NO_MATCH);
    CHECK(ignorecase("1"), "a", NO_MATCH);
    CHECK(ignorecase("a"), "1", NO_MATCH);
    CHECK(ignorecase("["), "{", NO_MATCH);
    CHECK(sequence(ignorecase("a"), "b"), "Ab", "Ab");

    // NONEOF matches one char, if no subrule matches here, and never the end of the input
    CHECK(noneof("x"), "abc", "a");
    CHECK(noneof("x", "y"), "abc", "a");
    CHECK(noneof("a"), "abc", NO_MATCH);
    CHECK(noneof("x", "a"), "abc", NO_MATCH);
    CHECK(noneof("ab"), "ac", "a");
    CHECK(noneof("ab"), "ab", NO_MATCH);
    CHECK(noneof("x"), "", NO_MATCH);
    CHECK(noneof(charrange("x", "z")), "a", "a");
    CHECK(noneof(charrange("a", "c")), "a", NO_MATCH);
    CHECK(noneof(charrange("x", "z"), charrange("a", "c")), "a", NO_MATCH);
    CHECK(noneof(charrange("a", "c"), charrange("x", "z")), "a", NO_MATCH);
    CHECK(noneof(charrange("x", "z"), "a"), "a", NO_MATCH);
    CHECK(noneof(charrange("x", "z"), "b"), "a", "a");
    CHECK(sequence(noneof(charrange("x", "z")), "b"), "ab", "ab");
    CHECK(sequence(noneof("x"), "b"), "ab", "ab");
    CHECK(noneof(capture("k", "x")), "a", "a");
    CHECK(noneof(capture("k", "a")), "a", NO_MATCH);

    // ONEORMORE is greedy, needs one match, and stops when a match consumes nothing
    CHECK(oneormore("a"), "aaab", "aaa");
    CHECK(oneormore("a"), "b", NO_MATCH);
    CHECK(oneormore("ab"), "ababa", "abab");
    CHECK(oneormore(charrange("a", "b")), "abc", "ab");
    CHECK(oneormore(sequence("a", "b")), "abac", "ab");
    CHECK(oneormore(optional("x")), "a", "");
    CHECK(oneormore(zeroormore("x")), "xxa", "xx");
    CHECK(oneormore(test("a")), "a", "");
    CHECK(oneormore(testnot("x")), "a", "");
    CHECK(oneormore(END), "", "");
    CHECK(sequence(oneormore(optional("x")), "a"), "a", "a");
    CHECK(oneormore(capture("k", optional("x"))), "a", "");

    assertThat("ONEORMORE with no parent returns the last CAPTURE", ({
        Capture* res = CBoil.parseRule(oneormore(capture("k", charrange("a", "c"))), "abc");
        bool ok = res && strcmp(res->name, "k") == 0 && textIs(res, "c");
        if (res) CBoil.clear(res);
        ok;
    }));

    // OPTIONAL always matches, and consumes only on a match
    CHECK(optional("a"), "a", "a");
    CHECK(optional("a"), "b", "");
    CHECK(optional("ab"), "ac", "");
    CHECK(optional(sequence("a", "b")), "ac", "");
    CHECK(sequence(optional("a"), "b"), "ab", "ab");
    CHECK(sequence(optional("a"), "b"), "b", "b");

    // SEQUENCE needs every rule to match, in order
    CHECK(sequence("a", "b", "c"), "abc", "abc");
    CHECK(sequence("a", "b", "c"), "abd", NO_MATCH);
    CHECK(sequence("a", "b", "c"), "ab", NO_MATCH);
    CHECK(sequence(sequence("a", "b"), sequence("c", "d")), "abcd", "abcd");

    // Strings match exactly
    CHECK("abc", "abc", "abc");
    CHECK("abc", "abd", NO_MATCH);
    CHECK("abc", "ab", NO_MATCH);
    CHECK("abc", "", NO_MATCH);

    // TEST matches if the subrule does, consumes nothing, and keeps nothing
    CHECK(test("a"), "a", "");
    CHECK(test("a"), "b", NO_MATCH);
    CHECK(test(charrange("a", "b")), "a", "");
    CHECK(test(charrange("a", "b")), "c", NO_MATCH);
    CHECK(sequence(test("a"), "ab"), "ab", "ab");
    CHECK(sequence(test(charrange("a", "b")), "ab"), "ab", "ab");
    CHECK(sequence(test(charrange("x", "z")), "ab"), "ab", NO_MATCH);
    CHECK(sequence(test(sequence("a", "b")), "ab"), "ab", "ab");
    CHECK(sequence(test(sequence("a", "x")), "ab"), "ab", NO_MATCH);
    CHECK(sequence(test(capture("k", "a")), "a"), "a", "a");
    CHECK(sequence(test(capture("k", charrange("a", "a"))), "a"), "a", "a");
    assertThat("TEST keeps no captures", ({
        Capture* res = CBoil.parseRule(capture("r", sequence(test(capture("k", "a")), "a")), "a");
        bool ok = res && !CBoil.get(res, "k");
        if (res) CBoil.clear(res);
        ok;
    }));
    assertThat("TEST with no parent keeps no captures", !CBoil.parseRule(sequence(test(capture("k", "a")), "a"), "a"));

    // TESTNOT matches if the subrule does not, consumes nothing, and keeps nothing
    CHECK(testnot("a"), "b", "");
    CHECK(testnot("a"), "a", NO_MATCH);
    CHECK(testnot(charrange("a", "b")), "c", "");
    CHECK(testnot(charrange("a", "b")), "a", NO_MATCH);
    CHECK(sequence(testnot("x"), "ab"), "ab", "ab");
    CHECK(sequence(testnot("a"), "ab"), "ab", NO_MATCH);
    CHECK(sequence(testnot(charrange("x", "z")), "ab"), "ab", "ab");
    CHECK(sequence(testnot(charrange("x", "z")), "ab"), "b", NO_MATCH);
    CHECK(sequence(testnot(charrange("a", "b")), "ab"), "ab", NO_MATCH);
    CHECK(sequence(testnot(sequence("a", "x")), "ab"), "ab", "ab");
    CHECK(sequence(testnot(sequence("a", "b")), "ab"), "ab", NO_MATCH);
    CHECK(sequence(testnot(capture("k", "a")), "a"), "a", NO_MATCH);
    CHECK(sequence(testnot(capture("k", "x")), "a"), "a", "a");
    CHECK(testnot(END), "a", "");
    CHECK(testnot(END), "", NO_MATCH);

    // TRANSFORM captures like CAPTURE and hands the Capture to its function
    tagCalls = 0; untagCalls = 0;
    checkNamed("tagRoot", "a", "a");
    assertThat("TRANSFORM calls its function and its cleanup once", tagCalls == 1 && untagCalls == 1);
    tagCalls = 0; untagCalls = 0;
    checkNamed("tagNested", "ab", "ab");
    assertThat("TRANSFORM of a nested rule calls its function and its cleanup once", tagCalls == 1 && untagCalls == 1);
    tagCalls = 0; untagCalls = 0;
    checkNamed("tagNested", "ac", NO_MATCH);
    assertThat("TRANSFORM does not call its function when it does not match", tagCalls == 0 && untagCalls == 0);
    tagCalls = 0; untagCalls = 0;
    checkNamed("tagRolledBack", "a", "a");
    assertThat("A rolled back TRANSFORM runs its cleanup", tagCalls == 1 && untagCalls == 1);
    tagCalls = 0; untagCalls = 0;
    checkNamed("tagLookahead", "a", "a");
    assertThat("A TRANSFORM within TEST runs its cleanup", tagCalls == 1 && untagCalls == 1);
    tagCalls = 0; untagCalls = 0;
    checkNamed("plainRoot", "a", "a");
    assertThat("TRANSFORM with no cleanup runs none", untagCalls == 0);
    checkNamed("prefixRoot", "a", "a");
    assertThat("TRANSFORM does not run the cleanup of a longer name", untagCalls == 0);
    assertThat("TRANSFORM with no RuleSet does not match", !CBoil.parseRule(transform(tag, "a"), "a"));
    assertThat("TRANSFORM with no RuleSet, nested, does not match", !CBoil.parseRule(transform(tag, sequence("a")), "a"));

    // SUBRULE with nothing to find
    assertThat("SUBRULE with no RuleSet does not match", !CBoil.parseRule(capture("r", subrule(leaf)), "a"));

    // get: these keys all hash to the last slot of every table up to 32 entries, so they probe and wrap around
    assertThat("get finds colliding keys", ({
        Capture* res = CBoil.parseRule(capture("r", sequence(
            capture("ai", "a"), capture("cw", "b"), capture("dv", "c"), capture("ee", "d"),
            capture("gc", "e"), capture("hr", "f"), capture("ia", "g")
        )), "abcdefg");
        bool ok = res && res->numKVs == 7 && CBoil.get(res, "ai") && CBoil.get(res, "cw") && CBoil.get(res, "dv")
            && CBoil.get(res, "ee") && CBoil.get(res, "gc") && CBoil.get(res, "hr") && CBoil.get(res, "ia")
            && !CBoil.get(res, "nosuch");
        if (res) CBoil.clear(res);
        ok;
    }));
    assertThat("get finds the same key again after collisions", ({
        Capture* res = CBoil.parseRule(capture("r", sequence(capture("ai", "a"), capture("cw", "b"), capture("ai", "c"), capture("cw", "d"))), "abcd");
        bool ok = res && res->numKVs == 2 && CBoil.get(res, "ai")->matches == 2 && CBoil.get(res, "cw")->matches == 2;
        if (res) CBoil.clear(res);
        ok;
    }));
    // Growing the table moves these rolled back keys ahead of the ones that stay, so removing them must not lose those
    assertThat("get finds colliding keys that survive a rollback", ({
        Capture* res = CBoil.parseRule(capture("r", sequence(
            capture("ai", "a"), capture("cw", "b"),
            firstof(sequence(capture("dv", "c"), capture("ee", "d"), capture("gc", "e"), capture("hr", "f"), capture("ia", "g"), "!"), "cdefg")
        )), "abcdefg");
        bool ok = res && res->numKVs == 2 && CBoil.get(res, "ai") && CBoil.get(res, "cw") && !CBoil.get(res, "dv") && !CBoil.get(res, "ia");
        if (res) CBoil.clear(res);
        ok;
    }));

    // The API with nothing to work on
    assertThat("parse with an unknown rule does not match", !CBoil.parse(&rules, "nosuch", "a"));
    assertThat("parse with no RuleSet does not match", !CBoil.parse(NULL, "leaf", "a"));
    assertThat("get with no Capture finds nothing", !CBoil.get(NULL, "k"));
    CBoil.clear(NULL);
    assertThat("clear with no Capture does nothing", true);

    printf("TESTS PASSED %d/%d\n", passed, checks);
    return 0;
}
