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

package com.osi.gnats.client.equipment;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.aircraft.AircraftUtil;
import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.equipment.AircraftInterface;

import com.osi.gnats.client.BaseClass;
import com.osi.gnats.rmi.equipment.RemoteAircraft;

public class ClientAircraft extends BaseClass implements AircraftInterface {
	private RemoteAircraft remoteAircraft;
	
	public ClientAircraft(NATSInterface natsInterface, Remote rs) throws RemoteException {
		super(natsInterface, "Aircraft", "Aircraft Functions", "Aircraft Functions");
		this.remoteAircraft = (RemoteAircraft)rs;
	}
	
	public ClientAircraft(NATSInterface natsInterface, RemoteAircraft rs) throws RemoteException {
		super(natsInterface, "Aircraft", "Aircraft Functions", "Aircraft Functions");
		this.remoteAircraft = (RemoteAircraft)rs;
	}
	
	public int load_aircraft(String trx_file, String mfl_file) {
		int retValue = -1;
		
		try {
			retValue = remoteAircraft.load_aircraft(trx_file, mfl_file);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public boolean validate_flight_plan_record(String string_track, String string_fp_route, int mfl_ft) throws RemoteException {
		return remoteAircraft.validate_flight_plan_record(string_track, string_fp_route, mfl_ft);
	}
	
	public int release_aircraft() {
		int retValue = -1;
		
		try {
			retValue = remoteAircraft.release_aircraft();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public String[] getAllAircraftId() {
		String[] retArray = null;
		
		try {
			retArray = remoteAircraft.getAllAircraftId();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retArray;
	}

	public String[] getAircraftIds(float minLatitude, float maxLatitude, float minLongitude, float maxLongitude, float minAltitude_ft, float maxAltitude_ft) {
		String[] retArray = null;
		
		try {
			retArray = remoteAircraft.getAircraftIds(minLatitude, maxLatitude, minLongitude, maxLongitude, minAltitude_ft, maxAltitude_ft);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retArray;
	}
	
	public String[] getAssignedAircraftIds() throws RemoteException {
		return remoteAircraft.getAssignedAircraftIds(auth_id);
	}
	
	public String[] getAssignedAircraftIds(String username) throws RemoteException {
		return remoteAircraft.getAssignedAircraftIds(username);
	}
	
	public Aircraft select_aircraft(String aircraft_id) {
		Aircraft retAircraft = null;
		
		try {
			retAircraft = remoteAircraft.select_aircraft(sessionId, aircraft_id);
			AircraftUtil.injectRemoteInstance(retAircraft, remoteAircraft);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retAircraft;
	}
	
	public int synchronize_aircraft_to_server(Aircraft aircraft) {
		int retValue = -1;
		
		if (aircraft != null) {
			try {
				retValue = remoteAircraft.synchronize_aircraft_to_server(aircraft);
			} catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		
		return retValue;
	}
}
