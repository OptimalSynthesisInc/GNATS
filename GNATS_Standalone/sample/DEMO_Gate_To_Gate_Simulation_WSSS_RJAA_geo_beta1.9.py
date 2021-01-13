"""
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.
Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Drs. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.
---------------------------------------------------------------------------------------------------------------------
"""


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
