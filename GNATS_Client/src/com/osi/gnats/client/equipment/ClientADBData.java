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

import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.equipment.ADBDataInterface;
import com.osi.gnats.client.BaseClass;
import com.osi.gnats.rmi.equipment.RemoteADBData;

public class ClientADBData extends BaseClass implements ADBDataInterface {
	private RemoteADBData remoteADBData;
	
	public ClientADBData(NATSInterface natsInterface, Remote rs) throws RemoteException {
		super(natsInterface, "ADB Data", "ADB Data Functions", "ADB Data Functions");
		this.remoteADBData = (RemoteADBData)rs;
	}
	
	public ClientADBData(NATSInterface natsInterface, RemoteADBData rs) throws RemoteException {
		super(natsInterface, "ADB Data", "ADB Data Functions", "ADB Data Functions");
		this.remoteADBData = rs;
	}
	
	public double getADB_cruiseTas(String ac_type, double altitude_ft) throws RemoteException {
		return remoteADBData.getADB_cruiseTas(ac_type, altitude_ft);
	}
	
	public double getADB_climbRate_fpm(String ac_type, double flight_level, String adb_mass) throws RemoteException {
		return remoteADBData.getADB_climbRate(ac_type, flight_level, adb_mass);
	}

	public double getADB_climbTas(String ac_type, double altitude_ft) throws RemoteException {
		return remoteADBData.getADB_climbTas(ac_type, altitude_ft);
	}
	
	public double getADB_descentRate_fpm(String ac_type, double flight_level, String adb_mass) throws RemoteException {
		return remoteADBData.getADB_descentRate(ac_type, flight_level, adb_mass);
	}
	
	public double getADB_descentTas(String ac_type, double altitude_ft) throws RemoteException {
		return remoteADBData.getADB_descentTas(ac_type, altitude_ft);
	}
}
