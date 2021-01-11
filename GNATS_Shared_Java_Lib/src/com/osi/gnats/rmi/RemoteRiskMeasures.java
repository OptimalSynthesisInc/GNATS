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

package com.osi.gnats.rmi;

import java.rmi.Remote;
import java.rmi.RemoteException;

public interface RemoteRiskMeasures extends BaseRemote {
	public Object getFlightsInRange(int sessionId, String aircraftID) throws RemoteException;
	public double getDistanceToRunwayThreshold(int sessionId, String aircraftId) throws RemoteException;
	public double getDistanceToRunwayEnd(int sessionId, String aircraftId) throws RemoteException;
	public double getVelocityAlignmentWithRunway(int sessionId, String aircraftId, String procedure) throws RemoteException;
	public Integer getPassengerCount(String aircraftType) throws RemoteException;
	public double getAircraftCost(String aircraftType) throws RemoteException;
	public Object getFlightsInWakeVortexRange(int sessionId, String refAircraftId, float envelopeStartLength, float envelopeStartBreadth, float envelopeEndLength, float envelopeEndBreadth, float envelopeRangeLength, float envelopeDecline) throws RemoteException;
	public int setAircraftBookValue(int sessionId, String aircraftId, double aircraftBookValue) throws RemoteException;
	public double getAircraftBookValue(int sessionId, String aircraftId) throws RemoteException;
	public int setCargoWorth(int sessionId, String aircraftId, double cargoWorth) throws RemoteException;
	public double getCargoWorth(int sessionId, String aircraftId) throws RemoteException;
	public int setPassengerLoadFactor(int sessionId, String aircraftId, double paxLoadFactor) throws RemoteException;
	public double getPassengerLoadFactor(int sessionId, String aircraftId) throws RemoteException;
	public int setTouchdownPointOnRunway(int sessionId, String aircraftId, double latitude, double longitude) throws RemoteException;
	public double[] getTouchdownPointOnRunway(int sessionId, String aircraftId) throws RemoteException;
	public int setTakeOffPointOnRunway(int sessionId, String aircraftId, double latitude, double longitude) throws RemoteException;
	public double[] getTakeOffPointOnRunway(int sessionId, String aircraftId) throws RemoteException;
	public double getL1Distance(int sessionId, String airportId, String aircraftId1, String aircraftId2) throws RemoteException;
	public double getDistanceToPavementEdge(int sessionId, String airportId, String aircraftId) throws RemoteException;
	public int load_FlightPhaseSequence(String filename) throws RemoteException;
	public void clear_FlightPhaseSequence() throws RemoteException;
	
	public int load_aviationOccurenceProfile(String dirPath) throws RemoteException;
}
