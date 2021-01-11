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

package com.osi.gnats.api.environment;

import com.osi.gnats.api.BaseInterface;

public interface TerminalAreaInterface extends BaseInterface {
	/**
	 * Get current SID procedure of the given airport on the given aircraft flight.
	 * @param acid
	 * @return
	 */
	public String getCurrentSid(String acid);
	
	/**
	 * Get current STAR procedure of the given airport on the given aircraft flight.
	 * @param acid
	 * @return
	 */
	public String getCurrentStar(String acid);
	
	/**
	 * Get current Approach procedure of the given airport on the given aircraft flight.
	 * @param acid
	 * @return
	 */
	public String getCurrentApproach( String acid);
	
	/**
	 * Get all SID procedures of the given airport.
	 * @param airport_code
	 * @return
	 */
	public String[] getAllSids(String airport_code);
	
	/**
	 * Get all STAR procedures of the given airport.
	 * @param airport_code
	 * @return
	 */
	public String[] getAllStars(String airport_code);
	
	/**
	 * Get all Approach procedures of the given airport.
	 * @param airport_code
	 * @return
	 */
	public String[] getAllApproaches(String airport_code);
	
	/**
	 * Get leg names of the given airport code, procedure type and procedure name.
	 * @param proc_type Procedure type.  The valid values are only limited to "SID", "STAR" and "APPROACH".
	 * @param proc_name Name of procedure.
	 * @param airport_code Airport code.
	 * @return Array of leg names.
	 */
	public String[] getProcedure_leg_names(String proc_type, String proc_name, String airport_code);
	
	/**
	 * Get waypoints of the given airport code, procedure type, procedure name and leg name.
	 * @param proc_type Procedure type.  The valid values are only limited to "SID", "STAR" and "APPROACH".
	 * @param proc_name Name of procedure.
	 * @param airport_code Airport code.
	 * @param proc_leg_name Name of procedure leg.
	 * @return
	 */
	public String[] getWaypoints_in_procedure_leg(String proc_type, String proc_name, String airport_code, String proc_leg_name);

	/**
	 * Get latitude and longitude degree of a given waypoint.
	 * @param waypoint_name
	 * @return
	 */
	public double[] getWaypoint_Latitude_Longitude_deg(String waypoint_name);
	
	/**
	 * Calculate distance between two geological points
	 * @param latx
	 * @param lonx
	 * @param laty
	 * @param lony
	 * @return
	 */
	public double calculateWaypointDistance(float latx, float lonx, float laty, float lony);
	
	/**
	 * Get upper-bound altitude data of the procedure.
	 * @param proc_type
	 * @param proc_name
	 * @param airport_code
	 * @param proc_leg_name
	 * @param proc_wp_name
	 * @return
	 */
	public double getProcedure_alt_1(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name);
	
	/**
	 * Get lower-bound altitude data of the procedure.
	 * @param proc_type
	 * @param proc_name
	 * @param airport_code
	 * @param proc_leg_name
	 * @param proc_wp_name
	 * @return
	 */
	public double getProcedure_alt_2(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name);
	
	/**
	 * Get speed limit of the procedure.
	 * @param proc_type
	 * @param proc_name
	 * @param airport_code
	 * @param proc_leg_name
	 * @param proc_wp_name
	 * @return
	 */
	public double getProcedure_speed_limit(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name);
	
	/**
	 * Get altitude description of the procedure.
	 * @param proc_type
	 * @param proc_name
	 * @param airport_code
	 * @param proc_leg_name
	 * @param wp_name
	 * @return
	 */
	public String getProcedure_alt_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);
	
	/**
	 * Get speed limit description of the procedure.
	 * @param proc_type
	 * @param proc_name
	 * @param airport_code
	 * @param proc_leg_name
	 * @param wp_name
	 * @return
	 */
	public String getProcedure_speed_limit_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name);

}
