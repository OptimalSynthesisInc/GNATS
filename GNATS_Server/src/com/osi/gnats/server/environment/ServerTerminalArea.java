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

package com.osi.gnats.server.environment;

import java.rmi.RemoteException;

import com.osi.gnats.rmi.environment.RemoteTerminalArea;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import java.lang.Math;
import java.io.File;

public class ServerTerminalArea extends ServerClass implements RemoteTerminalArea {
	
	public ServerTerminalArea(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	public String getCurrentSid(String acid) {
		String retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getCurrentSidStarApproach(acid, "SID");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getCurrentStar(String acid) {
		String retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getCurrentSidStarApproach(acid, "STAR");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getCurrentApproach(String acid) {
		String retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getCurrentSidStarApproach(acid, "APPROACH");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAllSids(String airport_code) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getAllSidsStarsApproaches(airport_code, "SID");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAllStars(String airport_code) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getAllSidsStarsApproaches(airport_code, "STAR");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getAllApproaches(String airport_code) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getAllSidsStarsApproaches(airport_code, "APPROACH");
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getProcedure_leg_names(String proc_type, String proc_name, String airport_code) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_leg_names(proc_type, proc_name, airport_code);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String[] getWaypoints_in_procedure_leg(String proc_type, String proc_name, String airport_code, String proc_leg_name) {
		String[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getWaypoints_in_procedure_leg(proc_type, proc_name, airport_code, proc_leg_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double calculateWaypointDistance(float lat1, float lng1, float lat2, float lng2) {
		double earthRadius = 6371000; //meters
	    double dLat = lat2-lat1;
	    double dLng = lng2-lng1;
	    double a = Math.sin(dLat/2) * Math.sin(dLat/2) +
	               Math.cos(lat1) * Math.cos(lat2) *
	               Math.sin(dLng/2) * Math.sin(dLng/2);
	    double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
	    float dist = (float) (earthRadius * c);

	    return dist * 3.2808;
	}

	public int getClosestWaypoint(float[][] waypointOptions, float[] targetWaypoint) {
		int shortestDistanceWaypointIndex = 0;
		double shortestDistance = 999999999;
		for(int i = 0; i < waypointOptions.length; i++) {
			double wpDistance = calculateWaypointDistance(waypointOptions[i][0], waypointOptions[i][1], targetWaypoint[0], targetWaypoint[1]);
			if(wpDistance < shortestDistance) {
				shortestDistance = wpDistance;
				shortestDistanceWaypointIndex = i;
			}
		}
		return shortestDistanceWaypointIndex;
	}
	
	public double[] getWaypoint_Latitude_Longitude_deg(String waypoint_name) {
		double[] retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getWaypoint_Latitude_Longitude_deg(waypoint_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double getProcedure_alt_1(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name) {
		double retValue = -1;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_alt_1(proc_type, proc_name, airport_code, proc_leg_name, proc_wp_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double getProcedure_alt_2(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name) {
		double retValue = -1;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_alt_2(proc_type, proc_name, airport_code, proc_leg_name, proc_wp_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public double getProcedure_speed_limit(String proc_type, String proc_name, String airport_code, String proc_leg_name, String proc_wp_name) {
		double retValue = -1;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_speed_limit(proc_type, proc_name, airport_code, proc_leg_name, proc_wp_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getProcedure_alt_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name) {
		String retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_alt_desc(proc_type, proc_name, airport_code, proc_leg_name, wp_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}
	
	public String getProcedure_speed_limit_desc(String proc_type, String proc_name, String airport_code, String proc_leg_name, String wp_name) {
		String retValue = null;
		
		
		if(ServerNATS.cifpExists) {	
			retValue = cEngine.getProcedure_speed_limit_desc(proc_type, proc_name, airport_code, proc_leg_name, wp_name);
		}
		else {
			System.out.println("This function won't work due to absence of FAA CIFP file.");
		}
		
		return retValue;
	}

}
