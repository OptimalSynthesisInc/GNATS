#!/bin/bash

LIB_DIR=${PWD}/../dist

tar -xvf grib_api-1.11.0.tar.gz

cd grib_api-1.11.0/

./configure --prefix=${LIB_DIR}/grib_api --disable-fortran --with-jasper=${LIB_DIR}/jasper CFLAGS='-m64 -fPIC' LDFLAGS=-m64
make
make check
make install
