# GNATS sample
#
# Optimal Synthesis Inc.
#
# Oliver Chen
# 02.13.2020
#
# Demo of gate-to-gate trajectory simulation.
#
# The aircraft starts from the origin gate, goes through departing taxi plan, takes off, goes through different flight phases to the destination airport, and finally reaches the destination gate.

from GNATS_Python_Header_standalone import *

simulationInterface.clear_trajectory()

environmentInterface.load_rap(DIR_share + "/tg/rap")

aircraftInterface.load_aircraft(DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_geo.trx", DIR_share + "/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx")

simulationInterface.setupSimulation(86400, 30)

simulationInterface.start()

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
                                ac_name = 'SQ12');
                                    
post_process.plotSingleAircraftTrajectory();
    
# Delete temp directory
print("Deleting directory:", planned_dirname)
rmtree(planned_dirname)

# Stop GNATS Standalone environment
gnatsStandalone.stop()

shutdownJVM()
