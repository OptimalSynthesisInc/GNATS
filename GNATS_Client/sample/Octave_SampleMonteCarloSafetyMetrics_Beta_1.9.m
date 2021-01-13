%{
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
%}

%{
            NATIONAL AIRSPACE TRAJECTORY-PREDICTION SYSTEM (GNATS)
          Copyright 2018 by Optimal Synthesis Inc. All rights reserved
          
Author: Hari Iyer
Date: 02/02/2019

Update: 03/12/2020  Oliver Chen
%}

source("sample/GNATS_Octave_Header_Client.m");

%Please enter GNATS HOME location
%GNATS_HOME = 'PLEASE_ENTER_GNATS_HOME_LOCATION_HERE';
GNATS_HOME = '.';

fprintf('This module illustrates a Monte Carlo Simulation template to observe changes to flight trajectory when the SafetyMetricsInterface functions are used to perturb flight parameters.\nGraphs for vital flight parameters are plotted after the simulation runs through.\n');

%Flight parameters to perturb, any of the following attributes can be inserted.
%PERTURB_ATTRIBUTES = {'AIRSPEED', 'ALTITUDE', 'WAYPOINT_LATITUDE_AND_LONGITUDE', 'WAYPOINT_LONGITUDE', 'WAYPOINT_LATITUDE', 'CURRENT_LATITUDE', 'CURRENT_LONGITUDE', 'CURRENT_LATITUDE_AND_LONGITUDE'};
PERTURB_ATTRIBUTES = {};

%{
%Monte-Carlo Simulation setup for perturbing true air speed (knots)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
meanAirspeedSpeed = 450;
sampleSize = 10;
airspeedVector = meanAirspeedSpeed + 10*randn(sampleSize,1);

%Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
meanCourse = 3.14;
sampleSize = 10;
courseVector = meanCourse + 0.1*randn(sampleSize,1);

%Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
meanRocd = 25;
sampleSize = 10;
rocdVector = meanRocd + randn(sampleSize,1);

%Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
waypointIndex = 8;
meanLatitude = 34.422439;
meanLongitude = -118.025853;
sampleSize = 10;
latitudeVector = meanLatitude + 0.1*randn(sampleSize,1);
longitudeVector = meanLongitude + 0.1*randn(sampleSize,1);
%}

%Monte-Carlo Simulation setup for perturbing altitude (feet)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1898';
meanAltitude = 25000;
sampleSize = 1;
altitudeVector = meanAltitude + 1000*randn(sampleSize,1);

%{
%Monte-Carlo Simulation setup for perturbing geo coordinates (latitude)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
waypointIndex = 8;
meanLatitude = 34.422439;
sampleSize = 10;
latitudeVector = meanLatitude + 0.1*randn(sampleSize,1);

%Monte-Carlo Simulation setup for perturbing geo coordinates (longitude)
%Please enter target aircraft callsign here. SWA1897 is an example.
aircraftID = 'SWA1897';
waypointIndex = 8;
meanLongitude = -118.025853;
sampleSize = 10;
longitudeVector = meanLongitude + 0.1*randn(sampleSize,1);
%}

