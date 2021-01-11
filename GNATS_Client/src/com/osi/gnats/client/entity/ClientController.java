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

package com.osi.gnats.client.entity;

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.entity.ControllerInterface;
import com.osi.gnats.client.BaseClass;
import com.osi.gnats.rmi.entity.RemoteController;
import com.osi.util.AircraftClearance;

public class ClientController extends BaseClass implements ControllerInterface {
	private RemoteController remoteController;
	
	public ClientController(NATSInterface natsInterface, RemoteController re) throws RemoteException {
		super(natsInterface, "Controller", "Controller Interface", "Controller Functions");
		this.remoteController = re;
	}	
    
	public ClientController(NATSInterface natsInterface, Remote re) throws RemoteException {
		super(natsInterface, "Controller", "Controller Interface", "Controller Functions");
		this.remoteController = (RemoteController)re;
	}
	
	public int setDelayPeriod(String acid, AircraftClearance aircraft_clearance, float seconds) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((acid != null) && (!acid.equals("")) && (seconds > 0)) {
				retValue = remoteController.setDelayPeriod(acid, aircraft_clearance, seconds);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setActionRepeat(String controllerAircraftID, String controllerRepeatParameter) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.setActionRepeat(sessionId, controllerAircraftID, controllerRepeatParameter);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int skipFlightPhase(String controllerAircraftID, String controllerFlightPhase) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.skipFlightPhase(sessionId, controllerAircraftID, controllerFlightPhase);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setWrongAction(String controllerAircraftID, String controllerOriginalChangeParameter, String controllerWrongChangeParameter) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.setWrongAction(sessionId, controllerAircraftID, controllerOriginalChangeParameter,  controllerWrongChangeParameter);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setActionReversal(String controllerAircraftID, String controllerChangeParameter) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.setActionReversal(sessionId, controllerAircraftID, controllerChangeParameter);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setPartialAction(String controllerAircraftID, String controllerChangeParameter, float controllerOriginalTarget, float controllerPercentage) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.setPartialAction(sessionId, controllerAircraftID, controllerChangeParameter, controllerOriginalTarget, controllerPercentage);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int skipChangeAction(String controllerAircraftID, String controllerSkipParameter) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.skipChangeAction(sessionId, controllerAircraftID, controllerSkipParameter);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setActionLag(String controllerAircraftID, String controllerLagParameter, float controllerLagTimeConstant, float controllerPercentageError, float controllerParameterTarget) {
		int retValue = 1; // Default.  Means error
		
		try {
			if ((controllerAircraftID != null) && (!controllerAircraftID.equals(""))) {
				retValue = remoteController.setActionLag(sessionId, controllerAircraftID, controllerLagParameter, controllerLagTimeConstant, controllerPercentageError, controllerParameterTarget);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int setControllerAbsence(String controllerAircraftID, int timeSteps) {
		int retValue = 1; // Default.  Means error
		
		try {
			if (timeSteps > 0) {
				retValue = remoteController.setControllerAbsence(sessionId, controllerAircraftID, timeSteps);
			}
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int releaseAircraftHold(String controllerAircraftID, String approach, String targetWaypoint) throws RemoteException, Exception {
		int retValue = 1; // Default.  Means error
		
		try {
			retValue = remoteController.releaseAircraftHold(sessionId, controllerAircraftID, approach, targetWaypoint);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public void enableConflictDetectionAndResolution(boolean flag) {
		try {
			remoteController.enableConflictDetectionAndResolution(flag);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public Object[][] getCDR_status() throws RemoteException {
		return remoteController.getCDR_status();
	}
	
	public void setCDR_initiation_distance_ft_surface(float distance) {
		try {
			remoteController.setCDR_initiation_distance_ft_surface(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setCDR_initiation_distance_ft_terminal(float distance) {
		try {
			remoteController.setCDR_initiation_distance_ft_terminal(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setCDR_initiation_distance_ft_enroute(float distance) {
		try {
			remoteController.setCDR_initiation_distance_ft_enroute(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}

	public void setCDR_separation_distance_ft_surface(float distance) {
		try {
			remoteController.setCDR_separation_distance_ft_surface(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setCDR_separation_distance_ft_terminal(float distance) {
		try {
			remoteController.setCDR_separation_distance_ft_terminal(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setCDR_separation_distance_ft_enroute(float distance) {
		try {
			remoteController.setCDR_separation_distance_ft_enroute(distance);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void enableMergingAndSpacingAtMeterFix(String centerId, String meterFix, String spacingType, float spacing) {
		try {
			remoteController.enableMergingAndSpacingAtMeterFix(centerId, meterFix, spacingType, spacing);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void disableMergingAndSpacingAtMeterFix(String centerId, String meterFix) {
		try {
			remoteController.disableMergingAndSpacingAtMeterFix(centerId, meterFix);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void enableStrategicWeatherAvoidance(boolean flag) {
		try {
			remoteController.enableStrategicWeatherAvoidance(flag);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setWeather_pirepFile(String pathFilename){
		try {
			remoteController.setWeather_pirepFile(pathFilename);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setWeather_polygonFile(String pathFilename){
		try {
			remoteController.setWeather_polygonFile(pathFilename);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public void setWeather_sigmetFile(String pathFilename) {
		try {
			remoteController.setWeather_sigmetFile(pathFilename);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
	}
	
	public int setTacticalWeatherAvoidance(String waypoint_name, float duration_sec) {
		int retValue = 1;
		
		try {
			retValue = remoteController.setTacticalWeatherAvoidance(waypoint_name, duration_sec);
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		return retValue;
	}
	
	public int insertAirborneWaypoint(String acid,
			int index_to_insert,
			String waypoint_type,
			String waypoint_name,
			float waypoint_latitude,
			float waypoint_longitude,
			float waypoint_altitude,
			float waypoint_speed_lim,
			String spdlim_desc) throws RemoteException {
		int retValue = 1;
		
		retValue = remoteController.insertAirborneWaypoint(acid, index_to_insert, waypoint_type, waypoint_name, waypoint_latitude, waypoint_longitude, waypoint_altitude, waypoint_speed_lim, spdlim_desc);
		
		return retValue;
	}
	
	public int deleteAirborneWaypoint(String acid,
			int index_to_delete) throws RemoteException {
		int retValue = 1;
		
		retValue = remoteController.deleteAirborneWaypoint(acid, index_to_delete);
		
		return retValue;
	}
	
	public void setTargetWaypoint(String acid,
			int index_of_target) throws RemoteException {
		remoteController.setTargetWaypoint(acid, 1, index_of_target);
	}
}
