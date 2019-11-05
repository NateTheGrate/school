#!/usr/bin/python3

import random
import string
import sys

a = open("a.txt", "w+")
b = open("b.txt", "w+")
c = open("c.txt", "w+")

string1 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)]) + "\n"
string2 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)]) + "\n"
string3 = ''.join([random.choice(string.ascii_lowercase) for n in range(10)])

a.write(string1)
b.write(string2)
c.write(string3 +  "\n")

sys.stdout.write(string1 + string2 + string3 + '\n')

one = random.randint(1, 42)
two = random.randint(1, 42)

sys.stdout.write(str(one) + '\n')
sys.stdout.write(str(two) + '\n')

sys.stdout.write(str(one + two) + '\n')

a.close()
b.close()
c.close()