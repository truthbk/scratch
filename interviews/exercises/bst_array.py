#!/usr/bin/python

import sys

class BSTNode(object):
    val = None
    left = None
    right = None

    def __init__(self, v=None, l=None, r=None):
        self.val = v
        self.left = l
        self.right = r

    def insert(self, v):
        if self.val is None:
            self.val = v

        if v<self.val:
            if self.left is None:
                self.left = BSTNode(v)
            else:
                self.left.insert(v)
        elif v>self.val:
            if self.right is None:
                self.right = BSTNode(v)
            else:
                self.right.insert(v)

    def get_height(self):
        l_height = 0;
        r_height = 0;
        if self.left:
            l_height = self.left.get_height()
        if self.left:
            r_height = self.right.get_height()

        return max(l_height, r_height) + 1

    def in_order(self):
        if self.left:
            self.left.in_order()
        print self.val
        if self.right:
            self.right.in_order()

    def pre_order(self):
        if self.left:
            self.left.in_order()
        if self.right:
            self.right.in_order()
        print self.val

    def post_order(self):
        print self.val
        if self.left:
            self.left.in_order()
        if self.right:
            self.right.in_order()



def bst_from_array(arr, bst):
    if(len(arr)>=1):
        bst.insert(arr[len(arr)/2])
    if(len(arr)>1):
        bst_from_array(arr[:len(arr)/2],bst)
        bst_from_array(arr[len(arr)/2+1:],bst)

def main(argv):
    numlist = map(int,argv[1:])
    BST = BSTNode()
    bst_from_array(numlist, BST)

    print "height is %d " % BST.get_height()
    BST.in_order()

if __name__ == '__main__':
    main(sys.argv[1:])
