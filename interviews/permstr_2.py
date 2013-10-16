#!/usr/bin/python

import sys

class Permutator(object):
    def __init__(self, astr):
        self._str=astr
        self._positions = []
        for c in self._str:
            self._positions.append(-1)

    def index_used(self, idx):
        for i in xrange(0, len(self._positions),1): 
            if self._positions[i]==idx:
                return True

        return False

    def permute(self, idx):
        if idx == len(self._str):
            #print permutation
            permutation=[]
            for p in self._positions:
                permutation.append(self._str[p])

            print ''.join(permutation)
            return


        for i in xrange(0, len(self._positions), 1):
            if not self.index_used(i):
                self._positions[idx]=i
                self. permute(idx+1)
                self._positions[idx]=-1

def main(argv):
    perms = Permutator(argv[0])

    perms.permute(0)

if __name__ == '__main__':
    main(sys.argv[1:])
