#!/usr/bin/python

import sys

def mov_end(l, n):
    for i in reversed(xrange(0, n, 1)):
        l[n+i]=l[i]

def wmerge(l1, n1, l2, n2): #weird merge
    if n1+n2 != len(l1):
        return

    #put l1 at the end of the array
    mov_end(l1, n2)

    i=0
    j=n2
    k=0

    while j!=len(l1) and k!=n2:
        print "i=%d, k=%d" % (j,k)
        if l1[j]<l2[k]:
            l1[i]=l1[j]
            j=j+1
        elif l1[j]>=l2[k]:
            l1[i]=l2[k]
            k=k+1

        i=i+1
    
    if j==len(l1):
        l1[i]=l2[k]

    return l1

def main(argv):
    print "Sort a stack (biggest on top) using only two stacks."
    data1 = [2, 4, 5, 7, 11, 13, 19, 0, 0, 0, 0, 0, 0, 0]
    data2 = [1, 2, 3, 4, 5, 15, 20]

    res = wmerge(data1,7,data2,7)
    print res

if __name__ == '__main__':
    main(sys.argv[1:])
