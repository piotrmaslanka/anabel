# coding=UTF-8
# Generates simple 1000-point series
# Copyright (c) 2012 Piotr Maślanka
from struct import pack
import os

bp = 'd:/mirabelka/'

for a in (250, 500, 750, 1000):
    os.mkdir(bp+str(a))
    for b in (a-125, a):
        s = open(bp+str(a)+'/'+str(b), 'wb')
        s.write('ANABEL\x00\x00')
        for f in xrange(b-124, b+1):
            s.write(pack('>Q', f)+'\x00\x00\x00\x00')
        s.close()


