# GNATS sample
#
# Optimal Synthesis Inc.
#
# Oliver Chen
# 03.12.2020
#
# Demo of changing aircraft state
#
# This program starts the simulation for a period of time then GNATS pause automatically.
# When the simulation is at pause status, we change the aircraft state.
# When the simulation resumes, it continues to run the rest of the simulation until it finishes.
#
# Users can compare the trajectory data to see the change of aircraft state.

from GNATS_Python_Header_standalone import *

simulationInterface.clear_trajectory()
environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx")

simulationInterface.setupSimulation(27000, 30)

simulationInterface.start(1020)

# Check simulation status.  Continue to next code when it is at PAUSE status
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

# =====================================================

# Required
# Every time at pause, get the latest aircraft data
curAircraft = aircraftInterface.select_aircraft("SQ12")
if not (curAircraft is None):
    print("Setting new state to aircraft SQ12")
    
    # Set new state value
    #curAircraft.setCruise_tas_knots(450)
    #curAircraft.setCruise_alt_ft(35000)

    curAircraft.setLatitude_deg(1.0)
    curAircraft.setLongitude_deg(105.0)

# =====================================================

simulationInterface.resume(7110) # Resume and continue the simulation

while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

# =====================================================

# Required
# Every time at pause, get the latest aircraft data
curAircraft = aircraftInterface.select_aircraft("SQ12")

# Set new state value
#curAircraft.setCruise_tas_knots(450)
#curAircraft.setCruise_alt_ft(35000)

#curAircraft.setLatitude_deg(36.0)
#curAircraft.setLongitude_deg(-120.0)
curAircraft.setAltitude_ft(32000.0)
curAircraft.setTas_knots(400.0)
curAircraft.setCourse_rad(110.0 * 3.1415926 / 180)
curAircraft.setRocd_fps(50.0)

# =====================================================

simulationInterface.resume()

# Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED) :
        break
    else:
        time.sleep(1)

millis = int(round(time.time() * 1000))
print("Outputting trajectory data.  Please wait....")
# Output the trajectory result file
simulationInterface.write_trajectories(os.path.splitext(os.path.basename(__file__))[0] + "_trajectory_" + str(millis) + ".csv")

aircraftInterface.release_aircraft()
environmentInterface.release_rap()


# Stop GNATS Standalone environment
gnatsStandalone.stop()

shutdownJVM()
