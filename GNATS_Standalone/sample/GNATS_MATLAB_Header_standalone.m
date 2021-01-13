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

% Clear java variables in MATLAB
clear java;

env_GNATS_HOME = getenv('GNATS_HOME');

str_GNATS_HOME = '';

DIR_share = '../GNATS_Server/share';

if isempty(env_GNATS_HOME)
    str_GNATS_HOME = '';
else
    str_GNATS_HOME = strcat(env_GNATS_HOME, '/');
end

javaaddpath(strcat(str_GNATS_HOME, 'dist/gnats-standalone.jar'),'-end');
javaaddpath(strcat(str_GNATS_HOME, '../GNATS_Client/dist/gnats-shared.jar'),'-end');
javaaddpath(strcat(str_GNATS_HOME, '../GNATS_Client/dist/gnats-client.jar'),'-end');
javaaddpath(strcat(str_GNATS_HOME, '../GNATS_Client/dist/json.jar'),'-end');
javaaddpath(strcat(str_GNATS_HOME, '../GNATS_Client/dist/commons-logging-1.2.jar'),'-end');

% GNATS simulation status definition
% You can get simulation status and know what it refers to
GNATS_SIMULATION_STATUS_READY = com.osi.util.Constants.GNATS_SIMULATION_STATUS_READY;
GNATS_SIMULATION_STATUS_START = com.osi.util.Constants.GNATS_SIMULATION_STATUS_START;
GNATS_SIMULATION_STATUS_PAUSE = com.osi.util.Constants.GNATS_SIMULATION_STATUS_PAUSE;
GNATS_SIMULATION_STATUS_RESUME = com.osi.util.Constants.GNATS_SIMULATION_STATUS_RESUME;
GNATS_SIMULATION_STATUS_STOP = com.osi.util.Constants.GNATS_SIMULATION_STATUS_STOP;
GNATS_SIMULATION_STATUS_ENDED = com.osi.util.Constants.GNATS_SIMULATION_STATUS_ENDED;

% Aircraft Clearance variables
AIRCRAFT_CLEARANCE_PUSHBACK = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_PUSHBACK;
AIRCRAFT_CLEARANCE_TAXI_DEPARTING = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_TAXI_DEPARTING;
AIRCRAFT_CLEARANCE_TAKEOFF = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_TAKEOFF;
AIRCRAFT_CLEARANCE_ENTER_ARTC = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_ENTER_ARTC;
AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE;
AIRCRAFT_CLEARANCE_ENTER_TRACON = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_ENTER_TRACON;
AIRCRAFT_CLEARANCE_APPROACH = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_APPROACH;
AIRCRAFT_CLEARANCE_TOUCHDOWN = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_TOUCHDOWN;
AIRCRAFT_CLEARANCE_TAXI_LANDING = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_TAXI_LANDING;
AIRCRAFT_CLEARANCE_RAMP_LANDING = com.osi.util.AircraftClearance.AIRCRAFT_CLEARANCE_RAMP_LANDING;

%Flight Phases
FLIGHT_PHASE_PREDEPARTURE = com.osi.util.FlightPhase.FLIGHT_PHASE_PREDEPARTURE;
FLIGHT_PHASE_ORIGIN_GATE = com.osi.util.FlightPhase.FLIGHT_PHASE_ORIGIN_GATE;
FLIGHT_PHASE_PUSHBACK = com.osi.util.FlightPhase.FLIGHT_PHASE_PUSHBACK;
FLIGHT_PHASE_RAMP_DEPARTING = com.osi.util.FlightPhase.FLIGHT_PHASE_RAMP_DEPARTING;
FLIGHT_PHASE_TAXI_DEPARTING = com.osi.util.FlightPhase.FLIGHT_PHASE_TAXI_DEPARTING;
FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING = com.osi.util.FlightPhase.FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING;
FLIGHT_PHASE_TAKEOFF = com.osi.util.FlightPhase.FLIGHT_PHASE_TAKEOFF;
FLIGHT_PHASE_CLIMBOUT = com.osi.util.FlightPhase.FLIGHT_PHASE_CLIMBOUT;
FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN = com.osi.util.FlightPhase.FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN;
FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE = com.osi.util.FlightPhase.FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;
FLIGHT_PHASE_TOP_OF_CLIMB = com.osi.util.FlightPhase.FLIGHT_PHASE_TOP_OF_CLIMB;
FLIGHT_PHASE_CRUISE = com.osi.util.FlightPhase.FLIGHT_PHASE_CRUISE;
FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN = com.osi.util.FlightPhase.FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN;
FLIGHT_PHASE_TOP_OF_DESCENT = com.osi.util.FlightPhase.FLIGHT_PHASE_TOP_OF_DESCENT;
FLIGHT_PHASE_INITIAL_DESCENT = com.osi.util.FlightPhase.FLIGHT_PHASE_INITIAL_DESCENT;
FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN = com.osi.util.FlightPhase.FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN;
FLIGHT_PHASE_APPROACH = com.osi.util.FlightPhase.FLIGHT_PHASE_APPROACH;
FLIGHT_PHASE_FINAL_APPROACH = com.osi.util.FlightPhase.FLIGHT_PHASE_FINAL_APPROACH;
FLIGHT_PHASE_GO_AROUND = com.osi.util.FlightPhase.FLIGHT_PHASE_GO_AROUND;
FLIGHT_PHASE_TOUCHDOWN = com.osi.util.FlightPhase.FLIGHT_PHASE_TOUCHDOWN;
FLIGHT_PHASE_LAND = com.osi.util.FlightPhase.FLIGHT_PHASE_LAND;
FLIGHT_PHASE_EXIT_RUNWAY = com.osi.util.FlightPhase.FLIGHT_PHASE_EXIT_RUNWAY;
FLIGHT_PHASE_TAXI_ARRIVING = com.osi.util.FlightPhase.FLIGHT_PHASE_TAXI_ARRIVING;
FLIGHT_PHASE_RUNWAY_CROSSING = com.osi.util.FlightPhase.FLIGHT_PHASE_RUNWAY_CROSSING;
FLIGHT_PHASE_RAMP_ARRIVING = com.osi.util.FlightPhase.FLIGHT_PHASE_RAMP_ARRIVING;
FLIGHT_PHASE_DESTINATION_GATE = com.osi.util.FlightPhase.FLIGHT_PHASE_DESTINATION_GATE;
FLIGHT_PHASE_LANDED = com.osi.util.FlightPhase.FLIGHT_PHASE_LANDED;
FLIGHT_PHASE_HOLDING = com.osi.util.FlightPhase.FLIGHT_PHASE_HOLDING;

gnatsStandalone = GNATSStandalone.start();
if isempty(gnatsStandalone)
    printf('Can''t start GNATS Standalone\n');
    return;
end

simulationInterface = gnatsStandalone.getSimulationInterface();

environmentInterface = gnatsStandalone.getEnvironmentInterface();

equipmentInterface = gnatsStandalone.getEquipmentInterface();

aircraftInterface = equipmentInterface.getAircraftInterface();

entityInterface = gnatsStandalone.getEntityInterface();
controllerInterface = entityInterface.getControllerInterface();
pilotInterface = entityInterface.getPilotInterface();

airportInterface = environmentInterface.getAirportInterface();
terminalAreaInterface = environmentInterface.getTerminalAreaInterface();
riskMeasuresInterface = gnatsStandalone.getRiskMeasuresInterface();