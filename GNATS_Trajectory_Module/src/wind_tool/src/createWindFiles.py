#!/usr/bin/env python

'''THIS FILE DOWNLOADS THE LATEST WIND FILES IN GRIB2 FORMAT 
AND STORES IN THE LOCATION OF YUR CHOICE (DEFAULT IS CURRENT FOLDER)
:usage ./createWindFiles.py <path to dir to store current wind files> <state date and time> < end date and time> <path_to_NATS_Server_utility>
Typical example:

    $./createWindFiles.py ./ ---> Download the current wind data and 
                                 forecasted wind data for the next 21 hours 
                                 from the current UTC time to the current directory (./).
                                 Further this will process the GRIB2 files and create .h5 files 
                                 from them to be used in NATS. This last functionality
                                 uses the windtool module from NATS and has to be compiled. 
                                 
    $./createWindFiles.py ./ 2018-04-09,9 ---> Download wind data from 9th April 2018 9 hours UTC
                                               all the way upto current UTC time to the current directory.
    
    
    $./createWindFiles.py ./ 2018-04-09,9 2018-04-12,17---> Download wind data from 9th April 2018 9 hours UTC 
                                                            to 12th April 2018 17 hours UTC to the current directory.
'''

from ftplib import FTP
import os, sys, os.path
import datetime, numpy as np

max_hr_in_forecast = 23

if os.name == 'nt': # Windows OS
    env_GNATS_SERVER_HOME = os.environ.get('GNATS_HOME')
else :
    env_GNATS_SERVER_HOME = os.environ.get('GNATS_SERVER_HOME')

if (env_GNATS_SERVER_HOME is None) :
    GNATS_SERVER_HOME = "./"
else :
    GNATS_SERVER_HOME = env_GNATS_SERVER_HOME + "/"

def daterange( start_date, end_date ):
    if start_date <= end_date:
        for n in range( ( end_date - start_date ).days + 1 ):
            yield start_date + datetime.timedelta( n )
    else:
        for n in range( ( start_date - end_date ).days + 1 ):
            yield start_date - datetime.timedelta( n )

def getRap_dateString(varDate):
	if varDate.month < 10 and varDate.day < 10:
		retString = str(varDate.year)+'0'+str(varDate.month)+'0'+str(varDate.day);
	elif varDate.month < 10:
		retString = str(varDate.year)+'0'+str(varDate.month)+str(varDate.day);
	elif varDate.day < 10:
		retString = str(varDate.year)+str(varDate.month)+'0'+str(varDate.day);
	else:
		retString = str(varDate.year)+str(varDate.month)+str(varDate.day);
	
	return retString
	
def downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filestodnld):
    ftp.cwd(rapdir)
    filenames = ftp.nlst() # get filenames within the directorys

    for f in filenames:
        if f in filestodnld and '.idx' not in f:
            local_filename = os.path.join(dirtodnld, f)
            if os.path.exists(local_filename):
                print 'File ', f,' already exists in ', dirtodnld
                continue;
            print 'Now downloading ',f,' to ',local_filename;
            file = open(local_filename, 'wb')
            ftp.retrbinary('RETR '+ f, file.write)
    ftp.cwd('../../../../../../../');

    

def downloadManager(dirtodnld = os.getcwd(), \
                      hour1 = -100,day1 = -100,month1 = -100,year1=-100, \
                      hour2 = -100,day2 = -100,month2 = -100,year2=-100):
    if not os.path.exists(dirtodnld):
        print "Creating directory: ", dirtodnld
        os.mkdir(dirtodnld)
    
    NOAAwebsite = 'ftpprd.ncep.noaa.gov'
    ftp = FTP(NOAAwebsite)
    ftp.login()

    rapPath = 'pub/data/nccf/com/rap/prod'
	
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

        rapdir = rapPath + '/rap.' + getRap_dateString(now)
        
        if not(rapdir in ftp.nlst(rapPath)) :
         	lastDate = now + datetime.timedelta(days=-1)
	    	rapdir = rapPath + '/rap.' + getRap_dateString(lastDate)

        downloadWindFilesinFolder(ftp, rapdir, dirtodnld, filenames)
    
    elif hour1 >= 0 and hour2 < 0:
        now = datetime.datetime.utcnow()
        
        start = datetime.datetime(year1,month1,day1,hour1,0,0,0)
        
        if start > now:
            start = now;
        
        if start.day < now.day or start.month < now.month or start.year < now.year:
            for date in daterange( start, now ):
				rapdir = rapPath + '/rap.' + getRap_dateString(date)
		
		 		if not(rapdir in ftp.nlst(rapPath)) :
		 			lastDate = date + datetime.timedelta(days=-1)
					rapdir = rapPath + '/rap.' + getRap_dateString(lastDate)
                
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
				rapdir = rapPath + '/rap.' + getRap_dateString(now)
		
		 		if not(rapdir in ftp.nlst(rapPath)) :
		 			lastDate = now + datetime.timedelta(days=-1)
					rapdir = rapPath + '/rap.' + getRap_dateString(lastDate)
				
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
        
                rapdir = rapPath + '/rap.' + getRap_dateString(now)
		
                if not(rapdir in ftp.nlst(rapPath)) :
		 			lastDate = now + datetime.timedelta(days=-1)
					rapdir = rapPath + '/rap.' + getRap_dateString(lastDate)
				
                downloadWindFilesinFolder(ftp,rapdir,dirtodnld,filenames)
    
    else:
        now = datetime.datetime.utcnow()
        start = datetime.datetime(year1,month1,day1,hour1,0,0,0)
        
        end = datetime.datetime(year2,month2,day2,hour2,0,0,0)        
        
        if year1 == year2 and month1 == month2 and day1 == day2:
			if now.month < 10 and now.day < 10:
			    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+'0'+str(month1)+'0'+str(day1);
			elif now.month < 10:
			    rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+'0'+str(month1)+str(day1);
			elif now.day < 10:
				rapdir = 'pub/data/nccf/com/rap/prod/rap.' + str(year1)+str(month1)+'0'+str(day1);
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
				rapdir = rapPath + '/rap.' + getRap_dateString(date)
		
		 		if not(rapdir in ftp.nlst(rapPath)) :
		 			lastDate = date + datetime.timedelta(days=-1)
					rapdir = rapPath + '/rap.' + getRap_dateString(lastDate)
                
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
    
