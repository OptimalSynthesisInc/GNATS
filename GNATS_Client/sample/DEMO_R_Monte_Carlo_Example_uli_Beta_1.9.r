
# This is a prototype example of an R program to access GNATS functions.
# It runs a Monte Carlo Simulation on controller absence time and saves plots of aircraft
# trajectory, speed, altitude, vertical speed with respect to time.

# The graphs are by default saved as a PDF file. If using RStudio, the graph plotting can be enhanced
# with interactive plotting.

# Please enter the location of GNATS Server on your local machine (Eg. /home/user/Desktop/GNATS_Server)
GNATS_Server <- 'PLEASE_ENTER_GNATS_SERVER_LOCATION_HERE'

# Please enter the location of GNATS Client on your local machine (Eg. /home/user/Desktop/GNATS_Client)
GNATS_Client <- 'PLEASE_ENTER_GNATS_CLIENT_LOCATION_HERE'

# Java initialization with rJava library, setting up JARs to access GNATS functions. rJava can be installed by running install.packages("rJava") in R shell.
library(rJava)
.jinit('.')
.jaddClassPath(paste(GNATS_Client, '/dist/gnats-client.jar', sep = ""))
.jaddClassPath(paste(GNATS_Client, '/dist/gnats-shared.jar', sep = ""))
.jaddClassPath(paste(GNATS_Client, '/dist/commons-logging-1.2.jar', sep = ""))
.jaddClassPath(paste(GNATS_Client, '/dist/json.jar', sep = ""))
.jaddClassPath(paste(GNATS_Client, '/dist/rmiio-2.1.2.jar', sep = ""))

gnatsClientFactory <- .jnew('GNATSClientFactory')
gnatsClient <- gnatsClientFactory$getGNATSClient()
aircraftClearance <- .jnew('com.osi.util.AircraftClearanceReference')
constants <- .jnew('com.osi.util.Constants')



# Initializing GNATS Interfaces and variables
simulationInterface <- gnatsClient$getSimulationInterface()
equipmentInterface <- gnatsClient$getEquipmentInterface()
entityInterface <- gnatsClient$getEntityInterface()
aircraftInterface <- equipmentInterface$getAircraftInterface()
environmentInterface <- gnatsClient$getEnvironmentInterface()
airportInterface <- environmentInterface$getAirportInterface()
terminalAreaInterface <- environmentInterface$getTerminalAreaInterface()
controllerInterface <- entityInterface$getControllerInterface()


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
 
# Attributes to perturb, can be any among 'AIRSPEED', 'ALTITUDE', 'WAYPOINT_LATITUDE_AND_LONGITUDE', 'WAYPOINT_LONGITUDE', 'WAYPOINT_LATITUDE', 'CURRENT_LATITUDE', 'CURRENT_LONGITUDE', 'CURRENT_LATITUDE_AND_LONGITUDE'

PERTURB_ATTRIBUTES <- list('ALTITUDE')
time <- c()
speed <- c()
altitude <- c()
rocd <- c()
course <- c()
latitude <- c()
longitude <- c()

#Monte-Carlo Simulation setup for perturbing true air speed (feet) and controller absense time (time steps)
aircraftID <- 'SWA1897'
meanAltitude <- 25000
sampleSize <- 10
meanTimeStep <- 6
timeStepVector = meanTimeStep + matrix(rnorm(sampleSize*1), sampleSize, 1)
altitudeVector = meanAltitude + 1000*matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing true air speed (knots)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'SWA1897'
#meanAirspeedSpeed <- 450
#sampleSize <- 10
#airspeedVector <- meanAirspeedSpeed + 10*matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'SWA1897'
#meanCourse <- 3.14
#sampleSize <- 10
#courseVector <- meanCourse + 0.1*matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'SWA1897'
#meanRocd <- 25
#sampleSize <- 10
#rocdVector <- meanRocd + matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing geo coordinates (latitude and longitude)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'SWA1897'
#waypointIndex <- 8
#meanLatitude <- 34.422439
#meanLongitude <- -118.025853
#sampleSize <- 10
#latitudeVector <- meanLatitude + 0.1*matrix(rnorm(sampleSize*1), sampleSize, 1)
#longitudeVector <- meanLongitude + 0.1*matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup that perturbs number of time steps for controller absence
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'SWA1897'
#meanTimeStep <- 6
#sampleSize <- 10
#timeStepVector <- meanTimeStep + matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing geo coordinates (latitude)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE'
#waypointIndex <- 8
#meanLatitude <- 34.422439
#sampleSize <- 10
#latitudeVector <- meanLatitude + 0.1*matrix(rnorm(sampleSize*1), sampleSize, 1)

