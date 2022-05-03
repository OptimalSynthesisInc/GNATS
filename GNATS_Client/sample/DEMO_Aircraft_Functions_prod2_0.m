%{
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
%}

% GNATS sample
%
% Optimal Synthesis Inc.
%
% Oliver Chen
% 03.12.2020
%
% Demo of aircraft-related functions.
%
% Users can learn how to obtain aircraft instance, show related aircraft information, start/pause/resume simulation and output result trajectory file.

% Run header module
run('GNATS_MATLAB_Header_Client.m');

if not(isempty(simulationInterface))
    simulationInterface.clear_trajectory();

    environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
    
    aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx'));
    
    aircraftIdArray_withinZone = aircraftInterface.getAircraftIds(30.0, 35.0, -115.0, -90.0, -1.0, -1.0);
    if ((~isempty(aircraftIdArray_withinZone)) & (size(aircraftIdArray_withinZone) > 0))
        for i = 1: size(aircraftIdArray_withinZone)
            curAcId = aircraftIdArray_withinZone(i);

            fprintf('Aircraft id in selected zone = %s, time = %ld\n', char(curAcId), round(tic * 1000));
        end
    end
    
    disp('****************************************');

    curAircraft = aircraftInterface.select_aircraft('SQ12');
    if ~isempty(curAircraft)
        airborne_flight_plan_waypoint_name_array = curAircraft.getFlight_plan_waypoint_name_array();

        for i = 1: (size(airborne_flight_plan_waypoint_name_array))
            fprintf('SQ12 airborne flight plan waypoint name = %s\n', char(airborne_flight_plan_waypoint_name_array(i, 1)));
        end
        
        curAircraft.delay_departure(100); % Delay the departure time for 100 sec
    else
        fprintf('Aircraft SQ12 not found');
    end
    
    simulationInterface.setupSimulation(10000, 10);

    simulationInterface.start(1000);

    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE)
            break;
        else
            pause(1);
        end
    end
    
    curAircraft = aircraftInterface.select_aircraft('SQ12');
    if not(isempty(curAircraft))
        disp('****************************************');
        fprintf('SQ12 (pausing at %f', simulationInterface.get_curr_sim_time());
        fprintf(' sec, latitude = %f', curAircraft.getLatitude_deg());
        fprintf(', longitude = %f', curAircraft.getLongitude_deg());
        fprintf(', altitude = %f\n', curAircraft.getAltitude_ft());
        disp('****************************************');
    end

	simulationInterface.resume();

    % Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
    while true
        runtime_sim_status = simulationInterface.get_runtime_sim_status();
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED)
            break;
        else
            pause(1);
        end
    end

    % Format epoch time string
    millis = datestr(now, 'yyyymmdd HHMMSS');
    InputDate = datenum(millis, 'yyyymmdd HHMMSS');
    UnixOrigin = datenum('19700101 000000', 'yyyymmdd HHMMSS');
    EpochSecond = round((InputDate-UnixOrigin)*86400000);

	S = dbstack();
    cur_filename = char(S(1).file);
    strIndexArray = strfind(cur_filename, '.m');

    disp('Outputting trajectory data.  Please wait....');
    fileName = sprintf('%s_%s.csv', cur_filename(1: strIndexArray(1)-1), num2str(EpochSecond));
    % Output the trajectory result file
    simulationInterface.write_trajectories(fileName);
    
    aircraftInterface.release_aircraft();
    environmentInterface.release_rap();
end

gnatsClient.disConnect()
