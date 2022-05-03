'''
          GENERALIZED NATIONAL AIRSPACE TRAJECTORY-PREDICTION SYSTEM (GNATS)
          Copyright 2018 by Optimal Synthesis Inc. All rights reserved
          
Author: Parikshit Dutta
Date: 2018-04-02
'''

from GNATS_header import DIR_share
import GNATS_MonteCarlo_Interface as GNATSMC
import PostProcessor as pp
import numpy as np

print('This file runs MC simulation. \nSeveral examples given. \nPlease go through the code that is commented out.')
print('Usage:  python sample/DEMO_MC_Simulation/Example_MC_code.py ')
args_dict = {1:'latitude', \
             2:'longitude', \
             3:'altitude', \
             4:'cruise_tas', \
             5:'cruise_altitude', \
             6:'flight_plan_lat_deg', \
             7:'flight_plan_lon_deg', \
             8:'flight_plan_lat_lon_deg', \
             9:'departure_delay', \
             10:'rate_of_climb_and_descent', \
             11:'pilot_partial_action_vertical_speed', \
             12:'controller_partial_action_course', \
             13:'controller_absence'}

MC_interface = GNATSMC.GNATS_MonteCarlo_Interface(track_file = DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_geo_uli.trx", \
                                                max_flt_lev_file= DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx", \
                                                interval = 1);

'''The Flight plan for ULI13-2553603 from the TRX file 
share/tg/trx/TRX_07132005_noduplicates_cut_crypted_10Flights.trx
is used in this Monte Carlo simulation.'''
ac_list = ["SQ12"];

'''Latitude location of the Aircraft at the start of the simulation to be perturbed.'''
# mean_lat = 38.0032615662;
# std_dev_lat = 0.01*np.abs(mean_lat)
# sample_sz_lat = 10;
# lat_vec = np.random.normal(mean_lat,std_dev_lat,sample_sz_lat)
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[1],lat_vec]

'''Longitude location of the Aircraft at the start of the simulation to be perturbed.'''
# mean_lon = -117.770446777;
# std_dev_lon = 0.0001*np.abs(mean_lon);
# sample_sz_lon = 10;
# lon_vec = np.random.normal(mean_lon,std_dev_lon,sample_sz_lon)
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[2],lon_vec]

'''Initial Altitude of the aircraft is not perturbed. But can be done similarly.'''  

'''Cruise Altitude to be perturbed.'''
# mean_alt = 33000;
# std_dev_alt = 0.05*mean_alt
# sample_sz_alt = 10;
# alt_vec = np.random.normal(mean_alt,std_dev_alt,sample_sz_alt)
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[5],alt_vec]

'''Flight Plan Latitude to be perturbed.'''
# fpwpidx = 4;
# mean_lat = 37.14604949951172;
# std_dev_lat = 0.01*mean_lat;
# sample_sz_lat = 10;
# lat_vec = np.random.normal(mean_lat,std_dev_lat,sample_sz_lat)    
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[6],lat_vec,fpwpidx]

'''Flight Plan Latitude and Longitude to be perturbed. 
Example illustrating perturbation of joint distribution variables.'''
# fpwpidx = 20;
# mean_lat = 40.2937660217;
# mean_lon = -87.557258606;
# std_dev_lat = 0.01*np.abs(mean_lat);
# std_dev_lon = 0.01*np.abs(mean_lon);
# mean = [mean_lat,mean_lon]
# cov = [[std_dev_lat**2, 0],[0,std_dev_lon**2]]
# sample_sz = 5;
# lat_vec,lon_vec = np.random.multivariate_normal(mean, cov, sample_sz).T
# fp_vec = []
# for i in range(sample_sz):
#     fp_vec.append([lat_vec[i],lon_vec[i]])
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[8],fp_vec,fpindex]


