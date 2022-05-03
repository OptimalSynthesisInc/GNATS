/*
------------------------------------- Credits -----------------------------------------------------------------
Generalized National Airspace Trajectory Simulation (GNATS) software
2017-2021 GNATS Development Team at Optimal Synthesis Inc. are:
Team Lead, Software Architecture and Algorithms: Dr. P. K. Menon
Algorithms and Prototyping: Dr. Parikshit Dutta
Java and C++ Code Development: Oliver Chen and Hari N. Iyer
Illustrative Examples in Python and MATLAB: Dr. Parikshit Dutta, Dr. Bong-Jun Yang, Hari Iyer
Illustrative Examples in SciLab and R: Hari Iyer
Vedik Jayaraj (Summer Intern) helped digitize 39 US airports together with the Arrival-Departure procedures and helped in beta testing of GNATS.
Acknowledgements: 
GNATS software was developed under the Arizona State University Subaward No. 18-275 under the NASA University Leadership Initiative Prime Contract No. NNX17AJ86A, with Professor Yongming Liu serving as the Principal Investigator. 
Beta Testing outside Optimal Synthesis Inc. was carried out at Arizona State University under the direction of Professor Yongming Liu, at Vanderbilt University under the direction of Professor Sankaran Mahadevan and Professor Pranav Karve, at the Southwest Research Institute under the direction of Dr. Baron Bichon and Dr. Erin DeCarlo, and at Carnegie-Mellon University under the direction of Professor Pingbo Tang.
NASA Technical points-of-contact: Dr. Anupa Bajwa, Dr. Kaushik Datta, Dr. John Cavolowsky, Dr. Kai Goebel
------------------------------------Legacy Source Code--------------------------------------------------------
Legacy Code for the GNATS software was derived from the software packages developed under the following NASA Small Business Innovation Research Projects:
1. 2004-2006 NASA Contract No. NNA05BE64C with Dr. Shon Grabbe of NASA Ames Research Center as the Technical Monitor.
2. 2008-2010 NASA Contract No. NNX08CA02C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
3. 2010-2011 NASA Phase III Contract No. NNA10DC12C with Dr. Joseph Rios of Ames Research Center as the Technical Monitor.
4. 2016-2018 NASA Contract No. NNX16CL11C with Dr. Nash’at Ahmad of NASA Langley Research Center as the Technical Monitor.

Contributors to these SBIR projects at Optimal Synthesis Inc. were: Dr. P. K. Menon (Principal Investigator), Jason Kwan (Software Engineer), Gerald M. Diaz (Software Engineer), Dr. Monish Tandale (Research Scientist), Dr. Prasenjit Sengupta (Research Scientist), Dr. Sang-Gyun Park (Research Scientist) and Dr. Parikshit Dutta (Research Scientist).
The inspiration for the SBIR projects is derived from the FACET software developed at NASA Ames Research Center by Dr. Banavar Sridhar, Dr. Karl Bilimoria, Dr. Gano Chatterji, Dr. Shon Grabbe and Dr. Kapil Sheth.

Dr. Victor H. L. Cheng of Optimal Synthesis Inc. provided the digitized data for 40 major US Airports
---------------------------------------------------------------------------------------------------------------------
*/

package com.osi.util;

/**
 * Definition of flight phase
 */
public class FlightPhaseReference {
	public static final FlightPhase FLIGHT_PHASE_PREDEPARTURE = FlightPhase.FLIGHT_PHASE_PREDEPARTURE;
	public static final FlightPhase FLIGHT_PHASE_ORIGIN_GATE = FlightPhase.FLIGHT_PHASE_ORIGIN_GATE;
	public static final FlightPhase FLIGHT_PHASE_PUSHBACK = FlightPhase.FLIGHT_PHASE_PUSHBACK;
	public static final FlightPhase FLIGHT_PHASE_RAMP_DEPARTING = FlightPhase.FLIGHT_PHASE_RAMP_DEPARTING;
	public static final FlightPhase FLIGHT_PHASE_TAXI_DEPARTING = FlightPhase.FLIGHT_PHASE_TAXI_DEPARTING;
	public static final FlightPhase FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING = FlightPhase.FLIGHT_PHASE_RUNWAY_THRESHOLD_DEPARTING;
	public static final FlightPhase FLIGHT_PHASE_TAKEOFF = FlightPhase.FLIGHT_PHASE_TAKEOFF;
	public static final FlightPhase FLIGHT_PHASE_CLIMBOUT = FlightPhase.FLIGHT_PHASE_CLIMBOUT;
	public static final FlightPhase FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN = FlightPhase.FLIGHT_PHASE_HOLD_IN_DEPARTURE_PATTERN;
	public static final FlightPhase FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE = FlightPhase.FLIGHT_PHASE_CLIMB_TO_CRUISE_ALTITUDE;
	public static final FlightPhase FLIGHT_PHASE_TOP_OF_CLIMB = FlightPhase.FLIGHT_PHASE_TOP_OF_CLIMB;
	public static final FlightPhase FLIGHT_PHASE_CRUISE = FlightPhase.FLIGHT_PHASE_CRUISE;
	public static final FlightPhase FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN = FlightPhase.FLIGHT_PHASE_HOLD_IN_ENROUTE_PATTERN;
	public static final FlightPhase FLIGHT_PHASE_TOP_OF_DESCENT = FlightPhase.FLIGHT_PHASE_TOP_OF_DESCENT;
	public static final FlightPhase FLIGHT_PHASE_INITIAL_DESCENT = FlightPhase.FLIGHT_PHASE_INITIAL_DESCENT;
	public static final FlightPhase FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN = FlightPhase.FLIGHT_PHASE_HOLD_IN_ARRIVAL_PATTERN;
	public static final FlightPhase FLIGHT_PHASE_APPROACH = FlightPhase.FLIGHT_PHASE_APPROACH;
	public static final FlightPhase FLIGHT_PHASE_FINAL_APPROACH = FlightPhase.FLIGHT_PHASE_FINAL_APPROACH;
	public static final FlightPhase FLIGHT_PHASE_GO_AROUND = FlightPhase.FLIGHT_PHASE_GO_AROUND;
	public static final FlightPhase FLIGHT_PHASE_TOUCHDOWN = FlightPhase.FLIGHT_PHASE_TOUCHDOWN;
	public static final FlightPhase FLIGHT_PHASE_LAND = FlightPhase.FLIGHT_PHASE_LAND;
	public static final FlightPhase FLIGHT_PHASE_EXIT_RUNWAY = FlightPhase.FLIGHT_PHASE_EXIT_RUNWAY;
	public static final FlightPhase FLIGHT_PHASE_TAXI_ARRIVING = FlightPhase.FLIGHT_PHASE_TAXI_ARRIVING;
	public static final FlightPhase FLIGHT_PHASE_RUNWAY_CROSSING = FlightPhase.FLIGHT_PHASE_RUNWAY_CROSSING;
	public static final FlightPhase FLIGHT_PHASE_RAMP_ARRIVING = FlightPhase.FLIGHT_PHASE_RAMP_ARRIVING;
	public static final FlightPhase FLIGHT_PHASE_DESTINATION_GATE = FlightPhase.FLIGHT_PHASE_DESTINATION_GATE;
	public static final FlightPhase FLIGHT_PHASE_LANDED = FlightPhase.FLIGHT_PHASE_LANDED;
	public static final FlightPhase FLIGHT_PHASE_HOLDING = FlightPhase.FLIGHT_PHASE_HOLDING;
}
