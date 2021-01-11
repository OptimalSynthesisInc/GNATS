/*
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
*/

package com.osi.util;

import java.lang.reflect.Field;

public class Constants {
	public static final int DUMMY_SESSION_ID = 0;
	
	public static final int GNATS_SIMULATION_STATUS_READY = 0;
	public static final int GNATS_SIMULATION_STATUS_START = 1;
	public static final int GNATS_SIMULATION_STATUS_PAUSE = 2;
	public static final int GNATS_SIMULATION_STATUS_RESUME = 3;
	public static final int GNATS_SIMULATION_STATUS_STOP = 4;
	public static final int GNATS_SIMULATION_STATUS_ENDED = 5;
	
	public static final double FEET_TO_METERS = 0.3048;
	public static final double METERS_TO_FEET = 1 / FEET_TO_METERS;
	
	public static final double TWO_PI = 2 * Math.PI;
	public static final double HALF_PI = Math.PI / 2.0;
	
	// source- http://ssd.jpl.nasa.gov/phys_props_earth.html
	public static final double MASS_EARTH_KG = 5.9736e24;
	public static final double MEAN_RADIUS_EARTH_M = 6371010;
	public static final double GRAVITATIONAL_CONST = 6.67300e-11;
	
	public static final double MEAN_RADIUS_EARTH_FEET = MEAN_RADIUS_EARTH_M * METERS_TO_FEET;
	
	public static final double DEFAULT_GRAVITY = 9.80665;
	public static final double DEFAULT_GRAVITY_FEET = DEFAULT_GRAVITY * METERS_TO_FEET;
	
	// these found in sim.h
	public static final double KNOTS_TO_FEET_PER_MIN = 101.2686;
	public static final double KNOTS_TO_METERS_PER_SEC = 0.5144444;
	public static final double METERS_PER_SEC_TO_KNOTS = 1.943844;
	public static final double FEET_PER_MIN_TO_METERS_PER_SEC = 0.005080;
	public static final double FEET_TO_NAUTICAL_MILES = 0.00016457879152518;
	public static final double NAUTICAL_MILES_TO_METERS = 1852;		
	public static final double NAUTICAL_MILES_TO_FEET = NAUTICAL_MILES_TO_METERS * METERS_TO_FEET;
	
	public static final String DEFAULT_SERVER_RMI_IP_ADDRESS = "127.0.0.1";
	public static final String DEFAULT_SERVER_RMI_PORT_NUMBER = "2017";
	public static final String DEFAULT_STANDALONE_RMI_PORT_NUMBER = "2020";
	
	public static final int SOCKET_PORT_NUMBER = 2019;
	
	public static final String MSG_FUNC_INVALID_STANDALONE_MODE = "This function is invalid in NATS Standalone mode";
	
	public static int getNATS_Simulation_Status_Def(String str_nats_simulation_status) throws Exception {
		int retValue = -1;
		
		if ((str_nats_simulation_status != null) && (str_nats_simulation_status.startsWith("NATS_SIMULATION_STATUS"))) {
			Field tmpField = Class.forName("com.osi.util.Constants").getField(str_nats_simulation_status);
			if (tmpField != null) {
				retValue = tmpField.getInt(null);
			}
		}
		
		return retValue;
	}

}
