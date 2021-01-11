#!/bin/sh

set echo off
kill -15 $1
set echo on

TARGET_PS=$(lsof -i tcp:3000 | awk 'NR!=1 {print $2}')

if [ "${TARGET_PS}" != '' ] ;   then
   set echo off
   
   kill -15 $TARGET_PS > /dev/null
   
   set echo on
fi

TARGET_PS=$(lsof -i tcp:2019 | awk 'NR!=1 {print $2}')

if [ "${TARGET_PS}" != '' ] ;   then
   set echo off
   
   kill -15 $TARGET_PS > /dev/null
   
   set echo on
fi