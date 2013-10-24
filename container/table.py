#! /usr/bin/python
alpha = "abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-=_+[]{};'|:,./<>?"
def randstr(strlen):
	import random
	strlen = abs(random.randint(1, strlen))
	clist = []
	for x in xrange(0, strlen):
		clist.append(random.choice(alpha))
	return "".join(clist)
import sys
argv = sys.argv
argc = len(argv)
n = 10 if argc < 2 else int(argv[1])
strlen = 10 if argc < 3 else int(argv[2])
table = {}
for x in xrange(0, n):
	key = randstr(strlen)
	while key in table:
		key = randstr(strlen)
	val = randstr(strlen)
	table[key] = val
	print "add", key, val
for key in table:
	print "assert", key, table[key]
print "info"
