# NATS sample
#
# Optimal Synthesis Inc.
#
# Oliver Chen
# 03.11.2020
#
# Demo of aircraft-related functions.
#
# Users can learn how to obtain aircraft instance, show related aircraft information, start/pause/resume simulation and output result trajectory file.

from GNATS_Python_Header_standalone import *

import datetime

planned_dirname = ""
output_filename = ""

simulationInterface.clear_trajectory()

environmentInterface.load_rap(DIR_share + "/tg/rap")
aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx")

if not(aircraftInterface is None):
	aircraftIdArray_withinZone = aircraftInterface.getAircraftIds(float(30.0), float(35.0), float(-115.0), float(-90.0), float(-1.0), float(-1.0))
	if (not(aircraftIdArray_withinZone is None) and (len(aircraftIdArray_withinZone) > 0)) :
		i = 0
		for i in range(0, len(aircraftIdArray_withinZone)):
			curAcId = aircraftIdArray_withinZone[i]
			print("Aircraft id in selected zone = ", curAcId)
	
	print("****************************************")
	
	curAircraft = aircraftInterface.select_aircraft("SQ12")
	if not(curAircraft is None):
		airborne_flight_plan_waypoint_name_array = curAircraft.getFlight_plan_waypoint_name_array()
		for j in range(0, len(airborne_flight_plan_waypoint_name_array)) :
			print("SQ12 flight plan waypoint name = ", airborne_flight_plan_waypoint_name_array[j])
		print("")
		
		curAircraft.delay_departure(100) # Delay the departure time for 100 sec

simulationInterface.setupSimulation(43000, 10)

# Start the simulation for 1000 secs then pause
simulationInterface.start(1000)

while True:
	runtime_sim_status = simulationInterface.get_runtime_sim_status()
	#print("Time:", datetime.now(), ", simulation continuing.... ")
	if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
		break
	else:
		time.sleep(1)

curAircraft = aircraftInterface.select_aircraft("SQ12")
if not(curAircraft is None):
	print("****************************************")
	print("SQ12 (pausing at", simulationInterface.get_curr_sim_time(), "sec), latitude = ", curAircraft.getLatitude_deg(), ", longitude = ", curAircraft.getLongitude_deg(), ", altitude = ", curAircraft.getAltitude_ft())
	print("****************************************")

# Resume and continue the simulation
simulationInterface.resume()

# Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
while True:
	runtime_sim_status = simulationInterface.get_runtime_sim_status()
	#print("Time:", datetime.datetime.now(), ", simulation continuing.... ")
	if (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED) :
		break
	else:
		time.sleep(1)

millis = int(round(time.time() * 1000))
print("Outputting trajectory data.  Please wait....")

planned_dirname = os.path.splitext(os.path.basename(__file__))[0] + "_" + str(millis)
output_filename = planned_dirname + ".csv"

# Output the trajectory result file
simulationInterface.write_trajectories(output_filename)

aircraftInterface.release_aircraft()
environmentInterface.release_rap()


# Stop GNATS Standalone environment
gnatsStandalone.stop()

# =========================================================

# The following statements read the result trajectory files and display plotting.

# Create temp directory and copy the result trajectory file into it
planned_dirname = "tmp_" + planned_dirname
os.makedirs("../GNATS_Standalone/" + planned_dirname)
copyfile("../GNATS_Standalone/" + output_filename, "../GNATS_Standalone/" + planned_dirname + "/" + output_filename)

post_process = pp.PostProcessor(file_path = "../GNATS_Standalone/" + planned_dirname,
                                ac_name = 'SQ12');
post_process.plotSingleAircraftTrajectory();

# Delete temp directory
print("Deleting directory:", planned_dirname)
rmtree("../GNATS_Standalone/" + planned_dirname) 

shutdownJVM()