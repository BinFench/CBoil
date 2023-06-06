#include "test.def"
#include <CBoil.h>

RULES(
    number, capture("number", oneormore(charrange("0", "9"))),
    parens, sequence("(", subrule(expression), ")"),
    factor, capture("factor", firstof(subrule(number), subrule(parens))),
    term, capture("term", sequence(subrule(factor),
                        zeroormore(firstof(
                            sequence("*", subrule(factor)),
                            sequence("/", subrule(factor))
                        )))),
    expression, capture("expression", sequence(subrule(term),
                            zeroormore(firstof(
                                    sequence("+", subrule(term)),
                                    sequence("-", subrule(term))
                            )))),
    inputLine, sequence(subrule(expression), END)
)

int calculate_expression(Capture* expr);

int calculate_factor(Capture* factor) {
    if (CBoil.get(factor, "number"))
        return atoi(CBoil.get(factor, "number")->captures->firstCap->str);
    return calculate_expression(CBoil.get(factor, "expression")->captures);
}

int calculate_term(Capture* term) {
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

int calculate_expression(Capture* expr) {
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

int calculate(char* equation) {
    Capture* res = PARSE(inputLine, equation);
    int result = 0;
    if (res) {
        result = calculate_expression(res);
        CBoil.clear(res);
    }
    return result;
}