#!/bin/bash

# Download Dependencies and utility sources
#  jasper: http://www.ece.uvic.ca/~frodo/jasper/software/jasper-1.900.1.zip
#  grib api: https://software.ecmwf.int/wiki/download/attachments/3473437/grib_api-1.11.0.tar.gz?api=v2
#  wgrib2: http://www.ftp.cpc.ncep.noaa.gov/wd51we/wgrib2/wgrib2.tgz

script_dir=$(dirname $(readlink -f $0))
cur_dir=$(pwd)
download_dir=$(readlink -f $script_dir/../third-party)

jasper_url=http://www.ece.uvic.ca/~frodo/jasper/software/jasper-1.900.1.zip

grib_api_url=https://software.ecmwf.int/wiki/download/attachments/3473437/grib_api-1.11.0.tar.gz

hdf5_url=http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.11.tar.gz

mkdir -p $download_dir

cd $download_dir
wget $jasper_url
wget $grib_api_url
wget $hdf5_url
cd $cur_dir