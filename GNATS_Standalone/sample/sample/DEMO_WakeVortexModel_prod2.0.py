'''
This module illustrates the function under SafetyMetricsInterface to model wake vortex. The trajectory is then plotted on Google Maps.
Details for the function usage can be found in the API documentation.
GNATS_Client/sample/WakeVortexEnvelope.png has an illustration for the same.
'''

from GNATS_Python_Header_standalone import *

print('This module illustrates the function under SafetyMetricsInterface to model wake vortex. The trajectory is then plotted on Google Maps.')

simulationInterface.clear_trajectory()
environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_SafetyMetrics_test_mfl.trx")

simulationInterface.setupSimulation(11000, 30)

simulationInterface.start(5000)

# Set aircraft to be simulated
aircraftCallsign = 'SWA1897'
aircraftInstance = aircraftInterface.select_aircraft(aircraftCallsign)

# Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

# Wake Vortex function, refer API documentation for details
# Function usage: getFlightsInWakeVortexRange(String refAircraftId, float envelopeStartLength, float envelopeStartBreadth, float envelopeEndLength, float envelopeEndBreadth, float envelopeRangeLength, float envelopeDecline)
aircraftInterface.select_aircraft("SWA1898").setLatitude_deg(aircraftInterface.select_aircraft("SWA1897").getLatitude_deg() - 0.0001)
aircraftInterface.select_aircraft("SWA1898").setLongitude_deg(aircraftInterface.select_aircraft("SWA1897").getLongitude_deg() - 0.0001)
aircraftInterface.select_aircraft("SWA1898").setCourse_rad(aircraftInterface.select_aircraft("SWA1897").getCourse_rad())

print((safetyMetricsInterface.getFlightsInWakeVortexRange('SWA1897', 5000, 2000, 400, 350, 5, 0)))

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
