#! /bin/bash
g++ -O2 -fPIC -I$1 -c py-interface.cpp
g++ -O2 -fPIC -I$1 -c exceptions.cpp
g++ -O2 -fPIC -I$1 -c fsobjects.cpp
g++ -O2 -fPIC -I$1 -c readquery.cpp
g++ -O2 -fPIC -I$1 -c timeseries.cpp
swig -python -c++ anabel.i
c++ -O2 -x c++ -fPIC -c anabel_wrap.cxx -I/usr/include/python2.7 -I/usr/include/c++/4.7.2 -I$1
c++ -lboost_system -lboost_filesystem -shared py-interface.o anabel_wrap.o exceptions.o fsobjects.o readquery.o timeseries.o -o _Anabel.so

echo "
class DataBuffer(object):
        def __init__(self, record_size, amount):
                self.abargs = record_size, amount
        def __enter__(self):
                self.buffer = allocate_buffer(*self.abargs)
                return self.buffer
        def __exit__(self, type, value, traceback):
                deallocate_buffer(self.buffer)
                return False
" >> Anabel.py
