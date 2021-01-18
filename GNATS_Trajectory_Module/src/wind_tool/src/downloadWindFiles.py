#!/usr/bin/env python

'''
Copyright 2018 Optimal Synthesis Inc.
Author: Parikshit Dutta

THIS FILE DOWNLOADS THE LATEST WIND FILES IN GRIB2 FORMAT 
AND STORES IN THE LOCATION OF YUR CHOICE (DEFAULT IS CURRENT FOLDER)
:usage ./createWindFiles.py <path to dir to store current wind files> <state date and time> < end date and time>
Typical example:

    $./createWindFiles.py ./ ---> Download the current wind data  and 
                                 forecasted wind data for the next 24 hours 
                                 from the current UTC time to the current directory (./).
                                 
    $./createWindFiles.py ./ 2018-04-09,9 ---> Download wind data from 9th April 2018 9 hours UTC
                                               all the way upto current UTC time to the current directory.
    
    
    $./createWindFiles.py ./ 2018-04-09,9 2018-04-12,17---> Download wind data from 9th April 2018 9 hours UTC 
                                                            to 12th April 2018 17 hours UTC to the current directory.
'''

from ftplib import FTP
import os, sys, os.path
import datetime, numpy as np

max_hr_in_forecast = 23

def daterange( start_date, end_date ):
    if start_date <= end_date:
        for n in range( ( end_date - start_date ).days + 1 ):
            yield start_date + datetime.timedelta( n )
    else:
        for n in range( ( start_date - end_date ).days + 1 ):
            yield start_date - datetime.timedelta( n )

def downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filestodnld):

    ftp.cwd(rapdir)
    filenames = ftp.nlst() # get filenames within the directorys

    for f in filenames:
        if f in filestodnld and '.idx' not in f:
            local_filename = os.path.join(dirtodnld, f)
            if os.path.exists(local_filename):
                print('File ', f,' already exists in ', dirtodnld);
                continue;
            print('Now downloading ',f,' to ',local_filename);
            file = open(local_filename, 'wb')
            ftp.retrbinary('RETR '+ f, file.write)
    ftp.cwd('../../../../../../../');

    

