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

package com.osi.gnats.client.equipment;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.groundvehicle.GroundVehicle;
import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.groundvehicle.GroundVehicleUtil;
import com.osi.gnats.api.equipment.GroundVehicleInterface;
import com.osi.gnats.client.BaseClass;
import com.osi.gnats.rmi.equipment.RemoteGroundVehicle;;

public class ClientGroundVehicle extends BaseClass implements GroundVehicleInterface {
	private RemoteGroundVehicle remoteGroundVehicle;

	public ClientGroundVehicle(NATSInterface natsInterface, Remote rs) throws RemoteException {
		super(natsInterface, "Ground Vehicle", "Ground Vehicle Functions", "Ground Vehicle Functions");
		this.remoteGroundVehicle = (RemoteGroundVehicle) rs;
	}

	public ClientGroundVehicle(NATSInterface natsInterface, RemoteGroundVehicle rs) throws RemoteException {
		super(natsInterface, "Ground Vehicle", "Ground Vehicle Functions", "Ground Vehicle Functions");
		this.remoteGroundVehicle = (RemoteGroundVehicle) rs;
	}

	public int load_groundVehicle(String trx_file) throws RemoteException {
		int retValue = 0;
		try {
			retValue = remoteGroundVehicle.load_groundVehicle(trx_file);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retValue;
	}

	public int release_groundVehicle() throws RemoteException {
		int retValue = 0;
		try {
			retValue = remoteGroundVehicle.release_groundVehicle();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retValue;
	}

	public String[] getAllGroundVehicleIds() throws RemoteException {
		String retArray[] = null;
		try {
			retArray = remoteGroundVehicle.getAllGroundVehicleIds();
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retArray;
	}
	
	public String[] getAssignedGroundVehicleIds() {
		String retArray[] = null;
		try {
			retArray = remoteGroundVehicle.getAssignedGroundVehicleIds(auth_id);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retArray;
	}

	
	public String[] getAssignedGroundVehicleIds(String username) {
		String retArray[] = null;
		try {
			retArray = remoteGroundVehicle.getAssignedGroundVehicleIds(username);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retArray;
	}
	
	public GroundVehicle select_groundVehicle(String groundVehicleId) throws RemoteException {
		GroundVehicle retGroundVehicle = null;
		try {
			retGroundVehicle = remoteGroundVehicle.select_groundVehicle(sessionId, groundVehicleId);
			GroundVehicleUtil.injectRemoteInstance(retGroundVehicle, remoteGroundVehicle);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retGroundVehicle;
	}

	public int synchronize_groundvehicle_to_server(GroundVehicle groundVehicle, String parameter) throws RemoteException {
		int retValue = 0;
		try {
			retValue = remoteGroundVehicle.synchronize_groundvehicle_to_server(groundVehicle, parameter);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retValue;
	}

	public int externalGroundVehicle_create_trajectory_profile(String groundVehicleId, String aircraftInService, String airport, float latitude, float longitude, float speed, float course) throws RemoteException {
		int retValue = 0;
		try {
			retValue = remoteGroundVehicle.externalGroundVehicle_create_trajectory_profile(groundVehicleId, aircraftInService, airport, latitude, longitude, speed, course);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retValue;
	}

	public int externalGroundVehicle_inject_trajectory_state_data(String groundVehicleId, String aircraftInService, float latitude, float longitude, float speed, float course) throws RemoteException {
		int retValue = 0;
		try {
			retValue = remoteGroundVehicle.externalGroundVehicle_inject_trajectory_state_data(groundVehicleId, aircraftInService, latitude, longitude, speed, course);
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}
		return retValue;
	}
}
