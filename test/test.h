#include "test.def"
#include <CBoil.h>

int* precalc;

// Forward declarations
int calculate_expression(Capture* expr);

int calculate_factor(Capture* factor) {
    CBOIL_INIT();
    if (CBoil.get(factor, "number"))
        return atoi(CBoil.get(factor, "number")->captures->firstCap->str);
    return calculate_expression(CBoil.get(factor, "expression")->captures);
}

void* factor(Capture* factor) {
    *precalc = calculate_factor(factor);
    return precalc;
}

int calculate_term(Capture* term) {
    CBOIL_INIT();
    CaptureKVList* factors = CBoil.get(term, "factor");
    int result = 0;
    if (factors->matches > 0) {
        result = calculate_factor(factors->captures);
        Token* token = term->firstCap;
        for (int i = 1; i < factors->matches; i++) {
            int right = calculate_factor(factors->captures + i);
            if (token->str[0] == '*') result *= right;
            else result /= right;
            token = token->next;
            if (i != factors->matches - 1)
                while (term != token->capture) token = token->next;
        }
    }
    return result;
}

void* term(Capture* term) {
    *precalc = calculate_term(term);
    return precalc;
}

int calculate_expression(Capture* expr) {
    CBOIL_INIT();
    CaptureKVList* terms = CBoil.get(expr, "term");
    int result = 0;
    if (terms->matches > 0) {
        result = calculate_term(terms->captures);
        Token* token = expr->firstCap;
        for (int i = 1; i < terms->matches; i++) {
            int right = calculate_term(terms->captures + i);
            if (token->str[0] == '-') right *= -1;
            result += right;
            token = token->next;
            if (i != terms->matches - 1)
                while (expr != token->capture) token = token->next;
        }
    }
    return result;
}

void* expression(Capture* expression) {
    *precalc = calculate_expression(expression);
    return precalc;
}

PARSER(
    RULES(precalculator,
        RULE(number, capture("number", oneormore(charrange("0", "9")))),
        RULE(parens, sequence("(", subrule(expression), ")")),
        RULE(factor, transform(factor, firstof(subrule(number), subrule(parens)))),
        RULE(term, transform(term, sequence(subrule(factor),
                            zeroormore(firstof(
                                sequence("*", subrule(factor)),
                                sequence("/", subrule(factor))
                            ))))),
        RULE(expression, transform(expression, sequence(subrule(term),
                                zeroormore(firstof(
                                        sequence("+", subrule(term)),
                                        sequence("-", subrule(term))
                                ))))),
        RULE(inputLine, sequence(subrule(expression), END))
    ),
    TRANSFORM(factor),
    TRANSFORM(term),
    TRANSFORM(expression)
);

RULES(calculator,
    RULE(number, capture("number", oneormore(charrange("0", "9")))),
    RULE(parens, sequence("(", subrule(expression), ")")),
    RULE(factor, capture("factor", firstof(subrule(number), subrule(parens)))),
    RULE(term, capture("term", sequence(subrule(factor),
                        zeroormore(firstof(
                            sequence("*", subrule(factor)),
                            sequence("/", subrule(factor))
                        ))))),
    RULE(expression, capture("expression", sequence(subrule(term),
                            zeroormore(firstof(
                                    sequence("+", subrule(term)),
                                    sequence("-", subrule(term))
                            ))))),
    RULE(inputLine, sequence(subrule(expression), END))
);

int calculate(char* equation) {
    CBOIL_INIT();
    Capture* res = CBoil.parse(&calculator, "inputLine", equation);
    int result = 0;
    if (res) {
        result = calculate_expression(res);
        CBoil.clear(res);
    }
    return result;
}