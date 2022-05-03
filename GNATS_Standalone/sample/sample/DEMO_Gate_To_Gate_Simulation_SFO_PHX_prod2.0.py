# GNATS sample
#
# Optimal Synthesis Inc.
#
# Oliver Chen
# 03.11.2020
#
# Demo of gate-to-gate trajectory simulation.
#
# The aircraft starts from the origin gate, goes through departing taxi plan, takes off, goes through different flight phases to the destination airport, and finally reaches the destination gate.

from GNATS_Python_Header_standalone import *

simulationInterface.clear_trajectory()

environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_SFO_PHX_GateToGate_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_SFO_PHX_mfl.trx")

#     # Controller to set human error: delay time
#     # Users can try the following setting and see the difference in trajectory
#controllerInterface.setDelayPeriod("SWA1897", AIRCRAFT_CLEARANCE_PUSHBACK, 7)
#controllerInterface.setDelayPeriod("SWA1897", AIRCRAFT_CLEARANCE_TAKEOFF, 20)

simulationInterface.setupSimulation(22000, 30) # SFO - PHX

simulationInterface.start(660)
           
# Use a while loop to constantly check simulation status.  When the simulation finishes, continue to output the trajectory data
while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
    if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
        break
    else:
        time.sleep(1)
 
# Pilot to set error scenarios
# Users can try the following setting and see the difference in trajectory
#pilotInterface.skipFlightPhase('SWA1897', 'FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE')
#pilotInterface.setActionRepeat('SWA1897', "VERTICAL_SPEED")
#pilotInterface.setWrongAction('SWA1897', "AIRSPEED", "FLIGHT_LEVEL")
#pilotInterface.setActionReversal('SWA1897', 'VERTICAL_SPEED')
#pilotInterface.setPartialAction('SWA1897', 'COURSE', 200, 50)
#pilotInterface.skipChangeAction('SWA1897', 'COURSE')
#pilotInterface.setActionLag('SWA1897', 'COURSE', 10, 0.05, 60)

simulationInterface.resume()

while True:
    runtime_sim_status = simulationInterface.get_runtime_sim_status()
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

# =========================================================
 
# The following statements read the result trajectory files and display plotting.
  
# Create temp directory and copy the result trajectory file into it
planned_dirname = "tmp_" + planned_dirname
os.makedirs(planned_dirname)
   
local_trajectory_filename = output_filename
   
copyfile(local_trajectory_filename, planned_dirname + "/" + local_trajectory_filename)
   
post_process = pp.PostProcessor(file_path = planned_dirname,
                                ac_name = 'SWA1897');
                                   
post_process.plotSingleAircraftTrajectory();
   
# Delete temp directory
print("Deleting directory:", planned_dirname)
rmtree(planned_dirname)

# Stop GNATS Standalone environment
gnatsStandalone.stop()

shutdownJVM()