#Monte-Carlo Simulation setup for perturbing geo coordinates (longitude)
#Please enter target aircraft callsign here. SWA1897 is an example.
#aircraftID <- 'PLEASE_ENTER_AIRCRAFT_CALLSIGN_HERE'
#waypointIndex <- 8
#meanLongitude <- -118.025853
#sampleSize <- 10
#longitudeVector <- meanLongitude + 0.1*matrix(rnorm(sampleSize*1), sampleSize, 1)


# Simulation begins
for (count in 1:sampleSize) {
	simulationInterface$clear_trajectory()
	environmentInterface$load_rap('share/tg/rap')
	aircraftInterface$load_aircraft('share/tg/trx/TRX_DEMO_SFO_PHX_GateToGate.trx', 'share/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx')
	simulationInterface$setupSimulation(as.integer(12000), as.integer(30))
	simulationInterface$start(.jlong(1000))

	while (1) {
            serverStatus = simulationInterface$get_runtime_sim_status()
	    if (serverStatus != GNATS_SIMULATION_STATUS_PAUSE) {
               Sys.sleep(1)
	    }
            else {
                break
            }
        }


        aircraft = aircraftInterface$select_aircraft(aircraftID)

	if ("AIRSPEED" %in% PERTURB_ATTRIBUTES) {
	    aircraft$setTas_knots(.jfloat(airspeedVector[[count]]))
	}

        if ("ALTITUDE" %in% PERTURB_ATTRIBUTES) {
            aircraft$setAltitude_ft(.jfloat(altitudeVector[[count]]))
	}
     

	controllerInterface$setControllerAbsence('SWA1897', as.integer(4))

	simulationInterface$resume()

        while (1) {
            serverStatus = simulationInterface$get_runtime_sim_status();
            if (serverStatus != GNATS_SIMULATION_STATUS_ENDED) {
               Sys.sleep(1)
	    }
            else {
                break
            }
        }

	trajectoryFile = sprintf("R-%s-Monte-Carlo-Sim-Trajectory_%s.csv", aircraftID, toString(count))
        simulationInterface$write_trajectories(paste('share/mcSimulation/', trajectoryFile, sep = ""))

        while(1) {
            if (file.exists(paste(GNATS_Server, '/share/mcSimulation/', trajectoryFile, sep = "")))
                break
            else
                Sys.sleep(1)
	}
        aircraftInterface$release_aircraft()
        Sys.sleep(1.5)
        environmentInterface$release_rap()
        Sys.sleep(1.5)

}

# Read output CSV file and plot
processFile = function(filepath) {
  count = 0
  con = file(filepath, "r")
  while ( TRUE ) {
    line = readLines(con, n = 1)
    if (count > 10) {
    if ( length(line) == 0 ) {
      break
    }
    rowValue = strsplit(line, "\\,")

    time <- c(time, as.numeric(rowValue[[1]][1]))
    speed <- c(speed, as.numeric(rowValue[[1]][6]))
    altitude <- c(altitude, as.numeric(rowValue[[1]][4]))
    rocd <- c(rocd, rowValue[[1]][5])
    course <- c(course, rowValue[[1]][8])
    latitude <- c(latitude, as.numeric(rowValue[[1]][2]))
    longitude <- c(longitude, as.numeric(rowValue[[1]][3]))
    }
    count = count + 1
  }
  close(con)

  csvName = strsplit(filepath, "\\/")
  plot.default(latitude, longitude, type="l", main=csvName[[1]][length(csvName[[1]])], xlab="Latitude (Degrees)", ylab="Longitude (Degrees)")
  plot(time, altitude, type="l", main=csvName[[1]][length(csvName[[1]])], xlab="Time (Seconds)", ylab="Altitude (Feet)")
  plot(time, speed, type="l", main=csvName[[1]][length(csvName[[1]])], xlab="Time (Seconds)", ylab="Speed (Knots)")
  plot(time, rocd, type="l", main=csvName[[1]][length(csvName[[1]])], xlab="Time (Seconds)", ylab="Rate of Climb/Descent (Feet per Second)")
  plot(time, course, type="l", main=csvName[[1]][length(csvName[[1]])], xlab="Time (Seconds)", ylab="Course (Degrees)")
}

# Loop through all the files written during simulation for graph plotting
for (i in 1:sampleSize) {
  processFile(paste(GNATS_Server, "/share/mcSimulation/R-", aircraftID, "-Monte-Carlo-Sim-Trajectory_", i, ".csv", sep = ""))
}
