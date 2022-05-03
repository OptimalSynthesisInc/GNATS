#!/bin/sh

set PWD = {PWD}

if [ ${PWD} != *"utility" ];then
   cd utility
fi

TARGET_PS=$(lsof -i tcp:3000 | awk 'NR!=1 {print $2}')

if [ "${TARGET_PS}" != '' ] ;   then
   set echo off
   
   kill -9 $TARGET_PS > /dev/null
   
   set echo on
fi

node-v8.11.1-linux-x64/bin/node Main.js
