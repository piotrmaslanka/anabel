What is Anabel?
===============

Anabel is an open source equally-sized-blob-valued time-series-indexed storage engine written as a C++ library. 

Ideally, you should consider it as a very large hashmap, with unsigned 64-bit timestamps being keys, and binary blobs being valued. The primary constraint is that those blobs have exactly the same size. Timeseries - as the basic database is called - is an array of timestamps and values, timestamps being sorted ascending. In this way, timestamps become indexes.

Anabel is a C++ library. You are welcome to use it directly in your project, and because of the fact that values have no types you will frequently need to play around with the memory to get the desired effect. Anabel can be nicely bound to Python via SWIG - Python wrapper functions will even output the data directly as numpy tables! SWIG script and sample easy-to-modify build script is included.

Anabel is concurrency-safe via portable advisory locking. All simple operations do not preclude locking the entire database for writing.

Anabel's on-disk binary format is very simple and capable of storing absurd amounts of data. It uses very little space comparing to putting the same data in a RDBMS. If you had the need to read the data without interfacing with Anabel, the format's easy enough for you to do it in fabulously small amount of time and code.

Because Anabel is a library, getting the most out of it would require writing a program to interface with it. However, a simple command-line program - "werkzeug" - is attached. It can perform most simple operations on the database, and in a pinch can be used to do full interfacing with it.

See the project's wiki for more information, and LICENSE for license text.