'''Departure delay to be perturbed.'''
# mean_time = 60;
# std_dev_time = 0.5*mean_time
# sample_sz_time = 10;
# time_vec = np.random.randint(mean_time-std_dev_time,mean_time+std_dev_time,sample_sz_time)
# #Put Poisson if you know the arrival or departure rate at airport.  
# time_vec = np.random.poisson(mean_time,sample_sz_time)
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[9],time_vec]

'''Rate of climb and descent to be perturbed'''
# mean_rocd = 13.6666669846
# std_dev_rocd = 0.1*np.abs(mean_rocd);
# sample_sz_rocd = 10;
# rocd_vec = np.random.normal(mean_rocd,std_dev_rocd,sample_sz_rocd)
# '''args = [ac_name, var_name, var_vals, fpindex (optional)]'''        
# args = [ac_list,args_dict[10],rocd_vec]


'''perturbing multiple aircrafts at a time.'''
# ac_list = ['ULI13-2553603','ULI3-33855','ULI18-9603'];
# var_name = [args_dict[2],args_dict[1],args_dict[3]];
# mean_1 = -117.770446777;
# std_dev_1 = 0.01*np.abs(mean_1);
# mean_2 = 35.0555992126;
# std_dev_2 = 0.01*np.abs(mean_2);
# mean_3 = 2200.0;
# std_dev_3 = 0.1*np.abs(mean_3);
# mean = [mean_1,mean_2,mean_3];
# cov = [[std_dev_1**2,0,0],[0,std_dev_2**2,0],[0,0,std_dev_3**2]];
# sample_sz = 50;
# vec_1,vec_2,vec_3 = np.random.multivariate_normal(mean, cov, sample_sz).T
# var_vec = []
# for i in range(sample_sz):
#     var_vec.append([vec_1[i],vec_2[i],vec_3[i]])    
# args = [ac_list,var_name,var_vec];


'''perturbing multiple variables of the same aircraft at a time.'''
var_name = [args_dict[6],args_dict[7],args_dict[5]];
fpwidx = [4,4,-1];#Flight plan latitude, Flight plan longitude, cruise altitude
mean_1 = 37.14604949951172;
std_dev_1 = 0.01*np.abs(mean_1);
mean_2 = -121.94306182861328;
std_dev_2 = 0.01*np.abs(mean_2);
mean_3 = 33000;
std_dev_3 = 0.01*np.abs(mean_3);
mean = [mean_1,mean_2,mean_3];
cov = [[std_dev_1**2,0,0],[0,std_dev_2**2,0],[0,0,std_dev_3**2]];
sample_sz = 5;
vec_1,vec_2,vec_3 = np.random.multivariate_normal(mean, cov, sample_sz).T
var_vec = []
for i in range(sample_sz):
    var_vec.append([vec_1[i],vec_2[i],vec_3[i]])    
args = [ac_list,var_name,var_vec,fpwidx];

'''perturbing multiple variables of the same aircraft at a time with pause.
It will perturb multiple variables '''
'''args = [ac_name, 
                time_pause,
                var_name, 
                var_vals, 
                index (optional),
                std_dev (optional), 
                sample_size (optional)]'''
                 
'''Usage 1: args = [ac_list,time_pause,var_name].
            Then the program will generate the samples on their own. 
            Mean is nominal value. std dev = 0.1 and sample size = 10.
            We have generated only normal distribution. 
            You can generate your own distribution by modifying the function:
            getValue() in GNATS_MonteCarlo_Interface.py'''     
                       
# time_pause = [400,700]                
# var_name = [args_dict[2],args_dict[1]];
# args = [ac_list,time_pause,var_name];
 
'''Usage 2: args = [ac_list,time_pause,var_name,[[]],[],std_dev, sample_size ].
            Then the program will generate the samples on their own. 
            Mean is nominal value. std dev and sample size provided.
            We have generated only normal distribution. 
            You can generate your own distribution by modifying the function:
            getValue() in GNATS_MonteCarlo_Interface.py'''
 
