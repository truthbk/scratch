#!/usr/bin/python

import sys


def flip(s): # strings are immutable in Python. So we can't do it the usual way.

    rev_s = list(s)
    for i in xrange(0,len(s)/2,1):
        aux = rev_s[i]
        rev_s[i] = rev_s[len(s)-1-i]
        rev_s[len(rev_s)-1-i]=aux

    return ''.join(rev_s)

def flip_words(s):
    words = s.split(" ")
    rev_words=[]
    for w in words:
        rev_words.append(flip(w))

    return " ".join(rev_words)



def main(argv):
    s = " ".join(argv)
    rev_s = flip(s)
    print flip_words(rev_s)

if __name__ == '__main__':
    main(sys.argv[1:])
