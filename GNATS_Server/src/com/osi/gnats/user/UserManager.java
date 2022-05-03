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

package com.osi.gnats.user;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.FileReader;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.rmi.RemoteException;
import java.util.Map;
import java.util.Set;

import com.osi.gnats.rmi.ServerSession;
import com.osi.gnats.server.ServerNATS;
import com.osi.util.Constants;

import java.util.Date;
import java.util.HashMap;
import java.util.Iterator;

/**
 * A management module to handle all user-related functionality on NATS Server.
 * 
 * All client connections into NATS Server are stored in a server session pool.  This pool stores session information so that the server can identify the source of clients and check user permission.  A connection session data is created when a NATSClient instance is initialized.  The same session ID will be used between NATS Server and Client.
 * A connection session will exist since NATSClient is initialized and will be terminated when NATSClient.disconnect() is called.
 * 
 * Definition of user is stored in NATS_Server/share/user.conf
 * Three types of users are defined:
 * [Administrator]
 * admin
 * [Simulation_Admin]
 * sim1
 * [Normal_User]
 * user1:{AC_ID_1, AC_ID_2}, {GroundVehicle_ID_1, GroundVehicle_ID_2}
 * user2
 * 
 * The aircraft list followed by the user authentication ID is optional.  If you want to pre-assign aircraft to certain users, you can define those aircraft IDs after the ":" sign.
 * 
 * A user login/logout mechanism is used to grant different permissions to the session.
 * 
 * Permission rule:
 * - Before user login, NATS Server grant default permission USER_PERMISSION_TYPE_UNLOGINED_USER to the session.
 * - If NATS Server and Client is installed on the same machine and the user hasn't logged in, NATS Server grant USER_PERMISSION_TYPE_ADMINISTRATOR to the session.
 * - If the client calls login() function, NATS Server grants the corresponding permission of the user.
 */
public class UserManager {
	private static Map map_user_conf = null;
	private static Map<String, String[]> map_user_assignedAircrafts = null;
	private static Map<String, String[]> map_user_assignedGroundVehicles = null;
	
	private static HashMap<Integer, ServerSession> serverSessionPool = new HashMap<Integer, ServerSession>();
	
	public static final String DEFAULT_LOCALHOST_ADMIN_USERNAME = "localhost_admin";

	private static Thread thread_ServerSessionMaintainer = null;
	
	static {
		thread_ServerSessionMaintainer = new Thread(new UserManager().new Runnable_ServerSessionMaintainer());
		thread_ServerSessionMaintainer.start();
	}
	
	/**
	 * Get the map data which stores the users and their corresponding assigned aircraft
	 * @return
	 */
	public static Map<String, String[]> getMap_user_assignedAircrafts() {
		return map_user_assignedAircrafts;
	}
	
	/**
	 * Get the map data which stores the users and their corresponding assigned ground vehicles
	 * @return
	 */
	public static Map<String, String[]> getMap_user_assignedGroundVehicles() {
		return map_user_assignedGroundVehicles;
	}
	
	/**
	 * Get the authentication ID from the connection session
	 * @param sessionId
	 * @return
	 * @throws RemoteException
	 */
	public static String getAuthidFromSession(Integer sessionId) throws RemoteException {
		String retString = null;
		
		if (ServerNATS.isStandalone_mode()) {
			retString = DEFAULT_LOCALHOST_ADMIN_USERNAME;
			
			return retString;
		}
		
		ServerSession tmpSession = serverSessionPool.get(sessionId);
		
		if (tmpSession != null) {
			String tmpAuthid = tmpSession.getAuth_id();
			if ((tmpAuthid != null) && (!"".equals(tmpAuthid))) {
				retString = tmpAuthid;
			} else {
				throw new RemoteException("Please login.");
			}
		} else {
			throw new RemoteException("Please login.");
		}
		
		return retString;
	}
	
	/**
	 * Add a ServerSession into the pool
	 * @param sessionId
	 * @param session
	 */
	public static void addServerSession(Integer sessionId, ServerSession session) {
		serverSessionPool.put(sessionId, session);
	}
	
	/**
	 * Get a ServerSession
	 * @param sessionId
	 * @return
	 */
	public static ServerSession getServerSession(Integer sessionId) {
		return serverSessionPool.get(sessionId);
	}
	
	/**
	 * Update a ServerSession
	 * @param sessionId
	 * @param session
	 */
	public static void updateServerSession(Integer sessionId, ServerSession session) {
		serverSessionPool.put(sessionId, session);
	}
	
	/**
	 * Delete a ServerSession
	 * @param sessionId
	 */
	public static void deleteServerSession(Integer sessionId) {
		serverSessionPool.remove(sessionId);
	}
	
