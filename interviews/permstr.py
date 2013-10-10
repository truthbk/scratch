#!/usr/bin/python

import sys

def permutations(x):

    if len(x) == 1:
        return [x]

    res = []
    baseperms = permutations(x[1:])
    for p in baseperms:
        for i in xrange(0, len(p)+1, 1):
            n_str = p[0:i] + x[0] + p[i:]
            res.append(n_str)

    return res




def main(argv):
    myperms = permutations(argv[0])

    for p in myperms:
        print p

if __name__ == '__main__':
    main(sys.argv[1:])