# time_pause = [400,700]                
# var_name = [args_dict[2],args_dict[1]];
# std_dev = [0.1,0.1];
# sample_size = 5; 
# args = [ac_list,time_pause,var_name,[[]],[],std_dev,sample_size];
 
'''Usage 3: args = [ac_list,time_pause,var_name,var_vals,fpindex,std_dev, sample_size ].
            Here the values are generated by the user and saved into var_vals 
            fpindex is the flight plan index and has the same size as var_vals.'''
# time_pause = [200,300,400]
# var_name = [args_dict[6],args_dict[7],args_dict[5]];
# fpwidx = [4,4,-1];#Flight plan latitude, Flight plan longitude, cruise altitude
# mean_1 = 37.14604949951172;
# std_dev_1 = 0.01*np.abs(mean_1);
# mean_2 = -121.94306182861328;
# std_dev_2 = 0.01*np.abs(mean_2);
# mean_3 = 33000;
# std_dev_3 = 0.01*np.abs(mean_3);
# mean = [mean_1,mean_2,mean_3];
# cov = [[std_dev_1**2,0,0],[0,std_dev_2**2,0],[0,0,std_dev_3**2]];
# sample_sz = 10;
# vec_1,vec_2,vec_3 = np.random.multivariate_normal(mean, cov, sample_sz).T
# var_vec = []
# for i in range(sample_sz):
#     var_vec.append([vec_1[i],vec_2[i],vec_3[i]])    
# args = [ac_list,time_pause,var_name,var_vec,fpwidx];        


'''Usage 3: Another example for pilot and controller action. 
            Use for pilot and controller actions. 
            Here only two pilot and controller action perturbations are included. 
            Others can be included similarly. You need to change the following:
            @1: args_dict variable in this file and GNATS_MonteCarlo_Interface.py.
            @2: write new code similar to one below to input it to MC parser. 
            @3: add the variable in setValue( ) function in GNATS_MonteCarlo_Interface.py'''
# time_pause = [200,300,1500]
# fpwidx = [-1,-1,-1] #Not really needed but used for argument consistency
# var_name = [args_dict[11],args_dict[12],args_dict[13]]
# sample_sz = 10;
# meanPercentage = 25;
# mean_vertical_spd_kts = 200;
# vec_1_1 = np.random.uniform(meanPercentage*0.9, meanPercentage*1.1,sample_sz);
# vec_1_2 = np.random.uniform(mean_vertical_spd_kts*0.9, mean_vertical_spd_kts*1.1,sample_sz)
# meanPercentage = 50;
# mean_course_deg = 200;
# vec_2_1 = np.random.uniform(meanPercentage*0.9, meanPercentage*1.1,sample_sz);
# vec_2_2 = np.random.uniform(mean_course_deg*0.9, mean_course_deg*1.1,sample_sz)
# meanTimeStep = 6;
# vec_3 = np.random.exponential(meanTimeStep,sample_sz)
# var_vec = [];
# for i in range(sample_sz):
#     var_vec.append([ [vec_1_1[i],vec_1_2[i]], [vec_2_1[i],vec_2_2[i]], vec_3[i] ]);
# args = [ac_list,time_pause,var_name,var_vec,fpwidx];        


''' RUN MONTE CARLO'''
'''This is deprecated. However, will still run. We have upgraded it 
with the function below.'''
#MC_interface.runMCSims(args)

'''Handles all kinds of MC Simulations. If you want to perturb IC you can do it. 
If you want to perturb states and controls at runtime you can also do that. 
You need to specify times when you want to do that.'''
MC_interface.MCManager(args)

'''Post Processing and plotting the MC examples.'''
for ac in ac_list:
    post_process = pp.PostProcessor(file_path = ".", \
                                    ac_name = ac,
                                    auto_detect_server_folder_flag = False);
#     post_process.plotRoutine();
#     post_process.plotSingleAircraftTrajectory();
#      
#     post_process.plotVariable('departure_delay')
    
    post_process.plotFinalHistograms(['latitude', \
                                      'longitude', \
                                      'altitude'])
