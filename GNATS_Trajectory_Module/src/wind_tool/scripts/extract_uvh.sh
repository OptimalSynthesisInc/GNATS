#!/bin/bash
#
# extract U-component of wind, V-component of wind, and Geopotential Height
# from all grib files in the specified directory and place the
# new grib files in a sub directory called uvh.

script_dir=$(dirname $(readlink -f $0))

grib_dir=$1
out_dir=$grib_dir/uvh

grib_copy=$script_dir/grib_copy

mkdir -p $out_dir

#grib_files=$(ls $grib_dir)
grib_files=$(find $grib_dir -type f)

for f in $grib_files; do

    # form the input file name
    in_file=$f

    # compute the base name and file extension to form the
    # output file name
    fname=$(basename "$f")
    extension="${fname##*.}"
    fname="${fname%.*}"
    out_file=$out_dir/$fname"_uvh."$extension

    echo "out_file=$out_file"

    $grib_copy -v -w name='Geopotential Height/U component of wind/V component of wind' $in_file $out_file
done