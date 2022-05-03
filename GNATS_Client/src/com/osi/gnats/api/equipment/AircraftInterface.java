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

package com.osi.gnats.api.equipment;

import java.rmi.RemoteException;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.api.BaseInterface;

public interface AircraftInterface extends BaseInterface {
	
	/**
	 * Load aircraft data.  Acceptable files are TRX and MFL files.
	 * @param trx_file TRX filename and path.  The filename must be ended with ".trx".
	 * @param mfl_file MFL filename and path.  The filename must be ended with ".trx".
	 * @return
	 */
	public int load_aircraft(String trx_file, String mfl_file);
	
	/**
	 * Get validation result of flight plan record
	 * 
	 * @param string_track String of the TRACK data
	 * @param string_fp_route String of the FP_ROUTE data
	 * @param mfl_ft Maximum flight level in feet
	 * @return Boolean value indicating whether the given flight plan is valid.
	 * @throws RemoteException
	 */
	public boolean validate_flight_plan_record(String string_track, String string_fp_route, int mfl_ft) throws RemoteException;
	
	/**
	 * Clean up all aircraft data.
	 * @return
	 */
	public int release_aircraft();
	
	/**
	 * Get complete aircraft IDs.
	 * @return
	 */
	public String[] getAllAircraftId();
	
	/**
	 * Get qualified aircraft ID which which satisfy the min/max range of latitude, longitude and/or altitude.
	 * @param minLatitude Minimum latitude degree
	 * @param maxLatitude Maximum latitude degree
	 * @param minLongitude Minimum longitude degree
	 * @param maxLongitude Maximum longitude degree
	 * @param minAltitude_ft Minimum altitude in feet.  This parameter is optional.
	 * @param maxAltitude_ft Maximum altitude in feet.  This parameter is optional.
	 * @return
	 */
	public String[] getAircraftIds(float minLatitude, float maxLatitude, float minLongitude, float maxLongitude, float minAltitude_ft, float maxAltitude_ft);
	
	/**
	 * Get IDs of aircrafts which are assigned to current session user
	 * 
	 * @return Array of aircraft IDs
	 * @throws RemoteException
	 */
	public String[] getAssignedAircraftIds() throws RemoteException;
	
	/**
	 * Get IDs of aircrafts which are assigned to the user
	 * 
	 * @param username
	 * @return Array of aircraft IDs
	 * @throws RemoteException
	 */
	public String[] getAssignedAircraftIds(String username) throws RemoteException;
	
	/**
	 * Get an aircraft object by aircraft ID.
	 * @param aircraft_id Aircraft ID
	 * @return
	 */
	public Aircraft select_aircraft(String aircraft_id);
	
	/**
	 * Push aircraft object to the server and synchronize the data.  Any update on an aircraft object must be synchronized to the server.  Otherwise, the updates are only valid on client side and does not reflect the changes on server.
	 * @param aircraft The aircraft object to be synchronized.
	 * @return Return value indicating the server operation response.  0 means success.  1 means error.
	 */
	public int synchronize_aircraft_to_server(Aircraft aircraft);
	
}