	/**
	 * Load user configuration file
	 * 
	 * NATS user config file is, by default, located in NATS_Server/share/user.conf
	 * @throws Exception
	 */
	public static void load_user_config() throws Exception {
		map_user_assignedAircrafts = new HashMap<String, String[]>();
		map_user_assignedGroundVehicles = new HashMap<String, String[]>();
		
		FileReader fr = new FileReader("share/user.conf");    
        BufferedReader br = new BufferedReader(fr);    

        String tmpAuthId = null;
        String[] tmpAssignedEquipmentStr = null;
        String tmpAssignedAircraftStr = null;
        String tmpAssignedGroundVehiclesStr = null;
        int tmpPermissionType = -1;
        User newUser = null;
        
        String line;
        
        map_user_conf = new HashMap();
        
		newUser = new User();
		newUser.setAuth_id(DEFAULT_LOCALHOST_ADMIN_USERNAME);
		newUser.setPermission_type(UserPermission.USER_PERMISSION_TYPE_ADMINISTRATOR);
		
		map_user_conf.put(DEFAULT_LOCALHOST_ADMIN_USERNAME, newUser);
		
        while ((line=br.readLine()) != null) {
        	tmpAuthId = ""; // Reset
        	
        	line = line.trim();

        	if (!line.startsWith("#")) {
				if (line.indexOf("[Administrator]") > -1) {
					tmpPermissionType = UserPermission.USER_PERMISSION_TYPE_ADMINISTRATOR;
				} else if (line.indexOf("[Simulation_Admin]") > -1) {
					tmpPermissionType = UserPermission.USER_PERMISSION_TYPE_SIMULATION_ADMIN;
				} else if (line.indexOf("[Normal_User]") > -1) {
					tmpPermissionType = UserPermission.USER_PERMISSION_TYPE_NORMAL_USER;
				}

				if ((line.indexOf("[") < 0) && (line.indexOf("]") < 0)) {
					if ((line.indexOf(":") > -1) && (line.length() > line.indexOf(":"))) {
						line = line.replaceAll(" ", "");
						tmpAuthId = line.substring(0, line.indexOf(":")).trim();
						tmpAssignedEquipmentStr = line.split("\\},\\{");
						tmpAssignedAircraftStr = tmpAssignedEquipmentStr[0].replace("{", "");
						if (-1 < tmpAssignedAircraftStr.indexOf(":")) {
							tmpAssignedAircraftStr = tmpAssignedAircraftStr.substring(tmpAssignedAircraftStr.indexOf(":")+1);
						}
						tmpAssignedGroundVehiclesStr = tmpAssignedEquipmentStr[1].replace("}", "");
					} else {
						tmpAuthId = line;
						tmpAssignedAircraftStr = null;
						tmpAssignedGroundVehiclesStr = null;
					}

					if (!tmpAuthId.equals("")) {
						newUser = new User();
						newUser.setAuth_id(tmpAuthId);
						newUser.setPermission_type(tmpPermissionType);
						
						map_user_conf.put(tmpAuthId, newUser);

						if (tmpAssignedAircraftStr != null) {
							String[] tmpAircraftArray = tmpAssignedAircraftStr.split(",");
							if ((tmpAssignedAircraftStr != null) && (tmpAircraftArray != null)) {
								map_user_assignedAircrafts.put(tmpAuthId, tmpAircraftArray);
							}
						}
						
						if (tmpAssignedGroundVehiclesStr != null) {
							String[] tmpGroundVehiclesArray = tmpAssignedGroundVehiclesStr.split(",");
							if ((tmpAssignedGroundVehiclesStr != null) && (tmpGroundVehiclesArray != null)) {
								map_user_assignedGroundVehicles.put(tmpAuthId, tmpGroundVehiclesArray);
							}
						}
					}
				}
        	}
        }

        br.close();    
        fr.close();
	}

	/**
	 * Get user configuration data by authentication ID
	 * @param auth_id
	 * @return
	 */
	public static User getUserconfByAuthid(String auth_id) {
		User retObject = null;
		
		if ((auth_id != null) && (map_user_conf.containsKey(auth_id))) {
			retObject = (User)map_user_conf.get(auth_id);
		}
		
		return retObject;
	}
	
	/**
	 * Get user permission type by session ID
	 * @param sessionId
	 * @return
	 */
	public static int getUserPermission(Integer sessionId) {
		int retValue = -1;
		
		ServerSession session = getServerSession(sessionId);
		if (session != null) {
			retValue = session.getPermission_type(); 
		}
		
		return retValue;
	}
	
	/**
	 * Get user permission type by authentication ID
	 * @param auth_id
	 * @return
	 */
	public static int getUserPermission(String auth_id) {
		int retValue = UserPermission.USER_PERMISSION_TYPE_UNLOGINED_USER;
		
		User tmpUser = getUserconfByAuthid(auth_id);
		if (tmpUser != null) {
			retValue = tmpUser.getPermission_type();
		}
		
		return retValue;
	}

	/* Routine maintainer on server sessions
	 * 
	 * This class is intended to run as a Thread.
	 * The reason to have this class is to check if the socket is still alive.
	 * 
	 * The logic is to check every element of serverSessionPool, send detect message into the socket, remove the server session element if the socket is invalid.
	 */
	class Runnable_ServerSessionMaintainer implements Runnable {
		
		public Runnable_ServerSessionMaintainer() {}
		
		public void run() {
			Set<Integer> set_keys = null;
			Iterator<Integer> iterator = null;
			Integer curKey = null;
			ServerSession curSession = null;
			Socket tmpSocket = null;
			
			while (true) {
				// Prepare the detect message
				String jsonStr = "{msg_detect_client_alive: \"" + new Date().getTime() + "\"}";

				set_keys = serverSessionPool.keySet();
				if (set_keys != null) {
					iterator = set_keys.iterator();
					
					while (iterator.hasNext()) {
						curKey = iterator.next();
						if (curKey != null) {
							curSession = serverSessionPool.get(curKey);
							if (curSession != null)  {
								if ((curSession.getSocket() == null) || (curSession.getSocket().isClosed())) {
									iterator.remove();
								} else {
									tmpSocket = curSession.getSocket();
									if ((tmpSocket != null) && (!tmpSocket.isClosed())) {
										try {
											ServerNATS.socket_outputString(tmpSocket, jsonStr);
										} catch (Exception ex) {
											iterator.remove();
										}
									}
								}
							}
						}
					}
				}
				
				try {
					Thread.sleep(2000);
				} catch (Exception ex) {}
			}
		}
	}
	
}
