#!/usr/bin/python

import sys

def bst(arr, el):

    if arr[len(arr)/2] == el:
        return len(arr)/2
    elif len(arr) == 1:
        return -1

    if el<arr[len(arr)/2]:
        return bst(arr[0:len(arr)/2],el)
    else:
        return len(arr)/2 + 1 + bst(arr[len(arr)/2+1:],el)

def bst_abs(arr, el, s, e):

    if arr[s+(e-s)/2] == el:
        return s+(e-s)/2
    elif (e-s) == 0:
        return -1

    if el<arr[s+(e-s)/2]:
        return bst_abs(arr, el, s, s+(e-s)/2-1)
    else:
        return bst_abs(arr, el, s+(e-s)/2+1, e)

def main(argv):
    numlist = map(int,argv[1:])
    found = bst(numlist, int(argv[0]))
    if found<0:
        print 'element not found'
    else:
        print 'element found at %d ' % found

    found = bst_abs(numlist, int(argv[0]), 0, len(numlist)-1)
    if found<0:
        print 'element not found'
    else:
        print 'element found at %d ' % found

if __name__ == '__main__':
    main(sys.argv[1:])