def downloadManager(dirtodnld = os.getcwd(), \
                      hour1 = -100,day1 = -100,month1 = -100,year1=-100, \
                      hour2 = -100,day2 = -100,month2 = -100,year2=-100):
    
    if not os.path.exists(dirtodnld):
        os.mkdir(dirtodnld)
    
    NOAAwebsite = 'ftpprd.ncep.noaa.gov'
    ftp = FTP(NOAAwebsite)
    ftp.login()  

    '''As the product needed is the real time current data, we will download 
    the forecast data for the current hour    
    '''    
    if hour1 < 0:
        


        now = datetime.datetime.utcnow()
        hour = now.hour-1;        
        if hour < 10:
            hourstring = 't0'+str(hour)+'z'
        else:
            hourstring = 't'+str(hour)+'z'        
        filenames = []
        for k in range(max_hr_in_forecast+1):
            if k >= 10:
                filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
            else:
                filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
        
        if now.month <10 and now.day < 10:
            rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+'0'+str(now.day);
        elif now.month <10:
            rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+str(now.day);
        else:
            rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+str(now.month)+str(now.day);

        downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
    
    elif hour1 >= 0 and hour2 < 0:
        
        now = datetime.datetime.utcnow()
        
        start = datetime.datetime(year1,month1,day1,hour1,0,0,0)
        
        if start > now:
            
            start = now;
        
        
        if start.day < now.day or start.month < now.month or start.year < now.year:
            for date in daterange( start, now ):
                
                
                if date.month < 10 and date.day < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+'0'+str(date.month)+'0'+str(date.day);
                elif date.month < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+'0'+str(date.month)+str(date.day);
                else:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+str(date.month)+str(date.day);
                
                if date.day == start.day:
                    filenames = [] 
                    for k in range(start.hour,max_hr_in_forecast+1):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                
                            
                    
                    
                elif date.day == now.day:
                    filenames = [] 
                    for k in range(0,now.hour):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                    
                    if k >= 10:
                        hourstring = 't'+str(now.hour-1)+'z'
                    else:
                        hourstring = 't'+str(now.hour-1)+'z'
                    for k in range(max_hr_in_forecast+1):
                        if k >= 10:
                            filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
                        else:
                            filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
                
                            
                    
                else:
                    filenames = [] 
                    for k in range(max_hr_in_forecast+1):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                
                            
                downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
            
        else:
            if start.hour < now.hour:
                
                if now.month <10 and now.day < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+'0'+str(now.day);
                elif now.month <10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+str(now.day);
                else:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+str(now.month)+str(now.day);

                filenames = []
                for k in range(start.hour,now.hour):
                    if k >= 10:
                        hourstring = 't'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                    else:
                        hourstring = 't0'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
                
                
            elif start.hour == now.hour:
                if now.hour < 10:
                    hourstring = 't0'+str(now.hour-1)+'z'
                else:
                    hourstring = 't'+str(now.hour-1)+'z'        
                filenames = []
                for k in range(max_hr_in_forecast+1):
                    if k >= 10:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
                    else:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
        
                if now.month <10 and now.day < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+'0'+str(now.day);
                elif now.month <10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+'0'+str(now.month)+str(now.day);
                else:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(now.year)+str(now.month)+str(now.day);

                downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
    
    else:
        now = datetime.datetime.utcnow()
        start = datetime.datetime(year1,month1,day1,hour1,0,0,0)
        
        end = datetime.datetime(year2,month2,day2,hour2,0,0,0)        
        
        if year1 == year2 and month1 == month2 and day1 == day2:
            if now.month <10 and now.day < 10:
                rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+'0'+str(month1)+'0'+str(day1);
            elif now.month <10:
                rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+'0'+str(month1)+str(day1);
            else:
                rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+str(month1)+str(day1);
            
            if hour1 < now.hour and hour2 <now.hour:
                filenames = []
                for k in range(hour1,hour2):
                    if k >= 10:
                        hourstring = 't'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                    else:
                        hourstring = 't0'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
            elif hour1 < now.hour and hour2 > now.hour:
                filenames = []
                for k in range(hour1,now.hour):
                    if k >= 10:
                        hourstring = 't'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                    else:
                        hourstring = 't0'+str(k)+'z'
                        filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                
                if now.hour < 10:
                    hourstring = 't0'+str(now.hour-1)+'z'
                else:
                    hourstring = 't'+str(now.hour-1)+'z'  
                                                
                for k in range(min(hour2,max_hr_in_forecast+1)):
                    if k >= 10:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
                    else:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
            else:
                
                filenames = []
                
                if now.hour < 10:
                    hourstring = 't0'+str(now.hour-1)+'z'
                else:
                    hourstring = 't'+str(now.hour-1)+'z'  
                                                
                for k in range(hour1,hour2):
                    if k >= 10:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
                    else:
                        filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
                                                                    
            downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
        
        else:
            
            
            for date in daterange(start,end):
                
                if date.month < 10 and date.day < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+'0'+str(date.month)+'0'+str(date.day);
                if date.month < 10:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+'0'+str(date.month)+str(date.day);
                else:
                    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(date.year)+str(date.month)+str(date.day);
                        
                    
                if date.day == start.day and date.month == start.month and date.year == start.year:
                    filenames = [] 
                    for k in range(start.hour,max_hr_in_forecast+1):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                
                            

                    
                elif date.day == end.day and date.month == end.month and date.year == end.year:
                    filenames = [] 
                    for k in range(0,end.hour):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                    
                    if end.hour > now.hour:
                        if now.hour < 10:
                            hourstring = 't0'+str(now.hour-1)+'z'
                        else:
                            hourstring = 't'+str(now.hour-1)+'z'  
                        
                        for k in range(now.hour,end.hour):
                            if k >= 10:
                                filenames.append('rap.'+hourstring+'.awp130pgrbf'+str(k)+'.grib2')
                            else:
                                filenames.append('rap.'+hourstring+'.awp130pgrbf0'+str(k)+'.grib2')
                            
                else:
                    filenames = [] 
                    for k in range(max_hr_in_forecast+1):                                                
                        if k >= 10:
                            hourstring = 't'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                        else:
                            hourstring = 't0'+str(k)+'z'
                            filenames.append('rap.'+hourstring+'.awp130pgrbf00.grib2')
                
                downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
                    
                    
                
    ftp.quit()

def parseDateAndHour(time1,time2 = []):

    time1 = time1.split(',')
    hour1 = time1[1];
    day1 = time1[0].split('-')[2];
    month1 = time1[0].split('-')[1];
    year1 = time1[0].split('-')[0];
    
    if time2 != []:
        time2 = time2.split(',')
        hour2 = time2[1];
        day2 = time2[0].split('-')[2];
        month2 = time2[0].split('-')[1];
        year2 = time2[0].split('-')[0];
    
    
        return np.int(hour1),np.int(day1),np.int(month1),np.int(year1), \
            np.int(hour2),np.int(day2),np.int(month2),np.int(year2)
            
    return np.int(hour1),np.int(day1),np.int(month1),np.int(year1)
    

             
        
        
    



if __name__ == '__main__':
    if len(sys.argv) == 4:
        hour1,day1,month1,year1,hour2,day2,month2,year2 = parseDateAndHour(sys.argv[2],sys.argv[3])
        downloadManager(sys.argv[1],hour1,day1,month1,year1, \
                            hour2,day2,month2,year2)
    elif len(sys.argv) == 3:
        hour1,day1,month1,year1 = parseDateAndHour(sys.argv[2])
        downloadManager(sys.argv[1],hour1,day1,month1,year1)
    elif len(sys.argv) == 2:
        downloadManager(sys.argv[1])
    else:
        downloadManager()
