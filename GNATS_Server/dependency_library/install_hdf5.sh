#!/bin/bash

LIB_DIR=${PWD}/../dist

gzip -d hdf5-1.8.21.tar.gz
tar -xvf hdf5-1.8.21.tar

if [ -d "hdf5-1.8.21" ]
then
	cd hdf5-1.8.21/
	
	./configure --prefix=${LIB_DIR}/hdf5 --enable-production --enable-debug=all --enable-threadsafe --enable-unsupported --with-pthread=/usr/include,/usr/lib --enable-cxx CFLAGS='-fPIC'
	make
	make test
	make install
	
	cp ${LIB_DIR}/hdf5/lib/libhdf5.so.10.3.2 ${LIB_DIR}
	mv ${LIB_DIR}/libhdf5.so.10.3.2 ${LIB_DIR}/libhdf5.so
	
	cd ${LIB_DIR}
	ln -s libhdf5.so libhdf5.so.10
	
	cd ../dependency_library
fi
