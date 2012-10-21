#! /bin/bash
# Extract the files so that *.cpp and *.h are in anabel directory in some subfolder S. Then
# invoke build.sh with a single parameter - S, that is - the directory where those can be found
# you may want to adjust Python include library here
g++ -fPIC -I$1 -c main.cpp
g++ -fPIC -I$1 -c exceptions.cpp
g++ -fPIC -I$1 -c fsobjects.cpp
g++ -fPIC -I$1 -c readquery.cpp
g++ -fPIC -I$1 -c timeseries.cpp
swig -python -c++ anabel.i
c++ -x c++ -fPIC -c anabel_wrap.cxx -I/usr/include/python2.7 -I$1
c++ -lboost_system -lboost_filesystem -shared anabel_wrap.o exceptions.o fsobjects.o readquery.o timeseries.o -o _Anabel.so