%Monte-Carlo Simulation
for count = 1:sampleSize
    %Clear Trajectory data from previous iteration
    simulation.clear_trajectory();
    
    %Load/Reload wind and flight data since trajectory data had to be cleared from previous iteration
    environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
    aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_geo.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_mfl.trx'));    
    
    %Set up simulation (Versions of setupSimulation can be found in the
    %documentation)
    simulation.setupSimulation(11000, 30);
    %Start simulation to go through till 1000 seconds. This time can be
    %changed, and the simulation would pause at the provided time.
    simulation.start(1000.0);
    
    %Wait for simulation to pause
    while 1
        simStatus = simulation.get_runtime_sim_status();
        if (simStatus ~= GNATS_SIMULATION_STATUS_PAUSE)
           pause(1);
        else
            break;
        end
    end
    
    %Get current state of aircraft
    aircraft = aircraftInterface.select_aircraft(aircraftID);

    if (ismember('AIRSPEED',PERTURB_ATTRIBUTES))
        %aircraft.getTas_knots();
        aircraft.setTas_knots(airspeedVector(count));
    end
    if (ismember('ALTITUDE',PERTURB_ATTRIBUTES))
        %aircraft.getAltitude_ft();
        aircraft.setAltitude_ft(altitudeVector(count));
    end
    if (ismember('WAYPOINT_LATITUDE',PERTURB_ATTRIBUTES))
        aircraft.setFlight_plan_latitude_deg(waypointIndex, latitudeVector(count));
    end
    if (ismember('WAYPOINT_LONGITUDE',PERTURB_ATTRIBUTES))
       aircraft.setFlight_plan_longitude_deg(waypointIndex, longitudeVector(count));
    end
    if (ismember('WAYPOINT_LATITUDE_AND_LONGITUDE',PERTURB_ATTRIBUTES))
       aircraft.setFlight_plan_latitude_deg(waypointIndex,  latitudeVector(count));
       aircraft.setFlight_plan_longitude_deg(waypointIndex, longitudeVector(count));
    end

    if (ismember('CURRENT_LATITUDE',PERTURB_ATTRIBUTES))
       %aircraft.getLatitude_deg();
       aircraft.setLatitude_deg(latitudeVector(count));
    end
    if (ismember('CURRENT_LONGITUDE',PERTURB_ATTRIBUTES))
       %aircraft.getLongitude_deg();
        aircraft.setLongitude_deg(longitudeVector(count));
    end
    if (ismember('CURRENT_LATITUDE_AND_LONGITUDE',PERTURB_ATTRIBUTES))
        aircraft.setLatitude_deg(latitudeVector(count));
        aircraft.setLongitude_deg(longitudeVector(count));
    end
    
    %Safety Metrics Functions. The Safety Metrics Interface would be invoked if any
    %of these functions are called. An example for retrieving flights in safety 
    %bounding box has been shown here. The flights in this bounding box will vary 
    %based on the parameters perturbed above.
    %THESE PERTURBATIONS MIGHT LEAD TO ERRONEOUS FLIGHT SIMULATION RESULTS
    disp(safetyMetricsInterface.getFlightsInRange('SWA1897'));
    %safetyMetricsInterface.getDistanceToRunwayThreshold('SWA1897')
    %safetyMetricsInterface.getDistanceToRunwayEnd('SWA1897')
    %safetyMetricsInterface.getVelocityAlignmentWithRunway('SWA1897', 'ARRIVAL')
    %safetyMetricsInterface.getPassengerCount('A306')
    %safetyMetricsInterface.getAircraftCost('A306')

    simulation.resume();
    
    %Wait for simulation to get done
    while 1
        simStatus = simulation.get_runtime_sim_status();
        if (simStatus ~= GNATS_SIMULATION_STATUS_ENDED)
           pause(1);
        else
            break;
        end
    end
   
    %Generating and saving new trajectory
    trajectoryFile = sprintf([char(aircraft.getAcid()) '-Octave-Monte-Carlo-Sim-Trajectory_%s.csv'], num2str(count));
    simulation.write_trajectories([strcat(DIR_share, 'mcSimulation/') trajectoryFile]);

    %Wait for updated trajectory to get written
    while true
        if ~isempty(dir([GNATS_Server '/share/mcSimulation/',trajectoryFile]))
            pause(1);
        else
            break;
        end
    end

    %Clear simulation data for next iteration
    aircraftInterface.release_aircraft();
    pause(1.5);
    environmentInterface.release_rap();
    pause(1.5);
    
end

%Graph plotting routine invoked for visualizing vital flight parameter data 
PlotGraph(aircraftID, sampleSize, "Octave", GNATS_Server);

gnatsClient.disConnect()
