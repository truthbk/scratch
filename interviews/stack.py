#!/usr/bin/python

import sys
import types

class Stack(object): #LIFO

    def __init__(self):
        self.s = []

    def push(self, elem):
        self.s.append(elem)

    def pop(self):
        el = self.s.pop()
        return el

    def peek(self):
        return self.s[len(self.s)-1]

    def size(self):
        return len(self.s)

    def output(self):
        for i in reversed(xrange(0, self.size(), 1)):
            print self.s[i]

class MyQueue(object): #FIFO
    def __init__(self):
        self.s_in = Stack()
        self.s_out = Stack()

    def enqueue(self, el):
        if self.s_out.size() == 0:
            self.s_in.push(el)
        else:
            for i in xrange(0, self.s_out.size(),1):
                self.s_in.push(self.s_out.pop())

            self.s_in.push(el)

    def dequeue(self):
        if self.s_in.size()==0 and self.s_out.size()==0:
            return None

        if self.s_out.size():
            return self.s_out.pop()

        for i in xrange(0, self.s_in.size(), 1):
            self.s_out.push(self.s_in.pop())

        return self.s_out.pop()

    def size(self):
        return self.s_in.size()+self.s_out.size()


def transfer(orig, dest, val):
    while orig.size()>0 and orig.peek()>val:
        dest.push(orig.pop())

def main(argv):
    print "Sort a stack (biggest on top) using only two stacks."
    data = [5, 9, 3, 10, 14, 2, 17]
    dataq = [5, 9, 3, 10, 14, 2, 17]
    s_1 = Stack()
    s_1.s = data
    s_2 = Stack()

    while s_1.size() != 0:
        el = s_1.pop()
        if s_2.size() == 0 or s_2.peek() < el:
            s_2.push(el)
        else:
            transfer(s_2,s_1,el)
            s_1.push(el)

        s_2.output()

    print "Implement a queue with two stacks."
    q = MyQueue()
    q.enqueue(dataq[0])
    q.enqueue(dataq[1])
    q.enqueue(dataq[2])
    print q.dequeue()
    q.enqueue(dataq[3])
    print q.dequeue()
    for d in dataq:
        q.enqueue(d)
    while q.size():
        print q.dequeue()

if __name__ == '__main__':
    main(sys.argv[1:])
