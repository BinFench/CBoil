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