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

fprintf('This module illustrates a Monte Carlo Simulation template to observe changes to flight trajectory when the ControllerInterface functions are used to perturb flight parameters.\nGraphs for vital flight parameters are plotted after the simulation runs through.\n');

if (~exist(GNATS_HOME, 'dir'))
    fprintf('\nDirectory GNATS_HOME does not exist\n');
else
% 	gnatsStandalone = GNATSStandalone.start();
% 	if isempty(gnatsStandalone)
%     	printf('Can''t start GNATS Standalone\n');
%     	return;
% 	end

% 	simulation = gnatsStandalone.getSimulationInterface();
% 
% 	environmentInterface = gnatsStandalone.getEnvironmentInterface();
% 
% 	equipmentInterface = gnatsStandalone.getEquipmentInterface();
% 	aircraftInterface = equipmentInterface.getAircraftInterface();
% 
% 	entityInterface = gnatsStandalone.getEntityInterface();
% 	controllerInterface = entityInterface.getControllerInterface();
% 	pilotInterface = entityInterface.getPilotInterface();
	
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

    %Monte-Carlo Simulation setup that perturbs number of time steps for controller absence
    %Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'SWA1897';
    meanTimeStep = 6;
    sampleSize = 10;
    timeStepVector = meanTimeStep + int8(randn(sampleSize,1));

    %{
    %Monte-Carlo Simulation setup for perturbing geo coordinates (latitude)
    %Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE';
    waypointIndex = 8;
    meanLatitude = 34.422439;
    sampleSize = 10;
    latitudeVector = meanLatitude + 0.1*randn(sampleSize,1);

    %Monte-Carlo Simulation setup for perturbing geo coordinates (longitude)
    %Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE';
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
        simulationInterface.setupSimulation(8000, 30);
        
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

        %Controller Error Model Functions. The controller model would be invoked if any
        %of these functions are called. An example for setting controller absence has 
        %been shown here. Similarly, other attributes can also be perturbed.
        %THESE PERTURBATIONS MIGHT LEAD TO ERRONEOUS FLIGHT SIMULATION RESULTS
        %controllerInterface.setDelayPeriod('SWA1897', AIRCRAFT_CLEARANCE_PUSHBACK, 20);
        %controllerInterface.skipFlightPhase('SWA1897', 'FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE');
        %controllerInterface.setActionRepeat('SWA1897', 'VERTICAL_SPEED');
        %controllerInterface.setWrongAction('SWA1897', 'AIRSPEED', 'FLIGHT_LEVEL');
        %controllerInterface.setActionReversal('SWA1897', 'VERTICAL_SPEED');
        %controllerInterface.setPartialAction('SWA1897', 'COURSE', 200, 50);
        %controllerInterface.skipChangeAction('SWA1897', 'COURSE');
        %controllerInterface.setActionLag('SWA1897', 'COURSE', 90, 0.05, -110);
        controllerInterface.setControllerAbsence('SWA1897', timeStepVector(count));

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
        simulationInterface.write_trajectories([strcat(DIR_share, '/mcSimulation/'), trajectoryFile]);

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
