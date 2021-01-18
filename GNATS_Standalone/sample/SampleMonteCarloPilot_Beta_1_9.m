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

%{
          Generalized NATIONAL AIRSPACE TRAJECTORY-PREDICTION SYSTEM (GNATS)
          Copyright 2018 by Optimal Synthesis Inc. All rights reserved
          
Author: Hari Iyer
Date: 01/04/2019

Updated: 03/12/2020  Oliver Chen
%}

% Run header module
run('GNATS_MATLAB_Header_standalone.m');

%Enter GNATS HOME location
%GNATS_HOME = 'PLEASE_ENTER_GNATS_HOME_LOCATION_HERE';
GNATS_HOME = '.';

fprintf('This module illustrates a Monte Carlo Simulation template to observe changes to flight trajectory when the PilotInterface functions are used to perturb flight parameters.\nGraphs for vital flight parameters are plotted after the simulation runs through.\n');

if (~exist(GNATS_HOME, 'dir'))
    fprintf('\nDirectory GNATS_HOME does not exist\n');
else
    %Flight parameters to perturb, any of the following attributes can be inserted.
    %PERTURB_ATTRIBUTES = {'AIRSPEED', 'ALTITUDE', 'WAYPOINT_LATITUDE_AND_LONGITUDE', 'WAYPOINT_LONGITUDE', 'WAYPOINT_LATITUDE', 'CURRENT_LATITUDE', 'CURRENT_LONGITUDE', 'CURRENT_LATITUDE_AND_LONGITUDE'};
    PERTURB_ATTRIBUTES = {};

    %{
    %Monte-Carlo Simulation setup for perturbing altitude (feet)
    %Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'SWA1897';
    meanAltitude = 25000;
    sampleSize = 10;
    altitudeVector = meanAltitude + 1000*randn(sampleSize,1);

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

    %Monte-Carlo Simulation setup for perturbing percentage parameter of
    %partial actions to be performed by the pilot (Part of Pilot Error Model).
    %Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'SWA1897';
    meanPercentage = 25;
    sampleSize = 10;
    partialActionPercentageVector = meanPercentage + randn(sampleSize,1);

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
        simulationInterface.clear_trajectory();

        %Load/Reload wind and flight data since trajectory data had to be cleared from previous iteration
        environmentInterface.load_rap(strcat(DIR_share, '/tg/rap'));
        aircraftInterface.load_aircraft(strcat(DIR_share, '/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx'), strcat(DIR_share, '/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx'));

        %Set up simulation (Versions of setupSimulation can be found in the
        %documentation)
        simulationInterface.setupSimulation(10000, 30);
        
        %Start simulation to go through till 1000 seconds. This time can be
        %changed, and the simulation would pause at the provided time.
        simulationInterface.start(1000.0);

        %Wait for simulation to pause
        while 1
            simStatus = simulationInterface.get_runtime_sim_status();
            if (simStatus ~= GNATS_SIMULATION_STATUS_PAUSE)
               pause(1);
            else
                break;
            end
        end

        %Get current state of aircraft
        aircraft = aircraftInterface.select_aircraft(aircraftID);

        if (ismember('AIRSPEED',PERTURB_ATTRIBUTES))
            aircraft.setTas_knots(airspeedVector(count));
        end
        if (ismember('ALTITUDE',PERTURB_ATTRIBUTES))
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
           aircraft.setLatitude_deg(latitudeVector(count));
        end
        if (ismember('CURRENT_LONGITUDE',PERTURB_ATTRIBUTES))
            aircraft.setLongitude_deg(longitudeVector(count));
        end
        if (ismember('CURRENT_LATITUDE_AND_LONGITUDE',PERTURB_ATTRIBUTES))
            aircraft.setLatitude_deg(latitudeVector(count));
            aircraft.setLongitude_deg(longitudeVector(count));
        end

        %Pilot Error Model Functions. The pilot model would be invoked if any
        %of these functions are called. An example for setting partial action
        %for pilot error has been shown here. Similarly, other attributes can
        %also be perturbed.
        %THESE PERTURBATIONS MIGHT LEAD TO ERRONEOUS FLIGHT SIMULATION RESULTS
        %pilotInterface.setActionReversal('SWA1897', 'COURSE')
        pilotInterface.setPartialAction('SWA1897', 'VERTICAL_SPEED', 200, partialActionPercentageVector(count));
        %pilotInterface.skipFlightPhase('SWA1897', 'FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE')
        %pilotInterface.setActionRepeat('SWA1897', 'COURSE')
        %pilotInterface.setWrongAction('SWA1897', 'COURSE', 'AIRSPEED');
        %pilotInterface.skipChangeAction('SWA1897', 'COURSE')
        %pilotInterface.setActionLag('SWA1897', 'COURSE', 10, 0.05, 30)
        %pilotInterface.setFlightPlanReadError('SWA1897', 'VERTICAL_SPEED', 398.0)

        simulationInterface.resume();

        %Wait for simulation to get done
        while 1
            simStatus = simulationInterface.get_runtime_sim_status();
            if (simStatus ~= GNATS_SIMULATION_STATUS_ENDED)
               pause(1);
            else
                break;
            end
        end

        %Generating and saving new trajectory
        trajectoryFile = sprintf([char(aircraft.getAcid()) '-Monte-Carlo-Sim-Trajectory_%s.csv'], num2str(count));
        simulationInterface.write_trajectories([strcat(DIR_share, '/mcSimulation/') trajectoryFile]);

        %Wait for updated trajectory to get written
        while true
            if ~isempty(dir([DIR_share '/mcSimulation/', trajectoryFile]))
                break;
            else
                pause(1);
            end
        end

        %Clear simulation data for next iteration
        aircraftInterface.release_aircraft();
        pause(1.5);
        environmentInterface.release_rap();
        pause(1.5);

    end
    
    % Stop GNATS Standalone environment
	gnatsStandalone.stop();

    %Graph plotting routine invoked for visualizing vital flight parameter data 
    PlotGraph(aircraftID, sampleSize, 'MATLAB', DIR_share);
end
