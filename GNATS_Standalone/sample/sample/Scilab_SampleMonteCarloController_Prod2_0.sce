//{
//          Generalized NATIONAL AIRSPACE TRAJECTORY-PREDICTION SYSTEM (GNATS)
//          Copyright 2018 by Optimal Synthesis Inc. All rights reserved
//          
//Author: Hari Iyer
//Date: 02/07/2019
//
//Update: 02/26/2020  Oliver Chen
//}

warning('off');

// Please enter GNATS HOME location here
//GNATS_HOME = 'PLEASE_ENTER_GNATS_HOME_LOCATION_HERE';
GNATS_HOME = '.';

DIR_share = '../GNATS_Server/share';

if (~isdir(GNATS_HOME)) then
    disp("Directory GNATS_HOME does not exist");
else
    chdir(GNATS_HOME);
    
    javaclasspath(GNATS_HOME + '/dist/gnats-standalone.jar');
    javaclasspath(GNATS_HOME + '/../GNATS_Client/dist/gnats-client.jar');
    javaclasspath(GNATS_HOME + '/../GNATS_Client/dist/gnats-shared.jar');
    javaclasspath(GNATS_HOME + '/../GNATS_Client/dist/json.jar');
    javaclasspath(GNATS_HOME + '/../GNATS_Client/dist/commons-logging-1.2.jar');
    
    SimulationStatus = jimport('com.osi.util.Constants', %f);
    AircraftClearance = jimport('com.osi.util.AircraftClearance', %f);
    FlightPhase = jimport('com.osi.util.FlightPhase', %f);

    GNATSStandalone = jimport('GNATSStandalone', %f);
    
    //Flight Phases
    FLIGHT_PHASE_PREDEPARTURE = jgetfield(FlightPhase,"FLIGHT_PHASE_PREDEPARTURE");
    FLIGHT_PHASE_ORIGIN_GATE = jgetfield(FlightPhase,"FLIGHT_PHASE_ORIGIN_GATE");
    FLIGHT_PHASE_PUSHBACK = jgetfield(FlightPhase,"FLIGHT_PHASE_PUSHBACK");
    FLIGHT_PHASE_RAMP_DEPARTING = jgetfield(FlightPhase,"FLIGHT_PHASE_RAMP_DEPARTING");
    FLIGHT_PHASE_TAXI_DEPARTING = jgetfield(FlightPhase,"FLIGHT_PHASE_TAXI_DEPARTING");
    FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING = jgetfield(FlightPhase,"FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING");
    FLIGHT_PHASE_TAKEOFF = jgetfield(FlightPhase,"FLIGHT_PHASE_TAKEOFF");
    FLIGHT_PHASE_CLIMBOUT = jgetfield(FlightPhase,"FLIGHT_PHASE_CLIMBOUT");
    FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN = jgetfield(FlightPhase,"FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN");
    FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE = jgetfield(FlightPhase,"FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE");
    FLIGHT_PHASE_TOP_OF_CLIMB = jgetfield(FlightPhase,"FLIGHT_PHASE_TOP_OF_CLIMB");
    FLIGHT_PHASE_CRUISE = jgetfield(FlightPhase,"FLIGHT_PHASE_CRUISE");
    FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN = jgetfield(FlightPhase,"FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN");
    FLIGHT_PHASE_TOP_OF_DESCENT = jgetfield(FlightPhase,"FLIGHT_PHASE_TOP_OF_DESCENT");
    FLIGHT_PHASE_INITIAL_DESCENT = jgetfield(FlightPhase,"FLIGHT_PHASE_INITIAL_DESCENT");
    FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN = jgetfield(FlightPhase,"FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN");
    FLIGHT_PHASE_APPROACH = jgetfield(FlightPhase,"FLIGHT_PHASE_APPROACH");
    FLIGHT_PHASE_FINAL_APPROACH = jgetfield(FlightPhase,"FLIGHT_PHASE_FINAL_APPROACH");
    FLIGHT_PHASE_GO_AROUND = jgetfield(FlightPhase,"FLIGHT_PHASE_GO_AROUND");
    FLIGHT_PHASE_TOUCHDOWN = jgetfield(FlightPhase,"FLIGHT_PHASE_TOUCHDOWN");
    FLIGHT_PHASE_LAND = jgetfield(FlightPhase,"FLIGHT_PHASE_LAND");
    FLIGHT_PHASE_EXIT_RUNWAY = jgetfield(FlightPhase,"FLIGHT_PHASE_EXIT_RUNWAY");
    FLIGHT_PHASE_TAXI_ARRIVING = jgetfield(FlightPhase,"FLIGHT_PHASE_TAXI_ARRIVING");
    FLIGHT_PHASE_RUNWAY_CROSSING = jgetfield(FlightPhase,"FLIGHT_PHASE_RUNWAY_CROSSING");
    FLIGHT_PHASE_RAMP_ARRIVING = jgetfield(FlightPhase,"FLIGHT_PHASE_RAMP_ARRIVING");
    FLIGHT_PHASE_DESTINATION_GATE = jgetfield(FlightPhase,"FLIGHT_PHASE_DESTINATION_GATE");
    FLIGHT_PHASE_LANDED = jgetfield(FlightPhase,"FLIGHT_PHASE_LANDED");
    FLIGHT_PHASE_HOLDING = jgetfield(FlightPhase,"FLIGHT_PHASE_HOLDING");
    
    //Aircraft Clearance variables
    AIRCRAFT_CLEARANCE_PUSHBACK = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_PUSHBACK");
    AIRCRAFT_CLEARANCE_TAXI_DEPARTING = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_TAXI_DEPARTING");
    AIRCRAFT_CLEARANCE_TAKEOFF = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_TAKEOFF");
    AIRCRAFT_CLEARANCE_ENTER_ARTC = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_ENTER_ARTC");
    AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE");
    AIRCRAFT_CLEARANCE_ENTER_TRACON = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_ENTER_TRACON");
    AIRCRAFT_CLEARANCE_APPROACH = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_APPROACH");
    AIRCRAFT_CLEARANCE_TOUCHDOWN = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_TOUCHDOWN");
    AIRCRAFT_CLEARANCE_TAXI_LANDING = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_TAXI_LANDING");
    AIRCRAFT_CLEARANCE_RAMP_LANDING = jgetfield(AircraftClearance, "AIRCRAFT_CLEARANCE_RAMP_LANDING");
    
    //GNATS simulation status definition
    //You can get simulation status and know what it refers to
    GNATS_SIM_STATUS_READY = jgetfield(SimulationStatus, "DEFAULT_GRAVITY");
    GNATS_SIM_STATUS_START = jgetfield(SimulationStatus, "GNATS_SIMULATION_STATUS_START");
    GNATS_SIM_STATUS_PAUSE = jgetfield(SimulationStatus, "GNATS_SIMULATION_STATUS_PAUSE");
    GNATS_SIM_STATUS_RESUME = jgetfield(SimulationStatus, "GNATS_SIMULATION_STATUS_RESUME");
    GNATS_SIM_STATUS_STOP = jgetfield(SimulationStatus, "GNATS_SIMULATION_STATUS_STOP");
    GNATS_SIM_STATUS_ENDED = jgetfield(SimulationStatus, "GNATS_SIMULATION_STATUS_ENDED");
    
    // Initialize GNATS
    gnatsStandalone = GNATSStandalone.start();
    if isempty(gnatsStandalone)
        printf("Can''t start GNATS Standalone\n");
        
        return;
    end
    
    simulationInterface = gnatsStandalone.getSimulationInterface();

    equipmentInterface = gnatsStandalone.getEquipmentInterface();
    entityInterface = gnatsStandalone.getEntityInterface();
    aircraftInterface = equipmentInterface.getAircraftInterface();
    
    environmentInterface = gnatsStandalone.getEnvironmentInterface();
    airportInterface = environmentInterface.getAirportInterface();
    
    pilotInterface = entityInterface.getPilotInterface();
    controllerInterface = entityInterface.getControllerInterface();
    
    //Flight parameters to perturb, any of the following attributes can be inserted.
    //PERTURB_ATTRIBUTES = 'AIRSPEED,ALTITUDE,WAYPOINT_LATITUDE_AND_LONGITUDE,WAYPOINT_LONGITUDE,WAYPOINT_LATITUDE,CURRENT_LATITUDE,CURRENT_LONGITUDE,CURRENT_LATITUDE_AND_LONGITUDE';
    PERTURB_ATTRIBUTES = '';
    
    //{
    //Monte-Carlo Simulation setup for perturbing altitude (feet)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'SWA1897';
    //meanAltitude = 25000;
    //sampleSize = 10;
    //altitudeVector = meanAltitude + 1000*int8(rand(sampleSize,1,'normal'));
    //
    //Monte-Carlo Simulation setup for perturbing true air speed (knots)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'SWA1897';
    //meanAirspeedSpeed = 450;
    //sampleSize = 10;
    //airspeedVector = meanAirspeedSpeed + 10*int8(rand(sampleSize,1,'normal'));
    //
    //Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'SWA1897';
    //meanCourse = 3.14;
    //sampleSize = 10;
    //courseVector = meanCourse + 0.1*int8(rand(sampleSize,1,'normal'));
    //
    //Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'SWA1897';
    //meanRocd = 25;
    //sampleSize = 10;
    //rocdVector = meanRocd + int8(rand(sampleSize,1,'normal'));
    //
    //Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'SWA1897';
    //waypointIndex = 8;
    //meanLatitude = 34.422439;
    //meanLongitude = -118.025853;
    //sampleSize = 10;
    //latitudeVector = meanLatitude + 0.1*int8(rand(sampleSize,1,'normal'));
    //longitudeVector = meanLongitude + 0.1*int8(rand(sampleSize,1,'normal'));
    //}
    
    //Monte-Carlo Simulation setup that perturbs number of time steps for controller absence
    //Please enter target aircraft callsign here. SWA1897 is an example.
    aircraftID = 'SWA1897';
    meanTimeStep = 6;
    sampleSize = 10;
    timeStepVector = meanTimeStep + int8(rand(sampleSize,1,'normal'));
    
    //{
    //Monte-Carlo Simulation setup for perturbing geo coordinates (latitude)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE';
    //waypointIndex = 8;
    //meanLatitude = 34.422439;
    //sampleSize = 10;
    //latitudeVector = meanLatitude + 0.1*int8(rand(sampleSize,1,'normal'));
    
    //Monte-Carlo Simulation setup for perturbing geo coordinates (longitude)
    //Please enter target aircraft callsign here. SWA1897 is an example.
    //aircraftID = 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE';
    //waypointIndex = 8;
    //meanLongitude = -118.025853;
    //sampleSize = 10;
    //longitudeVector = meanLongitude + 0.1*int8(rand(sampleSize,1,'normal'));
    //}
    
    //Monte-Carlo Simulation
    for count = 1:sampleSize
        //Clear Trajectory data from previous iteration
        simulationInterface.clear_trajectory();
        
        //Load/Reload wind and flight data since trajectory data had to be cleared from previous iteration
        environmentInterface.load_rap(DIR_share + '/tg/rap');
        aircraftInterface.load_aircraft(DIR_share + '/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx', DIR_share + '/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx');
        
        //Set up simulation (Versions of setupSimulation can be found in the
        //documentation)
        simulationInterface.setupSimulation(12000, 30);
        //Start simulation to go through till 1000 seconds. This time can be
        //changed, and the simulation would sleep at the provided time.
        simulationInterface.start(1000.0);
        
        //Wait for simulation to sleep
        while 1
            simStatus = simulationInterface.get_runtime_sim_status();
            if (simStatus ~= GNATS_SIM_STATUS_PAUSE)
               sleep(1000);
            else
                break;
            end
        end
        
        //Get current state of aircraft
        aircraft = aircraftInterface.select_aircraft(aircraftID);
        
        if (strstr(PERTURB_ATTRIBUTES, 'AIRSPEED') ~= '')
            //aircraft.getTas();
            aircraft.setTas_knots(airspeedVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'ALTITUDE') ~= '')
            //aircraft.getAltitude();
            aircraft.setAltitude_ft(altitudeVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'WAYPOINT_LATITUDE') ~= '')
            aircraft.setFlight_plan_latitude_deg(waypointIndex, latitudeVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'WAYPOINT_LONGITUDE') ~= '')
           aircraft.setFlight_plan_longitude_deg(waypointIndex, longitudeVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'WAYPOINT_LATITUDE_AND_LONGITUDE') ~= '')
           aircraft.setFlight_plan_latitude_deg(waypointIndex,  latitudeVector(count));
           aircraft.setFlight_plan_longitude_deg(waypointIndex, longitudeVector(count));
        end
    
        if (strstr(PERTURB_ATTRIBUTES, 'CURRENT_LATITUDE') ~= '')
           //aircraft.getLatitude();
           aircraft.setLatitude_deg(latitudeVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'CURRENT_LONGITUDE') ~= '')
           //aircraft.getLongitude();
            aircraft.setLongitude_deg(longitudeVector(count));
        end
        if (strstr(PERTURB_ATTRIBUTES, 'CURRENT_LATITUDE_AND_LONGITUDE') ~= '')
            aircraft.setLatitude_deg(latitudeVector(count));
            aircraft.setLongitude_deg(longitudeVector(count));
        end
        
        //Controller Error Model Functions. The controller model would be invoked if any
        //of these functions are called. An example for setting controller absence has 
        //been shown here. Similarly, other attributes can also be perturbed.
        //THESE PERTURBATIONS MIGHT LEAD TO ERRONEOUS FLIGHT SIMULATION RESULTS
        //controllerInterface.setDelayPeriod('SWA1897', AIRCRAFT_CLEARANCE_PUSHBACK, 20);
        //controllerInterface.skipFlightPhase('SWA1897', 'FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE');
        //controllerInterface.setActionRepeat('SWA1897', 'VERTICAL_SPEED');
        //controllerInterface.setWrongAction('SWA1897', 'AIRSPEED', 'FLIGHT_LEVEL');
        //controllerInterface.setActionReversal('SWA1897', 'VERTICAL_SPEED');
        //controllerInterface.setPartialAction('SWA1897', 'COURSE', 200, 50);
        //controllerInterface.skipChangeAction('SWA1897', 'COURSE');
        //controllerInterface.setActionLag('SWA1897', 'COURSE', 90, 0.05, -110);
        controllerInterface.setControllerAbsence('SWA1897', timeStepVector(count));
    
        simulationInterface.resume();
        
        //Wait for simulation to get done  
        while 1
            simStatus = simulationInterface.get_runtime_sim_status();
            if (simStatus ~= GNATS_SIM_STATUS_ENDED)
               sleep(1000);
            else
                break;
            end
        end
       
        //Generating and saving new trajectory
        trajectoryFile = msprintf(strcat([aircraft.getAcid(), '-Scilab-Monte-Carlo-Sim-Trajectory_', string(count),'.csv']));
        simulationInterface.write_trajectories(strcat([DIR_share '/mcSimulation/' trajectoryFile]));
    
        //Wait for updated trajectory to get written
        sleep(2000);
    
        //Clear simulation data for next iteration
        aircraftInterface.release_aircraft();
        sleep(1500);
        environmentInterface.release_rap();
        sleep(1500);
        
    end
    
    // Stop GNATS Standalone environment
    gnatsStandalone.stop();
    
    //Plot graph for the Monte Carlo Simulation output
    trajectoryCount = sampleSize
    
    //Iterate through the number of trajectories to be plotted
    for currentTrajectory = 1:trajectoryCount
        //Read trajectory files in  to a table and convert it to an array.
        csvFile = mopen(strcat([DIR_share '/mcSimulation/' aircraftID '-Scilab-Monte-Carlo-Sim-Trajectory_' string(currentTrajectory) '.csv']),'r');
        FlightParameters = mgetl(csvFile);
        lengthFlightParameters = size(FlightParameters, 'r') - 10;
        
        //Initialize flight parameter variables
        trueAirSpeedReadings = zeros(lengthFlightParameters, 1);
        altitudeReadings = zeros(lengthFlightParameters, 1);
        timestampReadings = zeros(lengthFlightParameters, 1);
        latitudeReadings = zeros(lengthFlightParameters, 1);
        longitudeReadings = zeros(lengthFlightParameters, 1);
        rocdReadings = zeros(lengthFlightParameters, 1);
        courseReadings = zeros(lengthFlightParameters, 1);
    
        startIndex = 1;
        //Get relevant flight data starting line number from simulation CSV output
        for count=1:size(FlightParameters, 'r')
            trajectoryPoint = strsplit(char(FlightParameters(count)), ',');
            if(size(trajectoryPoint, 'r') > 3)
                if(~strcmp(string(trajectoryPoint(3)), aircraftID))
                    startIndex = count + 1;
                end
            end
        end
    
        trajectoryPoint = strsplit(char(FlightParameters(startIndex)), ',');
    
            
        //Traverse through trajectory points and read flight data
        for count=startIndex:size(FlightParameters, 'r')
            trajectoryPoint = strsplit(string(FlightParameters(count)), ',');
            if(~strcmp(string(trajectoryPoint(1)), ''))
                break;
            end
    
            trueAirSpeedReadings(count - 8,1) = strtod(string(trajectoryPoint(6)));
            altitudeReadings(count - 8,1) = strtod(string(trajectoryPoint(4)));
            timestampReadings(count - 8,1) = strtod(string(trajectoryPoint(1)));
            latitudeReadings(count - 8,1) = strtod(string(trajectoryPoint(2)));
            longitudeReadings(count - 8,1) = strtod(string(trajectoryPoint(3)));
            rocdReadings(count - 8,1) = strtod(string(trajectoryPoint(5)));
            courseReadings(count - 8,1) = strtod(string(trajectoryPoint(7)));
        end
    
        //Convert cells to matrix in order to plot graphs
        trueAirSpeedMatrix = trueAirSpeedReadings;
        altitudeMatrix = altitudeReadings;
        timestampMatrix = timestampReadings;
        latitudeMatrix = latitudeReadings;
        longitudeMatrix = longitudeReadings;
        rocdMatrix = rocdReadings;
        courseMatrix = courseReadings;
    
        //Graph visualization begins, this can be used as template for custom plots
    
    
        //Plot altitude vs timestamp
        subplot(3, 2, 1);
        title( 'Altitude vs. Time');
        plot(timestampMatrix, altitudeMatrix);
        xlabel('Time (s)');
        ylabel('Altitude (feet)');
        //Optionally limit the x or y axis limits to focus on part of graph
        //xlim([9000 11000])
    
        //Plot true arispeed vs timestamp
        subplot(3, 2, 2);
        title( 'True Airspeed vs. Time');
        plot(timestampMatrix, trueAirSpeedMatrix);
        xlabel('Time (s)');
        ylabel('True Airspeed (knots)');
        //xlim([9000 11000])
    
        //Plot latitude vs longitude
        subplot(3, 2, 3);
        title( 'Latitude vs. Longitude');
        plot(longitudeMatrix, latitudeMatrix);
        xlabel('Longitude (degree)');
        ylabel('Latitude (degree)');
        //xlim([-114 -110])
    
        //Plot rate of climb/descent vs timestamp
        subplot(3, 2, 4);
        title( 'Rate of Climb/Descent vs. Time');
        plot(timestampMatrix, rocdMatrix);
        xlabel('Time (s)');
        ylabel('Rate of Climb/Descent (feet/minute)');
        //xlim([9000 11000])
        
        //Plot course angle vs timestamp
        subplot(3, 2, 5);
        title( 'Course vs. Time');
        plot(timestampMatrix, courseMatrix);
        xlabel('Time (s)');
        ylabel('Course (degree)');
        //xlim([9000 11000])
        
        mclose(csvFile);
    
    end
end
