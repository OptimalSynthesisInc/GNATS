from GNATS_Python_Header_standalone import *

print('This module illustrates the Aircraft Load and Cost functions to model simulation situations. The trajectory is then plotted on Google Maps.')

simulationInterface.clear_trajectory()
environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx")

simulationInterface.setupSimulation(27000, 30) # SFO - PHX

simulationInterface.start(600)

#Set aircraft to be simulated
aircraftCallsign = 'SQ12'

# Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)

aircraft = aircraftInterface.select_aircraft('SQ12')

print("Before Calling Functions:")
print(("Aircraft Book Value: " + str(riskMeasuresInterface.getAircraftBookValue('SQ12'))))
print(("Aircraft Cargo Worth: " + str(riskMeasuresInterface.getCargoWorth('SQ12'))))
print(("Aircraft Passenger Load Factor: " + str(riskMeasuresInterface.getPassengerLoadFactor('SQ12'))))

riskMeasuresInterface.setAircraftBookValue('SQ12', 5.6)
riskMeasuresInterface.setCargoWorth('SQ12', 1.2)
riskMeasuresInterface.setPassengerLoadFactor('SQ12', 0.72)

print("\nAfter setting values:")
print(("Aircraft Book Value: " + str(riskMeasuresInterface.getAircraftBookValue('SQ12'))))
print(("Aircraft Cargo Worth: " + str(riskMeasuresInterface.getCargoWorth('SQ12'))))
print(("Aircraft Passenger Load Factor: " + str(riskMeasuresInterface.getPassengerLoadFactor('SQ12'))))

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
