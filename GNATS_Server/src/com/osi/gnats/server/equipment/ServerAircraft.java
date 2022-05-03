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

package com.osi.gnats.server.equipment;

import java.rmi.RemoteException;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import com.osi.gnats.aircraft.Aircraft;

import com.osi.gnats.rmi.equipment.RemoteAircraft;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.user.UserManager;
import com.osi.gnats.user.UserPermission;

/**
 * Actual RMI class which implements RemoteAircraft
 */
public class ServerAircraft extends ServerClass implements RemoteAircraft {
	
	public ServerAircraft(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	/**
	 * Check if the current user is the aircraft owner
	 * @param username User name
	 * @param acid Aircraft ID
	 * @return
	 * @throws RemoteException
	 */
	public static boolean isAircraftAssignee(String username, String acid) throws RemoteException {
		boolean retValue = false;
		
		String acAssignee = cEngine.getAircraftAssignee(acid);
		if ((username != null) && (!"".equals(username))) {
			if ((username.equals(acAssignee))
					|| (UserManager.getUserPermission(username) >= UserPermission.USER_PERMISSION_TYPE_SIMULATION_ADMIN)
					) {
				retValue = true;
			}
		} else {
			throw new RemoteException("The requested aircraft is not assigned to you.");
		}
		
		return retValue;
	}
	
	/**
	 * Load aircraft
	 */
	public int load_aircraft(String trx_file, String mfl_file) {
		int retValue = -1;
		
		retValue = cEngine.load_aircraft(trx_file, mfl_file);
		
		if (!ServerNATS.isStandalone_mode()) {
			if (retValue == 0) {
				// Check user assigned aircraft definition
				// Assign aircraft to the owners
				Map<String, String[]> map_user_assignedAircrafts = UserManager.getMap_user_assignedAircrafts();
				Set<String> setKey = map_user_assignedAircrafts.keySet();
				Iterator<String> iterator_key = setKey.iterator();
				while (iterator_key.hasNext()) {
					String tmpAuthId = iterator_key.next();
					String[] tmpArrayAircrafts = map_user_assignedAircrafts.get(tmpAuthId);
					
					if ((tmpAuthId != null) && (!"".equals(tmpAuthId)) && (tmpArrayAircrafts != null) && (tmpArrayAircrafts.length > 0)) {
						for (int i = 0; i < tmpArrayAircrafts.length; i++) {
							cEngine.request_aircraft(tmpAuthId, tmpArrayAircrafts[i].trim());
						}
					}
				}
			}
		}
		
		return retValue;
	}
	
	/**
	 * Load aircraft formatted in geo-location style
	 */
/*
	public int load_aircraft_geoStyle(String trx_file, String mfl_file) {
		int retValue = -1;
		
		retValue = cEngine.load_aircraft_geoStyle(trx_file, mfl_file);
		
		if (!ServerNATS.isStandalone_mode()) {
			if (retValue == 0) {
				// Check user assigned aircraft definition
				// Assign aircraft to the owners
				Map<String, String[]> map_user_assignedAircrafts = UserManager.getMap_user_assignedAircrafts();
				Set<String> setKey = map_user_assignedAircrafts.keySet();
				Iterator<String> iterator_key = setKey.iterator();
				while (iterator_key.hasNext()) {
					String tmpAuthId = iterator_key.next();
					String[] tmpArrayAircrafts = map_user_assignedAircrafts.get(tmpAuthId);
					
					if ((tmpAuthId != null) && (!"".equals(tmpAuthId)) && (tmpArrayAircrafts != null) && (tmpArrayAircrafts.length > 0)) {
						for (int i = 0; i < tmpArrayAircrafts.length; i++) {
							cEngine.request_aircraft(tmpAuthId, tmpArrayAircrafts[i].trim());
						}
					}
				}
			}
		}
		
		return retValue;
	}
*/
	public boolean validate_flight_plan_record(String string_track, String string_fp_route, int mfl_ft) throws RemoteException {
		return cEngine.validate_flight_plan_record(string_track, string_fp_route, mfl_ft);
	}
	
	/**
	 * Release aircraft
	 */
	public int release_aircraft() {
		int retValue = -1;
		
		retValue = cEngine.release_aircraft();
		
		return retValue;
	}
	
	/**
	 * Get all aircraft IDs
	 */
	public String[] getAllAircraftId() {
		return cEngine.getAllAircraftId();
	}
	
	/**
	 * Get aircraft IDs within the given zone
	 */
	public String[] getAircraftIds(float minLatitude, float maxLatitude, float minLongitude, float maxLongitude, float minAltitude_ft, float maxAltitude_ft) {
		String[] retObject = null;
		
		retObject = cEngine.getAircraftIds(minLatitude, maxLatitude, minLongitude, maxLongitude, minAltitude_ft, maxAltitude_ft);
		
		return retObject;
	}
	
	/**
	 * Get assigned aircraft IDs of the given user
	 */
	public String[] getAssignedAircraftIds(String username) throws RemoteException {
		if (ServerNATS.isStandalone_mode()) {
			username = UserManager.DEFAULT_LOCALHOST_ADMIN_USERNAME;
		}
		
		return cEngine.getAssignedAircraftIds(username);
	}
	
	/**
	 * Get aircraft instance
	 */
	public Aircraft select_aircraft(int sessionId, String aircraft_id) {
		Aircraft retAircraft = null;

		if ((aircraft_id != null) && (!"".equals(aircraft_id))) {
			retAircraft = cEngine.select_aircraft(sessionId, aircraft_id);
		}
		
		return retAircraft;
	}
	
	/**
	 * Synchronize aircraft instance data to NATS Server
	 * 
	 * Aircraft instance data exists on the RMI client side.  During runtime, the RMI server can not tell what client value is modified.
	 * To have consistent data between RMI server and client, this function will push client data to the server and update it.
	 */
	public int synchronize_aircraft_to_server(Aircraft aircraft) throws RemoteException {
		int retValue = -1;

		if (aircraft != null) {
			if (cEngine.isRealTime_simulation()) {
				String tmpAuth_id = UserManager.getAuthidFromSession(aircraft.getSessionId());
				
				// Check aircraft ownership
				// If this session user does not own the aircraft, a RemoteException will be thrown
				ServerAircraft.isAircraftAssignee(tmpAuth_id, aircraft.getAcid());
			}
			
			retValue = cEngine.synchronize_aircraft_to_server(aircraft);
		}
		
		return retValue;
	}
	
	/**
	 * Postpone the departure of the aircraft for certain period of time
	 */
	public int delay_departure(String acid, int seconds) {
		int retValue = -1;

		if (seconds > 0) {
			retValue = cEngine.delay_departure(acid, seconds);
		}
		
		return retValue;
	}
}