def writeConfigFile(dirtodnld = './',dirconffile = './',conf_filename = 'wind_tool.conf'):
    LAT_MIN = 20
    LAT_MAX = 50
    LAT_STEP = .5
    LON_MIN = -130
    LON_MAX = -60
    LON_STEP = .5
    ALT_MIN = 1000
    ALT_MAX = 45000
    ALT_STEP = 500

    splitted = dirtodnld.split('/');
        
    GRID_DEFINITION_FILE = splitted[-1]+'_grid.h5'
    
    
    
    fid = open(os.path.join(dirconffile,conf_filename),'w')
    fid.write('LAT_MIN = '+str(LAT_MIN)+'\n')
    fid.write('LAT_MAX = '+str(LAT_MAX)+'\n')
    fid.write('LAT_STEP = '+str(LAT_STEP)+'\n')
    fid.write('LON_MIN = '+str(LON_MIN)+'\n')
    fid.write('LON_MAX = '+str(LON_MAX)+'\n')
    fid.write('LON_STEP = '+str(LON_STEP)+'\n')
    fid.write('ALT_MIN = '+str(ALT_MIN)+'\n')
    fid.write('ALT_MAX = '+str(ALT_MAX)+'\n')
    fid.write('ALT_STEP = '+str(ALT_STEP)+'\n')
    fid.write('GRID_DEFINITION_FILE = '+str(GRID_DEFINITION_FILE)+'\n')    
    
    filelist = os.listdir(dirtodnld);
    cnt = 0;
    for k in range( len(filelist)):
        if filelist[k].endswith('.grb2') or  filelist[k].endswith('grib') or filelist[k].endswith('grib2'):
            fid.write(str(cnt)+' ,'+dirtodnld+'/'+filelist[k]+'\n');
	    cnt = cnt +1;
    fid.close()
        
            
def createH5FilesFromGrib(dirtodnld = './'):
    #if os.getcwd().endswith != 'src':
    #    os.chdir(execpath)
    
    if os.name == 'nt': # Windows OS
        ldLibraryPath = GNATS_SERVER_HOME + "/dist_win"
        exportLdLibraryPathCmd = "set PATH=%PATH%;" + ldLibraryPath + " && "
    else:
        ldLibraryPath = GNATS_SERVER_HOME + "/dist"
        exportLdLibraryPathCmd = "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:" + ldLibraryPath + " && "
    
    writeConfigFile(dirtodnld, os.getcwd());
    
    # Use stdbuf command to set shell buffer size.  This helps immediate log output when calling this Python program from outside.
    if os.name == 'nt': # Windows OS
        windtoolrun = ".\wind_tool"
        windtoolargs = " --config-file=wind_tool.conf --out-folder="+dirtodnld
    else:  
        windtoolrun = "stdbuf -i0 -o0 -e0 ./wind_tool"
        windtoolargs = " --config-file='wind_tool.conf' --out-folder="+dirtodnld
    
    os.system(exportLdLibraryPathCmd + windtoolrun + windtoolargs);
    os.chdir('../../..')

def deleteGribFiles(dirtodnld = './'):
    
    filelist = os.listdir(dirtodnld);
    
    if filelist == None:
        return 0;
    else:
        for fl in filelist:
            if fl.endswith('grib2') or fl.endswith('.grb2') or fl.endswith('grib'):
                os.system('rm '+ dirtodnld+'/'+fl);

        
    



if __name__ == '__main__':
    if len(sys.argv) == 4:
        hour1,day1,month1,year1,hour2,day2,month2,year2 = parseDateAndHour(sys.argv[2],sys.argv[3])
        downloadManager(sys.argv[1],hour1,day1,month1,year1, \
                        hour2,day2,month2,year2)
        createH5FilesFromGrib(sys.argv[1])
    elif len(sys.argv) == 3:
        hour1,day1,month1,year1 = parseDateAndHour(sys.argv[2])
        downloadManager(sys.argv[1],hour1,day1,month1,year1)
        createH5FilesFromGrib(sys.argv[1])
    elif len(sys.argv) == 2:
        downloadManager(sys.argv[1])
        createH5FilesFromGrib(sys.argv[1])
#         deleteGribFiles(sys.argv[1]);
    else:
        downloadManager()
