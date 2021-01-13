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


# This sample program demonstrates several hold patterns during simulation.
#
# Oliver Chen
# 10.02.2019
#

from GNATS_Python_Header_client import *


planned_dirname = ""
output_filename = ""

if simulationInterface is None:
    print("Can't get SimulationInterface")

else:
    simulationInterface.clear_trajectory()
    environmentInterface.load_rap("share/tg/rap")

    aircraftInterface.load_aircraft("share/tg/trx/TRX_DEMO_WSSS_RJAA_Hold_Pattern_geo.trx", "share/tg/trx/TRX_DEMO_WSSS_RJAA_mfl.trx")
    
    simulationInterface.setupSimulation(35000, 30)

    simulationInterface.start(900)
    
    while True:
        runtime_sim_status = simulationInterface.get_runtime_sim_status()
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
            break
        else:
            time.sleep(1)
    
    # (1) Holding
    # Delay clearance of AIRCRAFT_CLEARANCE_ENTER_ARTC
    # This will cause aircraft stay in FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN
    controllerInterface.setDelayPeriod("SQ12", AIRCRAFT_CLEARANCE_ENTER_ARTC, 2000)
    
    simulationInterface.resume(22830)

    while True:
        runtime_sim_status = simulationInterface.get_runtime_sim_status()
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
            break
        else:
            time.sleep(1)
    
    # (2) Holding
    # Delay clearance of AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE
    # This will cause aircraft stay in FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN
    controllerInterface.setDelayPeriod("SQ12", AIRCRAFT_CLEARANCE_DESCENT_FROM_CRUISE, 1700)

    simulationInterface.resume(2670)
    
    while True:
        runtime_sim_status = simulationInterface.get_runtime_sim_status()
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
            break
        else:
            time.sleep(1)
    
    # (3) Holding
    # Delay clearance of AIRCRAFT_CLEARANCE_ENTER_TRACON
    # This will cause aircraft stay in FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN
    controllerInterface.setDelayPeriod("SQ12", AIRCRAFT_CLEARANCE_ENTER_TRACON, 1700)
    
    simulationInterface.resume(3600)
    
    while True:
        runtime_sim_status = simulationInterface.get_runtime_sim_status()
        if (runtime_sim_status == GNATS_SIMULATION_STATUS_PAUSE) :
            break
        else:
            time.sleep(1)
    
    # (4) Holding
    # When entering FINAL_APPROACH phase
    # Delay clearance of AIRCRAFT_CLEARANCE_TOUCHDOWN for certain time which aircraft continues to descent to the altitude close to 500 ft above destination airport elevation
    # This will cause aircraft to GO AROUND
    controllerInterface.setDelayPeriod("SQ12", AIRCRAFT_CLEARANCE_TOUCHDOWN, 500)
    
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
gnatsClient.disConnect()
shutdownJVM()
