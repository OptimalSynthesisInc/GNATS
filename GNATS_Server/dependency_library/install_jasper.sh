#!/bin/bash

LIB_DIR=${PWD}/../dist

unzip jasper-1.900.1.zip

cd jasper-1.900.1/

./configure --prefix=${LIB_DIR}/jasper --disable-fortran CFLAGS='-m64 -fPIC' LDFLAGS=-m64
make
make install
