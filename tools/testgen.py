# coding=UTF-8
# Generates simple 1000-point series
# Copyright (c) 2012 Piotr Maślanka
from struct import pack
import os

bp = 'd:/mirabelka/'


for z in (0, 1000, 2000, 3000):
    os.mkdir(bp+str(z))
    for a in (z+0, z+250, z+500, z+750):
        os.mkdir(bp+str(z)+'/'+str(a))
        for b in (a+125, a):
            s = open(bp+str(z)+'/'+str(a)+'/'+str(b), 'wb')
            s.write('ANABEL\x00\x00')
            for f in xrange(b, b+125):
                s.write(pack('QL', f, f+100))
            s.close()
import sys
sys.exit()

for a in (0, 250, 500, 750):
    os.mkdir(bp+str(a))
    for b in (a+125, a):
        s = open(bp+str(a)+'/'+str(b), 'wb')
        s.write('ANABEL\x00\x00')
        for f in xrange(b, b+125):
            s.write(pack('QL', f, b+100))
        s.close()





