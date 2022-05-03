from GNATS_Python_Header_standalone import *

print('This module illustrates the TerrainInterface functions to get elevation information. The trajectory is then plotted on Google Maps.')

simulationInterface.clear_trajectory()
environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_mfl.trx")

simulationInterface.setupSimulation(12000, 30) # SFO - PHX

simulationInterface.start(600)

#Set aircraft to be simulated
aircraftCallsign = 'SWA1897'

# Use a while loop to constantly check simulation status
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

# Set terrain functions
# Some function may stop the aircraft from continuing moving forward.  If it happens, the plotting of the result trajectory will show nothing.
print("setTerrainProfile return value = ", (terrainInterface.setTerrainProfile(-56, 75, -180, 180, 0.1)))
print("getElevationAreaStats return value = ", (terrainInterface.getElevationAreaStats(1, 2, -161, -160)))
print("getElevation return value = ", (terrainInterface.getElevation(-55.2, -179.6)))
print("getElevationMapBounds return value = ", (list(terrainInterface.getElevationMapBounds()[3])))

simulationInterface.resume()

while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_ENDED) :
        break
    else:
        time.sleep(1)

millis = int(round(time.time() * 1000))
print("Outputting trajectory data.  Please wait....")
fileName = os.path.splitext(os.path.basename(__file__))[0] + "_" + str(millis) + ".csv"
# Output the trajectory result file
simulationInterface.write_trajectories(fileName)

aircraftInterface.release_aircraft()
environmentInterface.release_rap()

# Stop GNATS Standalone environment
gnatsStandalone.stop()

shutdownJVM()

#Plot trajectory on Google Maps
PathVisualizer.plotOnGoogleMap([aircraftCallsign], fileName)
