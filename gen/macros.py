import math

def PP_RSEQ_N(name, num):
    # Print n numbered args
    ret = "#define " + name + "_PP_RSEQ_N() " + str(num)
    for i in range(num):
        ret += "," + str(num-i-1)
    ret += "\n"
    return ret

def PP_ARG_N(name, num):
    # Concat number to name macro
    ret = "#define " + name + "_PP_ARG_N("
    for i in range(num):
        ret += "_" + str(i+1) + ","
    ret += "N,...) " + name + ("##N\n" if name != "" else "N\n")
    return ret

def PP_NARG_(name):
    return "#define " + name + "_PP_NARG_(...) " + name + "_PP_ARG_N(__VA_ARGS__)\n"

def NUMARGS(name):
    return "#define " + name + "_NUMARGS(...) " + name + "_PP_NARG_(__VA_ARGS__," + name + "_PP_RSEQ_N())\n"

def EVERY_SECOND(num):
    # Macro for returning every other variadic argument
    ret = NUMARGS("SECOND")
    ret += PP_NARG_("SECOND")
    ret += PP_ARG_N("SECOND", num)
    ret += PP_RSEQ_N("SECOND", num)
    ret += "#define SECOND0(args...)\n"
    for i in range(num):
        ret += "#define SECOND" + str(i+1) + "_(second,...) ,second __VA_OPT__(SECOND" + str(i) + "(__VA_ARGS__))\n"
        ret += "#define SECOND" + str(i+1) + "(first,...) SECOND" + str(i+1) + "_(__VA_ARGS__)\n"
    ret += "#define EVERY_SECOND(...) SECOND_NUMARGS(__VA_ARGS__)(__VA_ARGS__)\n"
    return ret

def ASCII():
    # Macro for uint8 to ASCII
    ret = NUMARGS("ASCII")
    ret += PP_NARG_("ASCII")
    ret += PP_ARG_N("ASCII", 256)
    ret += PP_RSEQ_N("ASCII", 256)
    for i in range(256):
        num = hex(i)[2:]
        if (len(num) != 2):
            num = "0" + num
        ret += "#define ASCII" + str(i) + " \"\\x" + num + "\"\n"
    ret += "#define ASCII(a) ASCII##a\n"
    return ret

def MAP(num):
    # Function for MAP macro on variadic args
    depth = math.ceil(math.log2(num))

    ret = "#define EVAL0(...) __VA_ARGS__\n"
    for i in range(depth-1):
        ret += "#define EVAL" + str(i+1) + "(...) EVAL" + str(i) + " (EVAL" + str(i) + " (__VA_ARGS__))\n"
    ret += "#define EVAL(...) EVAL" + str(depth-1) + " (EVAL" + str(depth-1) + " (__VA_ARGS__))\n"

    ret += """
#define MAP_END(...)
#define MAP_OUT
#define MAP_COMMA ,

#define MAP_GET_END2() 0, MAP_END
#define MAP_GET_END1(...) MAP_GET_END2
#define MAP_GET_END(...) MAP_GET_END1
#define MAP_NEXT0(test, next, ...) next MAP_OUT
#define MAP_NEXT1(test, next) MAP_NEXT0 (test, next, 0)
#define MAP_NEXT(test, next)  MAP_NEXT1 (MAP_GET_END test, next)

#define MAP0(f, x, peek, ...) f(x) MAP_NEXT (peek, MAP1) (f, peek, __VA_ARGS__)
#define MAP1(f, x, peek, ...) f(x) MAP_NEXT (peek, MAP0) (f, peek, __VA_ARGS__)

#define MAP_LIST_NEXT1(test, next) MAP_NEXT0(test, MAP_COMMA next, 0)
#define MAP_LIST_NEXT(test, next)  MAP_LIST_NEXT1(MAP_GET_END test, next)

#define MAP_LIST0(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST1)(f, peek, __VA_ARGS__)
#define MAP_LIST1(f, x, peek, ...) f(x) MAP_LIST_NEXT(peek, MAP_LIST0)(f, peek, __VA_ARGS__)

#define MAP(f, ...) EVAL(MAP1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
#define MAP_LIST(f, ...) EVAL(MAP_LIST1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0))
"""

    return ret

def VARIADIC():
    # Macro for uint8 to ASCII
    ret = NUMARGS("")
    ret += PP_NARG_("")
    ret += PP_ARG_N("", 256)
    ret += PP_RSEQ_N("", 256)
    ret += "\n#define numargs(args...) _NUMARGS(args)\n"
    return ret