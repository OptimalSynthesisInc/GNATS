# This is the header file for running R examples

# Java initialization with rJava library, setting up JARs to access GNATS functions. rJava can be installed by running install.packages("rJava") in R shell.
library(rJava)

.jinit('.')
.jaddClassPath(paste('../GNATS_Client/dist/gnats-client.jar', sep = ""))
.jaddClassPath(paste('../GNATS_Client/dist/gnats-shared.jar', sep = ""))
.jaddClassPath(paste('dist/gnats-standalone.jar', sep = ""))
.jaddClassPath(paste('../GNATS_Client/dist/commons-logging-1.2.jar', sep = ""))
.jaddClassPath(paste('../GNATS_Client/dist/json.jar', sep = ""))
.jaddClassPath(paste('../GNATS_Client/dist/rmiio-2.1.2.jar', sep = ""))

# Initialize GNATS variables
aircraftClearance <- .jnew('com.osi.util.AircraftClearanceReference')
constants <- .jnew('com.osi.util.Constants')

DIR_share = '../GNATS_Server/share'

AIRCRAFT_CLEARANCE_PUSHBACK <- aircraftClearance$AIRCRAFT_CLEARANCE_PUSHBACK
AIRCRAFT_CLEARANCE_TAXI_DEPARTING <- aircraftClearance$AIRCRAFT_CLEARANCE_TAXI_DEPARTING
AIRCRAFT_CLEARANCE_TAKEOFF <- aircraftClearance$AIRCRAFT_CLEARANCE_TAKEOFF
AIRCRAFT_CLEARANCE_ENTER_ARTC <- aircraftClearance$AIRCRAFT_CLEARANCE_ENTER_ARTC
AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE <- aircraftClearance$AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE
AIRCRAFT_CLEARANCE_ENTER_TRACON <- aircraftClearance$AIRCRAFT_CLEARANCE_ENTER_TRACON
AIRCRAFT_CLEARANCE_APPROACH <- aircraftClearance$AIRCRAFT_CLEARANCE_APPROACH
AIRCRAFT_CLEARANCE_TOUCHDOWN <- aircraftClearance$AIRCRAFT_CLEARANCE_TOUCHDOWN
AIRCRAFT_CLEARANCE_TAXI_LANDING <- aircraftClearance$AIRCRAFT_CLEARANCE_TAXI_LANDING
AIRCRAFT_CLEARANCE_RAMP_LANDING <- aircraftClearance$AIRCRAFT_CLEARANCE_RAMP_LANDING


GNATS_SIMULATION_STATUS_READY <- constants$GNATS_SIMULATION_STATUS_READY
GNATS_SIMULATION_STATUS_START <- constants$GNATS_SIMULATION_STATUS_START
GNATS_SIMULATION_STATUS_PAUSE <- constants$GNATS_SIMULATION_STATUS_PAUSE
GNATS_SIMULATION_STATUS_RESUME <- constants$GNATS_SIMULATION_STATUS_RESUME
GNATS_SIMULATION_STATUS_STOP <- constants$GNATS_SIMULATION_STATUS_STOP
GNATS_SIMULATION_STATUS_ENDED <- constants$GNATS_SIMULATION_STATUS_ENDED

clsGNATSStandalone <- .jnew('GNATSStandalone')

# Start GNATS Standalone environment
gnatsStandalone <- clsGNATSStandalone$start()

# Initializing GNATS Interfaces and variables
simulationInterface <- gnatsStandalone$getSimulationInterface()
equipmentInterface <- gnatsStandalone$getEquipmentInterface()
entityInterface <- gnatsStandalone$getEntityInterface()
aircraftInterface <- equipmentInterface$getAircraftInterface()
environmentInterface <- gnatsStandalone$getEnvironmentInterface()
airportInterface <- environmentInterface$getAirportInterface()
terminalAreaInterface <- environmentInterface$getTerminalAreaInterface()
controllerInterface <- entityInterface$getControllerInterface()

if (is.null(gnatsStandalone)) {
    print("Can't start GNATS Standalone")
    quit()
}
