from GNATS_Python_Header_standalone import *

print('This module illustrates the RiskMeasuresInterface functions to model simulation situations. The trajectory is then plotted on Google Maps.')

simulationInterface.clear_trajectory()
environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_RiskMeasures_test_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_2Aircrafts_RiskMeasures_test_mfl.trx")

# Controller to set human error: delay time
#controllerInterface.setDelayPeriod("SWA1897", AIRCRAFT_CLEARANCE_PUSHBACK, 7)
#controllerInterface.setDelayPeriod("SWA1897", AIRCRAFT_CLEARANCE_TAKEOFF, 20)

simulationInterface.setupSimulation(11000, 30) # SFO - PHX

simulationInterface.start(3090)

#Set aircraft to be simulated
aircraftCallsign = 'SWA1897'

# Use a while loop to constantly check simulation status.
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

print((riskMeasuresInterface.getFlightsInRange('SWA1897')))
#print(riskMeasuresInterface.getDistanceToRunwayThreshold('SWA1897'))
#print(riskMeasuresInterface.getDistanceToRunwayEnd('SWA1897'))
#print(riskMeasuresInterface.getVelocityAlignmentWithRunway('SWA1897', 'ARRIVAL'))
#print(riskMeasuresInterface.getPassengerCount('SA'))
#print(riskMeasuresInterface.getAircraftCost('SA'))

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
