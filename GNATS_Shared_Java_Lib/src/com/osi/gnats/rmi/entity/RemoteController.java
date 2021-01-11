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

package com.osi.gnats.rmi.entity;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.util.AircraftClearance;

public interface RemoteController extends Remote {

	public int setDelayPeriod(String acid, AircraftClearance aircraft_clearance, float seconds) throws RemoteException;
	
	public int setActionRepeat(int sessionId, String controllerAircraftID, String controllerRepeatParameter) throws RemoteException;
	
	public int skipFlightPhase(int sessionId, String controllerAircraftID, String controllerFlightPhase) throws RemoteException;
	
	public int setWrongAction(int sessionId, String controllerAircraftID, String controllerOriginalChangeParameter, String controllerWrongChangeParameter) throws RemoteException;
	
	public int setActionReversal(int sessionId, String controllerAircraftID, String controllerChangeParameter) throws RemoteException;
	
	public int setPartialAction(int sessionId, String controllerAircraftID, String controllerChangeParameter, float controllerOriginalTarget, float controllerPercentage) throws RemoteException;
	
	public int skipChangeAction(int sessionId, String controllerAircraftID, String controllerSkipParameter) throws RemoteException;
	
	public int setActionLag(int sessionId, String controllerAircraftID, String controllerLagParameter, float controllerLagTimeConstant, float controllerPercentageError, float controllerParameterTarget) throws RemoteException;

	public int setControllerAbsence(int sessionId, String controllerAircraftID, int timeSteps) throws RemoteException;
	
	public int releaseAircraftHold(int sessionId, String controllerAircraftID, String approach, String targetWaypoint) throws RemoteException, Exception;
	
	public void enableConflictDetectionAndResolution(boolean flag) throws RemoteException;
	
	public Object[][] getCDR_status() throws RemoteException;
	
	public void setCDR_initiation_distance_ft_surface(float distance) throws RemoteException;
	
	public void setCDR_initiation_distance_ft_terminal(float distance) throws RemoteException;
	
	public void setCDR_initiation_distance_ft_enroute(float distance) throws RemoteException;
	
	public void setCDR_separation_distance_ft_surface(float distance) throws RemoteException;
	
	public void setCDR_separation_distance_ft_terminal(float distance) throws RemoteException;
	
	public void setCDR_separation_distance_ft_enroute(float distance) throws RemoteException;

	public void enableMergingAndSpacingAtMeterFix(String centerId, String meterFix, String spacingType, float spacing) throws RemoteException;

	public void disableMergingAndSpacingAtMeterFix(String centerId, String meterFix) throws RemoteException;

	public void enableStrategicWeatherAvoidance(boolean flag) throws RemoteException;
	
	public void setWeather_pirepFile(String pathFilename) throws RemoteException;
	
	public void setWeather_polygonFile(String pathFilename) throws RemoteException;
	
	public void setWeather_sigmetFile(String pathFilename) throws RemoteException;
	
	public int setTacticalWeatherAvoidance(String waypoint_name, float duration) throws RemoteException;
	
	public int insertAirborneWaypoint(String acid,
			int index_to_insert,
			String waypoint_type,
			String waypoint_name,
			float waypoint_latitude,
			float waypoint_longitude,
			float waypoint_altitude,
			float waypoint_speed_lim,
			String spdlim_desc) throws RemoteException;
	
	public int deleteAirborneWaypoint(String acid,
			int index_to_delete) throws RemoteException;
	
	public void setTargetWaypoint(String acid,
			int waypoint_plan_to_use,
			int index_of_target) throws RemoteException;
	 
}
