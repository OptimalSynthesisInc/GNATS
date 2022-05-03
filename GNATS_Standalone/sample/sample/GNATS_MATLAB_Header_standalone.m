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
