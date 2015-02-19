#!/usr/bin/python

import sys

def robotways(pos, dst):
    if pos == dst:
        return 0
    if pos[0] == dst[0]:
        return 1

    elif pos[1] == dst[1]:
        return 1

    else:
        newposu = (pos[0], pos[1]+1)
        newposr = (pos[0]+1, pos[1])
        return robotways(newposu,dst) + robotways(newposr,dst) 



def main(argv):
    src = (int(argv[0]),int(argv[1]))
    dst = (int(argv[2]),int(argv[3]))

    result = (dst[0], dst[1], src[0], src[1], robotways(src,dst))

    print '1: the number of ways to get to %d,%d from %d,%d is %d' % result

if __name__ == '__main__':
    main(sys.argv[1:])
