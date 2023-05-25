from macros import *
import os

os.mkdir("../CBoildef")

f = open("../CBoildef/ASCII.def", "w")
f.write(ASCII())
f.close()

f = open("../CBoildef/EVERY_SECOND.def", "w")
f.write(EVERY_SECOND(256))
f.close()

f = open("../CBoildef/MAP.def", "w")
f.write(MAP(256))
f.close()

f = open("../CBoildef/VARIADIC.def", "w")
f.write(VARIADIC())
f.close()