#!/usr/bin/env python3
"""Compares libCBoil against an independent reference matcher on random rules and inputs.

For each case it checks whether the rule matched, the Tokens and subcaptures of every Capture, that
Tokens are linked and sized correctly, and that clearing the result releases every allocation.

Usage: test_fuzz.py [cases] [seed]
"""
import ctypes
import random
import sys

libc = ctypes.CDLL(None)
libc.malloc.restype = ctypes.c_void_p
libc.malloc.argtypes = [ctypes.c_size_t]
libc.realloc.restype = ctypes.c_void_p
libc.realloc.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
libc.free.argtypes = [ctypes.c_void_p]
cboil = ctypes.CDLL("libCBoil.so")


class Capture(ctypes.Structure):
    pass


class Token(ctypes.Structure):
    pass


class KVList(ctypes.Structure):
    _fields_ = [("key", ctypes.c_char_p), ("matches", ctypes.c_uint16), ("captures", ctypes.POINTER(Capture))]


Token._fields_ = [("size", ctypes.c_uint16), ("str", ctypes.c_char_p), ("capture", ctypes.POINTER(Capture)),
                  ("next", ctypes.POINTER(Token)), ("prev", ctypes.POINTER(Token))]
Capture._fields_ = [("name", ctypes.c_char_p), ("numTokens", ctypes.c_uint16), ("numKVs", ctypes.c_uint16),
                    ("capacity", ctypes.c_uint16), ("subcaptures", ctypes.POINTER(KVList)),
                    ("next", ctypes.POINTER(Capture)), ("firstCap", ctypes.POINTER(Token)),
                    ("lastCap", ctypes.POINTER(Token)), ("structure", ctypes.c_void_p), ("ruleSet", ctypes.c_void_p)]

live = 0
FREE = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
MALLOC = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_size_t)
REALLOC = ctypes.CFUNCTYPE(ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t)


def _free(ptr):
    global live
    if ptr:
        live -= 1
    libc.free(ptr)


def _malloc(size):
    global live
    live += 1
    return libc.malloc(size)


def _realloc(ptr, size):
    global live
    if not ptr:
        live += 1
    return libc.realloc(ptr, size)


class MemLib(ctypes.Structure):
    _fields_ = [("free", FREE), ("malloc", MALLOC), ("realloc", REALLOC)]


memlib = MemLib(FREE(_free), MALLOC(_malloc), REALLOC(_realloc))
cboil.cboil_parseRule.restype = ctypes.POINTER(Capture)
cboil.cboil_parseRule.argtypes = [ctypes.POINTER(MemLib), ctypes.c_char_p, ctypes.c_char_p]
cboil.cboil_clear.argtypes = [ctypes.POINTER(MemLib), ctypes.POINTER(Capture)]

ANY, ANYOF, CAPTURE, CHARRANGE, END, FIRSTOF, IGNORECASE, NONEOF, ONEORMORE, OPTIONAL, _, SEQUENCE, TEST, TESTNOT = range(14)
STRINGS = [b"a", b"b", b"q", b"!", b"ab", b"ba", b"aa"]
NAMES = [b"j", b"k", b"m", b"n", b"p", b"r", b"s", b"t", b"u", b"v", b"w", b"x", b"y", b"z"]


def generate(rng, depth):
    """A rule is bytes for a plain string, or a tuple of its type and contents"""
    if depth == 0 or rng.random() < 0.2:
        kind = rng.choice(["string", "string", ANY, END, CHARRANGE, IGNORECASE])
        if kind == "string":
            return rng.choice(STRINGS)
        if kind == CHARRANGE:
            return (CHARRANGE, bytes(sorted(rng.choice([b"ab", b"aq", b"!q"]))) if rng.random() < 0.5 else rng.choice([b"ba", b"qa"]))
        if kind == IGNORECASE:
            return (IGNORECASE, rng.choice([b"A", b"a", b"aB", b"!"]))
        return (kind,)
    kind = rng.choice([ANYOF, CAPTURE, FIRSTOF, NONEOF, ONEORMORE, OPTIONAL, SEQUENCE, SEQUENCE, TEST, TESTNOT])
    if kind in (ANYOF, FIRSTOF, NONEOF, SEQUENCE):
        return (kind, [generate(rng, depth - 1) for _ in range(rng.randint(1, 4))])
    if kind == CAPTURE:
        return (CAPTURE, rng.choice(NAMES), generate(rng, depth - 1))
    return (kind, generate(rng, depth - 1))


def child_bytes(rule):
    return rule if isinstance(rule, bytes) else compile_rule(rule)


def compile_rule(rule):
    """Lays a rule out the way the macros in CBoil.def do"""
    kind = rule[0]
    if kind in (ANY, END):
        return bytes([0, kind, 0]) + bytes(8)
    if kind in (CHARRANGE, IGNORECASE):
        return bytes([0, kind, 1]) + rule[1] + b"\0"
    if kind in (ANYOF, FIRSTOF, NONEOF, SEQUENCE):
        return bytes([0, kind, len(rule[1])]) + b"".join(child_bytes(c) + b"\0" for c in rule[1])
    if kind == CAPTURE:
        return bytes([0, CAPTURE, 2]) + rule[1] + b"\0" + child_bytes(rule[2]) + b"\0"
    return bytes([0, kind, 1]) + child_bytes(rule[1]) + b"\0"


