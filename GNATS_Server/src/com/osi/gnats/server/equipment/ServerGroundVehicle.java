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

package com.osi.gnats.server.equipment;

import java.rmi.RemoteException;

import com.osi.gnats.rmi.equipment.RemoteGroundVehicle;
import com.osi.gnats.groundvehicle.GroundVehicle;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.user.UserManager;
import java.util.Map;
import java.util.Set;
import java.util.Iterator;
import java.io.File;

public class ServerGroundVehicle extends ServerClass implements RemoteGroundVehicle {
	public ServerGroundVehicle(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	public int load_groundVehicle(String trx_file) throws RemoteException {
		int retValue = 0;
		
		
		if(ServerNATS.cifpExists) {		
			
			retValue = cEngine.load_groundVehicle(trx_file);
			
			if (!ServerNATS.isStandalone_mode()) {
				if (retValue == 0) {
					Map<String, String[]> map_user_assignedGroundVehicles = UserManager.getMap_user_assignedGroundVehicles();
					Set<String> setKey = map_user_assignedGroundVehicles.keySet();
					Iterator<String> iterator_key = setKey.iterator();
					while (iterator_key.hasNext()) {
						String tmpAuthId = iterator_key.next();
						String[] tmpArrayGroundVehicles = map_user_assignedGroundVehicles.get(tmpAuthId);
						
						if ((tmpAuthId != null) && (!"".equals(tmpAuthId)) && (tmpArrayGroundVehicles != null) && (tmpArrayGroundVehicles.length > 0)) {
							for (int i = 0; i < tmpArrayGroundVehicles.length; i++) {
								cEngine.request_groundVehicle(tmpAuthId, tmpArrayGroundVehicles[i].trim());
							}
						}
					}
				}
			}
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
			retValue = 1;
		}
		
		return retValue;
	}
	
	public int release_groundVehicle() throws RemoteException {
		int retValue = 0;
		
		
		if(ServerNATS.cifpExists) {		
			retValue = cEngine.release_groundVehicle();
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
			retValue = 1;
		}
		
		return retValue;
	}

	public String[] getAllGroundVehicleIds() throws RemoteException {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getAllGroundVehicleIds();
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAssignedGroundVehicleIds(String username) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {
			if (!ServerNATS.isStandalone_mode()) {
				username = UserManager.DEFAULT_LOCALHOST_ADMIN_USERNAME;
				retValue = cEngine.getAssignedGroundVehicleIds(username);
			}
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		return retValue;
	}
	
	public GroundVehicle select_groundVehicle(int sessionId, String groundVehicleId) throws RemoteException {
		GroundVehicle retValue = null;
		
		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.select_groundVehicle(sessionId, groundVehicleId);
		}
		
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		return retValue;
	}
	
	public int synchronize_groundvehicle_to_server(GroundVehicle groundVehicle, String parameter) throws RemoteException {
		int retValue = 0;
		
		
		if(ServerNATS.cifpExists) {		
			retValue = cEngine.synchronize_groundvehicle_to_server(groundVehicle, parameter);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
			retValue = 1;
		}
		
		return retValue;
	}

	public int externalGroundVehicle_create_trajectory_profile(String groundVehicleId, String aircraftInService, String airport, float latitude, float longitude, float speed, float course) throws RemoteException {
		int retValue = 0;

		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.externalGroundVehicle_create_trajectory_profile(groundVehicleId, aircraftInService, airport, latitude, longitude, speed, course);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
			retValue = 1;
		}
		
		return retValue;
	}

	public int externalGroundVehicle_inject_trajectory_state_data(String groundVehicleId, String aircraftInService, float latitude, float longitude, float speed, float course) throws RemoteException {
		int retValue = 0;

		
		if(ServerNATS.cifpExists) {
			retValue = cEngine.externalGroundVehicle_inject_trajectory_state_data(groundVehicleId, aircraftInService, latitude, longitude, speed, course);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
			retValue = 1;
		}
		
		return retValue;
	}
}
