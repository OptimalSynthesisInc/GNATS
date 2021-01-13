#!/bin/bash
#
# build the third-party dependencies and utils

# setup directories
script_dir=$(dirname $(readlink -f $0))
curr_dir=$(pwd)
third_party_dir=$(readlink -f $script_dir/../third-party)
include_dir=$(readlink -f $script_dir/../include)
lib_dir=$(readlink -f $script_dir/../lib)
share_dir=$(readlink -f $script_dir/../share)
proj_dir=$(readlink -f $script_dir/..)
top_dir=$(readlink -f $script_dir/../../..)
top_lib_dir=$top_dir/lib
top_include_dir=$top_dir/include

cd $third_party_dir

# obtain the name of the archives
jasper_archive=$(find . -name 'jasper-*.zip')
grib_api_archive=$(find . -name 'grib_api-*.tar.gz')
hdf5_archive=$(find . -name 'hdf5-*.tar.gz')

# obtain the name of the extraction directories
jasper_dir=$third_party_dir/$(echo $(basename $jasper_archive) | cut -d '.' --complement -f 4-)
grib_api_dir=$third_party_dir/$(echo $(basename $grib_api_archive) | cut -d '.' --complement -f 4-)
hdf5_dir=$third_party_dir/$(echo $(basename $hdf5_archive) | cut -d '.' --complement -f 4-)

build_jasper=0
build_grib_api=1
build_hdf5=0



if [ $build_jasper -eq 1 ]; then
    # Extract and build jasper
    unzip $jasper_archive
    chmod -R 755 $jasper_dir
    cd $jasper_dir
    ./configure --prefix=$jasper_dir/dist --disable-libjpeg  CFLAGS="-g -O3 -fPIC"
    make
    make install
    # copy the headers and libraries, remove pkgconfig directyro since
    # we won't support pkgconfig
    cp -r $jasper_dir/dist/include/* $include_dir/.
    cp -r $jasper_dir/dist/lib $proj_dir
    cd $third_party_dir
fi

if [ $build_grib_api -eq 1 ]; then
    # Extract and build grib_api
    tar zxvf $grib_api_archive
    chmod -R 755 $grib_api_dir
    cd $grib_api_dir
    ./configure --prefix=$grib_api_dir/dist --disable-fortran --disable-python --enable-pthread --with-jasper=$lib_dir CFLAGS="-g -O3 -fPIC -I$include_dir" LDFLAGS="-L$lib_dir"
    make
    make install
    # copy the headers and libraries, remove pkgconfig directory since
    # we won't support pkgconfig
    cp -r $grib_api_dir/dist/include/* $include_dir/.
    cp -r $grib_api_dir/dist/lib $proj_dir
    cp -r $grib_api_dir/dist/share/* $share_dir/tg_make_wind/.
    cd $third_party_dir
fi

if [ $build_hdf5 -eq 1 ]; then
    # Extract and build hdf5
    tar zxvf $hdf5_archive
    chmod -R 755 $hdf5_dir
    cd $hdf5_dir
    ./configure --prefix=$hdf5_dir/dist --enable-threadsafe --with-pthread=/usr/lib/x86_64-linux-gnu CFLAGS='-g -O3 -fPIC' CXXFLAGS='-g -O3 -fPIC'
    make
    make install
    # copy the headers and libraries, remove pkgconfig directory since
    # we won't support pkgconfig
    cp -r $hdf5_dir/dist/include/* $include_dir/.
    cp -r $hdf5_dir/dist/lib $proj_dir

    # also copy the headers and libraries to the top-level include and lib
    # directories so that hdf5 can be used by other projects.
    cp -r $hdf5_dir/dist/include/* $top_include_dir/.
    cp -r $hdf5_dir/dist/lib $top_dir

    cd $third_party_dir
fi
