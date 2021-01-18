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

package com.osi.gnats.client;

import com.osi.gnats.api.*;
import com.osi.gnats.rmi.*;
import java.rmi.RemoteException;

import java.rmi.Remote;
import java.io.*;

public class ClientRiskMeasures extends BaseClass implements RiskMeasuresInterface {
	RemoteRiskMeasures remoteRiskMeasures;

	public ClientRiskMeasures(NATSInterface natsInterface, RemoteRiskMeasures re) throws RemoteException {
		super(natsInterface, "RiskMeasures", "RiskMeasures Interface", "RiskMeasures Functions");
		this.remoteRiskMeasures = re;
	}

	public ClientRiskMeasures(NATSInterface natsInterface, Remote re) throws RemoteException {
		super(natsInterface, "RiskMeasures", "RiskMeasures Interface", "RiskMeasures Functions");
		this.remoteRiskMeasures = (RemoteRiskMeasures) re;
	}

	public Object getFlightsInRange(String aircraftID) {
		Object retObj = new Object();

		try {
			retObj = remoteRiskMeasures.getFlightsInRange(sessionId, aircraftID);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObj;
	}

	public double getDistanceToRunwayThreshold(String aircraftId) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getDistanceToRunwayThreshold(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public double getDistanceToRunwayEnd(String aircraftId) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getDistanceToRunwayEnd(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}

	public double getVelocityAlignmentWithRunway(String aircraftId, String procedure) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getVelocityAlignmentWithRunway(sessionId, aircraftId, procedure);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public int getPassengerCount(String aircraftType) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.getPassengerCount(aircraftType);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public double getAircraftCost(String aircraftType) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getAircraftCost(aircraftType);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public Object getFlightsInWakeVortexRange(String refAircraftId, float envelopeStartLength, float envelopeStartBreadth, float envelopeEndLength, float envelopeEndBreadth, float envelopeRangeLength, float envelopeDecline) {
		Object retObj = new Object();

		try {
			retObj = remoteRiskMeasures.getFlightsInWakeVortexRange(sessionId, refAircraftId, envelopeStartLength, envelopeStartBreadth, envelopeEndLength, envelopeEndBreadth, envelopeRangeLength, envelopeDecline);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retObj;
	}
	
	public int setAircraftBookValue(String aircraftId, double aircraftBookValue) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.setAircraftBookValue(sessionId, aircraftId, aircraftBookValue);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public double getAircraftBookValue(String aircraftId) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getAircraftBookValue(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public int setCargoWorth(String aircraftId, double cargoWorth) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.setCargoWorth(sessionId, aircraftId, cargoWorth);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public double getCargoWorth(String aircraftId) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getCargoWorth(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public int setPassengerLoadFactor(String aircraftId, double paxLoadFactor) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.setPassengerLoadFactor(sessionId, aircraftId, paxLoadFactor);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public double getPassengerLoadFactor(String aircraftId) {
		double retValue = 0.0;

		try {
			retValue = remoteRiskMeasures.getPassengerLoadFactor(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	
    public int setTouchdownPointOnRunway(String aircraftId, double latitude, double longitude) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.setTouchdownPointOnRunway(sessionId, aircraftId, latitude, longitude);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
    
	public double[] getTouchdownPointOnRunway(String aircraftId) {
		double retValue[] = null;

		try {
			retValue = remoteRiskMeasures.getTouchdownPointOnRunway(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public int setTakeOffPointOnRunway(String aircraftId, double latitude, double longitude) {
		int retValue = 0;

		try {
			retValue = remoteRiskMeasures.setTakeOffPointOnRunway(sessionId, aircraftId, latitude, longitude);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
    
	public double[] getTakeOffPointOnRunway(String aircraftId) {
		double retValue[] = null;

		try {
			retValue = remoteRiskMeasures.getTakeOffPointOnRunway(sessionId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}

		return retValue;
	}
	
	public double getL1Distance(String airportId, String aircraftId1, String aircraftId2) {
		double retObject = -1.0;
		
		try {
			retObject = remoteRiskMeasures.getL1Distance(sessionId, airportId, aircraftId1, aircraftId2);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retObject;
	}
	
	public double getDistanceToPavementEdge(String airportId, String aircraftId) {
		double retObject = -1.0;
		
		try {
			retObject = remoteRiskMeasures.getDistanceToPavementEdge(sessionId, airportId, aircraftId);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retObject;
	}
	
	public int load_FlightPhaseSequence(String filename) throws IOException{
		int retObject = 0;
		
		try {
			retObject = remoteRiskMeasures.load_FlightPhaseSequence(filename);
		} catch (Exception ex) {
			ex.printStackTrace();
			retObject = 1;
		}
		
		return retObject;
	}
	
	public void clear_FlightPhaseSequence() {
		try {
			remoteRiskMeasures.clear_FlightPhaseSequence();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
}