def ignorecase_matches(pattern, src):
    if len(src) < len(pattern):
        return False
    for p, c in zip(pattern, src):
        if chr(p).isalpha() and chr(c).isalpha():
            if chr(p).lower() != chr(c).lower():
                return False
        elif p != c:
            return False
    return True


def run(rule, src, pos):
    """Returns the position after a match, or None, and what it captured as text and (name, items)"""
    if isinstance(rule, bytes):
        return (pos + len(rule), [rule]) if src.startswith(rule, pos) else (None, [])
    kind = rule[0]
    if kind == ANY:
        return (pos + 1, [src[pos:pos + 1]]) if pos < len(src) else (None, [])
    if kind == END:
        return (pos, []) if pos == len(src) else (None, [])
    if kind == CHARRANGE:
        low, high = sorted(rule[1])
        return (pos + 1, [src[pos:pos + 1]]) if pos < len(src) and low <= src[pos] <= high else (None, [])
    if kind == IGNORECASE:
        size = len(rule[1])
        return (pos + size, [src[pos:pos + size]]) if ignorecase_matches(rule[1], src[pos:]) else (None, [])
    if kind in (ANYOF, NONEOF):
        found = any(run(c, src, pos)[0] is not None for c in rule[1])
        if pos < len(src) and found == (kind == ANYOF):
            return pos + 1, [src[pos:pos + 1]]
        return None, []
    if kind == CAPTURE:
        end, items = run(rule[2], src, pos)
        return (None, []) if end is None else (end, [(rule[1], items)])
    if kind == FIRSTOF:
        for child in rule[1]:
            end, items = run(child, src, pos)
            if end is not None:
                return end, items
        return None, []
    if kind == ONEORMORE:
        items = []
        current = pos
        matched = False
        while True:
            end, found = run(rule[1], src, current)
            if end is None:
                break
            matched = True
            items += found
            progressed = end != current
            current = end
            if not progressed:
                break
        return (current, items) if matched else (None, [])
    if kind == OPTIONAL:
        end, items = run(rule[1], src, pos)
        return (pos, []) if end is None else (end, items)
    if kind == SEQUENCE:
        items = []
        current = pos
        for child in rule[1]:
            current, found = run(child, src, current)
            if current is None:
                return None, []
            items += found
        return current, items
    if kind == TEST:
        return (pos, []) if run(rule[1], src, pos)[0] is not None else (None, [])
    if kind == TESTNOT:
        return (pos, []) if run(rule[1], src, pos)[0] is None else (None, [])
    raise AssertionError(kind)


def expected(items):
    """Text between subcaptures is one Token, and subcaptures are grouped by name"""
    tokens = []
    children = {}
    joined = False
    for item in items:
        if isinstance(item, bytes):
            if joined:
                tokens[-1] += item
            else:
                tokens.append(item)
            joined = True
        else:
            children.setdefault(item[0], []).append(expected(item[1]))
            joined = False
    return tokens, children


def actual(cap):
    tokens = []
    prev = None
    token = cap.firstCap
    while token:
        node = token.contents
        assert node.str is not None, "empty Token"
        assert node.size == len(node.str), "Token size does not match its text"
        assert ctypes.addressof(node.capture.contents) == ctypes.addressof(cap), "Token points at the wrong Capture"
        assert (ctypes.addressof(node.prev.contents) if node.prev else None) == prev, "Token prev is wrong"
        prev = ctypes.addressof(node)
        tokens.append(node.str)
        token = node.next
    assert (ctypes.addressof(cap.lastCap.contents) if cap.lastCap else None) == prev, "lastCap is not the last Token"
    assert cap.numTokens == len(tokens), "numTokens does not match the Tokens"

    children = {}
    for i in range(cap.capacity if cap.numKVs else 0):
        entry = cap.subcaptures[i]
        if entry.key:
            children[entry.key] = [actual(entry.captures[j]) for j in range(entry.matches)]
    assert len(children) == cap.numKVs, "numKVs does not match the keys"
    for key in children:
        found = cboil.cboil_get(ctypes.byref(cap), key)
        assert found, "get can not find a key"
    return tokens, children


cboil.cboil_get.restype = ctypes.c_void_p
cboil.cboil_get.argtypes = [ctypes.c_void_p, ctypes.c_char_p]


def main():
    cases = int(sys.argv[1]) if len(sys.argv) > 1 else 20000
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1
    rng = random.Random(seed)
    failures = 0
    matches = 0
    for case in range(cases):
        rule = (CAPTURE, b"root", generate(rng, rng.randint(1, 5)))
        src = bytes(rng.choice(b"abqA!") for _ in range(rng.randint(0, 8)))
        rule_bytes = compile_rule(rule)
        end, items = run(rule[2], src, 0)
        want = expected(items) if end is not None else None

        global live
        live = 0
        res = cboil.cboil_parseRule(ctypes.byref(memlib), rule_bytes, src)
        problem = None
        try:
            got = actual(res.contents) if res else None
            if got != want:
                problem = f"expected {want}, got {got}"
        except AssertionError as error:
            problem = str(error)
        if res:
            matches += 1
            cboil.cboil_clear(ctypes.byref(memlib), res)
        if live != 0 and not problem:
            problem = f"{live} allocations left after clear"
        if problem:
            failures += 1
            if failures <= 10:
                print(f"CASE {case} ERROR: {rule!r} on {src!r}: {problem}")
    print(f"TESTS PASSED {cases - failures}/{cases} ({matches} matched, seed {seed})")


main()
