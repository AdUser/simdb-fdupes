Overview
--------

Tool for finding similar images on filesystem.

Requirements
------------

Runtime:

* libsimdb
* libmagick

Compile-time:

* headers for libraries above
* C compiler with -std=c99 support
* cmake >= 2.6
* checkinstall (optional)

Build and install
-----------------

    cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release .
    make
    make test
    sudo checkinstall -- make install

Usage
-----

    simdb-fdupes -h
    simdb-fdupes -v ~/images-dir/ | tee dups.txt
