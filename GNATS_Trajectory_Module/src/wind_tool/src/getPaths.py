#!/usr/bin/python
import os
#rootDir = "/home/pdutta/Proj400GB/Projects/1603/DATA/wind/rap_130_20170406/nomads.ncdc.noaa.gov/RUC/13km/201704/20170406/";
rootDir = "/home/pdutta/Proj400GB/Projects/1603/DATA/wind/rap_130_20170419/nomads.ncdc.noaa.gov/RUC/13km/201704/20170419/";
cnt = 0;
file_exp = []
for dir_, _, files in os.walk(rootDir):
    for fileName in files:
	if '1900' in fileName:#('t14z' in fileName) and ('pgrbf' in fileName) and ('130' in fileName):
            relDir = os.path.relpath(dir_, rootDir)
            relFile = os.path.join(relDir, fileName)
            strng = str(cnt)+',  '+rootDir + relFile;
            print strng 
            file_exp.append(strng);
            cnt = cnt+1;

