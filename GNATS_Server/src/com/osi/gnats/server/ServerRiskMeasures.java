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

package com.osi.gnats.server;

import java.rmi.RemoteException;
import java.util.Scanner;
import java.io.File;
import java.io.FileNotFoundException;
import java.util.Arrays;
import java.util.ArrayList;
import java.awt.geom.*;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.rmi.RemoteRiskMeasures;

/**
 * Actual RMI class which implements RemoteRiskMeasures
 */
public class ServerRiskMeasures extends ServerClass implements RemoteRiskMeasures {

	/**
	 * Constructor
	 * @param serverNATS
	 * @throws RemoteException
	 */
	public ServerRiskMeasures(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	}
	
	/*
	 * Function to check existence of element in array.
	 */
	public static boolean contains(String[] array, String key) {
	      for (String element : array) {
	          if (key.equals(element)) {
	              return true;
	          }
	      }
	      
	      return false;
	}
	
	/*
	 * Function to calculate relative velocity (in knots) of an aircraft to a reference aircraft.
	 * Steps to calculate relative velocity:
		- Consider two aircraft in the airspace, one heading at 10 o’clock and the other one at 2 o’clock.
		  Both these aircraft are going to collide being at altitudes less than safe limits, dangerously close 
		  to each other. (Basically within the bounding box).
		- The velocity component along their current course is fetched. Either of the aircraft might be in 
		  climb, descend, or cruise phase. For each of them the velocity component is scaled in the vertical plane 
		  as the cosine component of their respective flight path angles which are w.r.t. horizon (-pi to +pi).
		- These velocity vectors might be pointing in different directions based on the aircraft. To address this, 
		  the vertical is assumed as reference, and the velocity are scaled in the horizontal plane as the cosine 
		  component of their course.
		- The magnitude difference between these velocity vectors is returned to the user.
	 */
	public double calculateRelativeVelocity(double refSpeed, double refCourse, double refFpa, double tempSpeed, double tempCourse, double tempFpa) {
		double refVelocityVertical, refVelocityHorizontal;
		double tempVelocityVertical,tempVelocityHorizontal;
		
		refVelocityVertical = refSpeed * Math.cos(refFpa);
		tempVelocityVertical = tempSpeed * Math.cos(tempFpa);
		refVelocityHorizontal = refVelocityVertical * Math.cos(refCourse);
		tempVelocityHorizontal = tempVelocityVertical * Math.cos(tempCourse);
		
		return (tempVelocityHorizontal - refVelocityHorizontal);
	}
	
	/*
	 * Function to calculate bearing angle (in degrees) between to (lat,lon) points.
	 */
	public double calculateBearing(double lat1, double lon1, double lat2, double lon2) {
		double longitude1, longitude2, latitude1, latitude2, longitudeDifference, x, y, bearingAngle;
		
		longitude1 = lon1;
		longitude2 = lon2;
		latitude1 = Math.toRadians(lat1);
		latitude2 = Math.toRadians(lat2);
		longitudeDifference = Math.toRadians(longitude2-longitude1);
		y = Math.sin(longitudeDifference)*Math.cos(latitude2);
		x = Math.cos(latitude1)*Math.sin(latitude2)-Math.sin(latitude1)*Math.cos(latitude2)*Math.cos(longitudeDifference);  
		bearingAngle = (Math.toDegrees(Math.atan2(y, x))+360)%360;
		
		return bearingAngle;
	}
	
	/*
	 * This function returns the curved distance (in miles) between points (lat1, lon1, alt1) and (lat2, lon2, alt2).
	 */
	public double calculateDistance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
		final int R = 6371;
		double latDistance, lonDistance, a, c, distance, altDiff;
		
	    latDistance = Math.toRadians(lat2 - lat1);
	    lonDistance = Math.toRadians(lon2 - lon1);
	    a = Math.sin(latDistance / 2) * Math.sin(latDistance / 2)
	            + Math.cos(Math.toRadians(lat1)) * Math.cos(Math.toRadians(lat2))
	            * Math.sin(lonDistance / 2) * Math.sin(lonDistance / 2);
	    c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
	    distance = R * c * 1000; //Kilometer to meters

	    altDiff = (alt1 - alt2) * 0.3048; // Feet to meters
	    distance = Math.pow(distance, 2) + Math.pow(altDiff, 2);
	    distance = Math.sqrt(distance) * 0.00062137;
	    
	    return distance;
	}
	
	/*
	 * This function returns the runway points (Threshold and end) in the form ((latitudeThreshold, longitudeThreshold),(latitudeEnd, longitudeEnd))
	 */
	public double[][] getRunwayEnds(String airportId, String runway) {
		double[][] runwayEnds;
		double runwayThresholdLat = 0.0, runwayThresholdLon = 0.0, runwayEndLat = 0.0, runwayEndLon = 0.0;
		String[] runwayExits;
		String runwayThreshold, runwayEnd;
		Object[] airportNodeMap, airportNodeData;
		int thresholdNodeID= 0, endNodeID = 0;
		
		runwayExits = cEngine.getRunwayExits(airportId, runway);
		runwayThreshold = runwayExits[0];
		runwayEnd = runwayExits[runwayExits.length - 1];
		airportNodeMap = cEngine.getLayout_node_map(airportId);
		airportNodeData = cEngine.getLayout_node_data(airportId);
		for (int i = 0; i < airportNodeMap.length; i++) {
			if (((Object[]) airportNodeMap[i])[0].toString().equals(runwayThreshold)) {
				thresholdNodeID = (int) ((Object[]) airportNodeMap[i])[1];
			}
			else if (((Object[]) airportNodeMap[i])[0].toString().equals(runwayEnd)) {
				endNodeID = (int) ((Object[]) airportNodeMap[i])[1];
			}
		}

		for (int i = 0; i < airportNodeData.length; i++) {
			if ((int) ((Object[]) airportNodeData[i])[0] == thresholdNodeID) {
				runwayThresholdLat = (double) ((Object[]) airportNodeData[i])[1];
				runwayThresholdLon = (double) ((Object[]) airportNodeData[i])[2];
			}
			else if ((int) ((Object[]) airportNodeData[i])[0] == endNodeID) {
				runwayEndLat = (double) ((Object[]) airportNodeData[i])[1];
				runwayEndLon = (double) ((Object[]) airportNodeData[i])[2];
			}
		}
		runwayEnds = new double[][] { { runwayThresholdLat, runwayThresholdLon }, { runwayEndLat, runwayEndLon } };
	
		return runwayEnds;
		
	}
	
	/*
	 * This function returns the flight data for aircrafts within the safety hazard range of a reference aircraft.
	 * This function takes in a particular aircraft callsign as input. It would form a bounding box around the 
	 * aircraft that would simulate the area under consideration for potential safety hazards. The aircraft are 
	 * filtered to get the ones that lie within this box +/- 1000 ft in altitude of the reference aircraft. 
	 * These flights would be analyzed for their position and velocity, relative to the reference aircraft. 
	 * This information is then returned to the user.
	 */
	public Object getFlightsInRange(int sessionId, String refAircraftId) {

		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, refAircraftId);
		
		if(!validAircraft) {
			System.out.println(refAircraftId + " is an Invalid Flight.");
			return null;
		}
		
		Aircraft aircraft;
		
		double refLat, refLon, refAlt, refSpeed, refFpa, refCourse, angle, distance, ax, ay, bx, by, cx, cy, dx, dy;
		double tempLat, tempLon, tempAlt, tempSpeed, tempFpa, tempCourse, maxAlt, minAlt;
		double[] latitudes, longitudes;
				
		ArrayList tempList = new ArrayList();
		ArrayList<String> flightsScanned = new ArrayList<String>();
		ArrayList tempListClone;
		ArrayList<ArrayList> qualifiedAircrafts = new ArrayList<ArrayList>();
				
		aircraft = cEngine.select_aircraft(sessionId, refAircraftId);
		refLat = aircraft.getLatitude_deg();
		refLon = aircraft.getLongitude_deg();
		refAlt = aircraft.getAltitude_ft();
		refSpeed = aircraft.getTas_knots();
		refCourse = aircraft.getCourse_rad() * 180/Math.PI;
		refFpa = aircraft.getFpa_rad() * 180/Math.PI;
		
		// +/- 0.5 degree of lat/lon from reference to form safety bounding box.
		ax = refLat + 0.5f;
		ay = refLon + 0.5f;
		bx = refLat + 0.5f;
		by = refLon - 0.5f;
		cx = refLat - 0.5f;
		cy = refLon - 0.5f;
		dx = refLat - 0.5f;
		dy = refLon + 0.5f;

		
		latitudes = new double[] { ax, bx, cx, dx };
		longitudes = new double[] { ay, by, cy, dy };
		Path2D path = new Path2D.Double();
		path.moveTo(latitudes[0], longitudes[0]);
		for (int i = 1; i < latitudes.length; ++i) {
			path.lineTo(latitudes[i], longitudes[i]);
		}
		path.closePath();
		
		maxAlt = refAlt + 2000;
		minAlt = refAlt - 2000;
		for (int i = 0; i < aircraftIds.length; i++) {

			aircraft = cEngine.select_aircraft(sessionId, aircraftIds[i]);
			tempLat = aircraft.getLatitude_deg();
			tempLon = aircraft.getLongitude_deg();
			tempAlt = aircraft.getAltitude_ft();
			tempSpeed = aircraft.getTas_knots();
			tempFpa = aircraft.getFpa_rad() * 180/Math.PI;
			tempCourse = aircraft.getCourse_rad() * 180/Math.PI;
			
			//The altitude, (lat,lon) position, and aircraft ID are used as filters to select flights within safety hazard area.
			if (path.contains(tempLat, tempLon) && tempAlt < maxAlt && tempAlt > minAlt && !aircraftIds[i].equals(refAircraftId)) {
				tempList.add(aircraftIds[i]);
				tempList.add(calculateRelativeVelocity(refSpeed, refCourse, refFpa, tempSpeed, tempCourse, tempFpa));
				tempList.add(tempAlt - refAlt);
				tempList.add(calculateBearing(refLat, refLon, tempLat, tempLon));
				tempList.add(calculateDistance(refLat, refLon, refAlt, tempLat, tempLon, tempAlt));
				tempListClone =  (ArrayList) tempList.clone();
				qualifiedAircrafts.add(tempListClone);
				flightsScanned.add(aircraftIds[i]);
				tempList.clear();
			}
		}

		return (Object)qualifiedAircrafts;
	}

	/*
	 * This function calculates the distance to reach runway threshold. This is the curved distance 
	 * of the current position of the aicrcraft to ground elevation.
	 */
	public double getDistanceToRunwayThreshold(int sessionId, String aircraftId) {
		
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return Double.NaN;
		}
		
		double[][] ends;
		double currentLat, currentLon;
		String arrivalRunway, arrivalAirport;
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);

		currentLat = aircraft.getLatitude_deg();
		currentLon = aircraft.getLongitude_deg();
		
		arrivalAirport = cEngine.getArrivalAirport(aircraftId);
		arrivalRunway = cEngine.getArrivalRunway(aircraftId);
		
		ends = getRunwayEnds(arrivalAirport, arrivalRunway);
		return calculateDistance(currentLat, currentLon, 0.0, ends[0][0], ends[0][1], 0.0);
	}

	/*
	 * This function calculates the distance to reach the end of the runway. This is linear distance
	 * from current position to runway end.
	 */
	public double getDistanceToRunwayEnd(int sessionId, String aircraftId) {
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return Double.NaN;
		}
		double[][] ends;
		double currentLat, currentLon;
		String departureRunway, departureAirport;
		Aircraft aircraft = null;
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);

		currentLat = aircraft.getLatitude_deg();
		currentLon = aircraft.getLongitude_deg();
		
		departureAirport = cEngine.getDepartureAirport(aircraftId);
		departureRunway = cEngine.getDepartureRunway(aircraftId);
		
		ends = getRunwayEnds(departureAirport, departureRunway);
		return calculateDistance(currentLat, currentLon, 0.0, ends[1][0], ends[1][1], 0.0);
	}
	
	/*
	 * For aircraft either in landing or takeoff phase, this function calculates and yields 
	 * the alignment (in degrees) of the runway in reference to the current course of aircraft velocity.
	 * The procedure argument can be either "DEPARTURE" or "ARRIVAL" for determining 
	 * reference airport and runway.
	 */
	public double getVelocityAlignmentWithRunway(int sessionId, String aircraftId, String procedure) {
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if (!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return Double.NaN;
		}
		else if (!procedure.equals("DEPARTURE") && !procedure.equals("ARRIVAL")) {
			System.out.println(procedure + " is invalid. Use \"DEPARTURE\" or \"ARRIVAL\".");
			return Double.NaN;
		}
		double alignmentAngle, aircraftCourse, runwayHeading;
		double[][] runwayEnds;

		String airport = null, runway= null;
		Aircraft aircraft;
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		if(procedure.equals("DEPARTURE")) {
			airport = cEngine.getDepartureAirport(aircraftId);
			runway = cEngine.getDepartureRunway(aircraftId);
		}
		else if(procedure.equals("ARRIVAL")) {
			airport = cEngine.getArrivalAirport(aircraftId);
			runway = cEngine.getArrivalRunway(aircraftId);
		}

		aircraftCourse = aircraft.getCourse_rad() * 180/Math.PI;
		runwayEnds = getRunwayEnds(airport, runway);
		runwayHeading = calculateBearing(runwayEnds[0][0], runwayEnds[0][1], runwayEnds[1][0], runwayEnds[1][1]);
		alignmentAngle = aircraftCourse - runwayHeading;

		return alignmentAngle;
		
	}
	
	/*
	 * This function returns the number of passengers occupying a particular aircraft, assuming 
	 * 100% load factor. Data for all aircraft types in the ADB database are provided.
	 */
	public Integer getPassengerCount(String aircraftType) {
		String aircraftDataFile = "share/AircraftData/AircraftData.csv";
		Integer passengerCount = null;
		Scanner scanner = null;
		try {
			scanner = new Scanner(new File(aircraftDataFile));
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		scanner.useDelimiter("\\n");
		while(scanner.hasNext()){
			String[] aircraftEntry = scanner.next().split(",");
			if(aircraftEntry[0].equals(aircraftType)) {
				passengerCount = Integer.parseInt(aircraftEntry[1]);
			}
        }
		scanner.close();
        if (passengerCount == null) {
        	System.out.println(aircraftType + " is an invalid aircraft type.");
        }
		return passengerCount;
	}
	
	/*
	 * This function returns the cost (in million US Dollars) of a particular
	 * aircraft. Data for all aircraft types in the ADB database are provided. 
	 */
	public double getAircraftCost(String aircraftType) {
		String aircraftDataFile = "share/AircraftData/AircraftData.csv";
		double aircraftCost = Double.NaN;
		Scanner scanner = null;
		try {
			scanner = new Scanner(new File(aircraftDataFile));
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		scanner.useDelimiter("\\n");
		while(scanner.hasNext()){
			String[] aircraftEntry = scanner.next().split(",");
			if(aircraftEntry[0].equals(aircraftType)) {
				aircraftCost = Double.parseDouble(aircraftEntry[2]);
			}
        }
		scanner.close();
		if (Double.isNaN(aircraftCost)) {
        	System.out.println(aircraftType + " is an invalid aircraft type.");
        }
        return aircraftCost;
	}
	
	
	/*
	 * This function returns the flight data for aircrafts within the wake vortex range of a reference aircraft.
	 * This function takes in a particular aircraft callsign and wake vortex boundary dimensions as input. It would 
	 * form a bounding box around the aircraft based on the parameters that would simulate the area under consideration 
	 * for potential safety hazards. The aircraft are filtered to get the ones that lie within this box of the reference aircraft. 
	 * These flights would be analyzed for their position and velocity, relative to the reference aircraft. 
	 * This information is then returned to the user.
	 */
	public Object getFlightsInWakeVortexRange(int sessionId, String refAircraftId, float envelopeStartLength, float envelopeStartBreadth, float envelopeEndLength, float envelopeEndBreadth, float envelopeRangeLength, float envelopeDecline) {

		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, refAircraftId);
		
		if(!validAircraft) {
			System.out.println(refAircraftId + " is an Invalid Flight.");
			return null;
		}
		else if (envelopeStartLength < 0 || envelopeStartBreadth < 0 || envelopeEndLength < 0 || envelopeEndBreadth < 0 || envelopeDecline < 0 || envelopeRangeLength < 0) {
			System.out.println("Invalid parameters to build wake vortex envelope.");
			return null;
		}
		
		Aircraft aircraft;
		
		double refLatDeg, refLonDeg, refLat, refLon, refAlt, refSpeed, refFpa, refCourse, vortexSlope, distance, percentageDecline, ax, ay, bx, by, cx, cy, dx, dy;
		double tempLat, tempLon, tempLatMap, tempLonMap, tempAlt, tempSpeed, tempFpa, tempCourse, maxAlt, minAlt, maxRangeLat, maxRangeLon, vortexHorizontalAngle, vortexHorizontalSlope, distanceToRadius, distanceStep;
		double[] latitudes, longitudes;
				
		ArrayList tempList = new ArrayList();
		ArrayList<String> flightsScanned = new ArrayList<String>();
		ArrayList tempListClone;
		ArrayList<ArrayList> qualifiedAircrafts = new ArrayList<ArrayList>();
				
		aircraft = cEngine.select_aircraft(sessionId, refAircraftId);
		refLat = aircraft.getLatitude_deg() * Math.PI / 180;
		refLon = aircraft.getLongitude_deg() * Math.PI / 180;
		refLatDeg = aircraft.getLatitude_deg();
		refLonDeg = aircraft.getLongitude_deg();
		refAlt = aircraft.getAltitude_ft();
		refSpeed = aircraft.getTas_knots();
		refCourse = aircraft.getCourse_rad();
		refFpa = aircraft.getFpa_rad() * 180/Math.PI;
		vortexHorizontalAngle = (refCourse + Math.PI) % (2 * Math.PI);

		envelopeRangeLength *= 1.6;
		distanceToRadius = envelopeRangeLength / 6378.1;

		maxRangeLat = Math.asin(Math.sin(refLat) * Math.cos(distanceToRadius) + Math.cos(refLat) * Math.sin(distanceToRadius) * Math.cos(vortexHorizontalAngle));
	    maxRangeLon = refLon + Math.atan2(Math.sin(vortexHorizontalAngle) * Math.sin(distanceToRadius) * Math.cos(refLat), Math.cos(distanceToRadius) - Math.sin(refLat) * Math.sin(maxRangeLat));

	    vortexSlope = (maxRangeLon - refLon) / (maxRangeLat - refLat);
	    
	    distanceToRadius = ((envelopeStartLength * 0.0003048) / (2 * 6378.1));
			
		ax = (Math.asin(Math.sin(refLat) * Math.cos(distanceToRadius) + Math.cos(refLat) * Math.sin(distanceToRadius) * Math.cos(refCourse + 1.57)));
	    ay = (refLon + Math.atan2(Math.sin(refCourse + 1.57) * Math.sin(distanceToRadius) * Math.cos(refLat), Math.cos(distanceToRadius) - Math.sin(refLat) * Math.sin(ax)));

		bx = (Math.asin(Math.sin(refLat) * Math.cos(distanceToRadius) + Math.cos(refLat) * Math.sin(distanceToRadius) * Math.cos(refCourse - 1.57)));
	    by = (refLon + Math.atan2(Math.sin(refCourse - 1.57) * Math.sin(distanceToRadius) * Math.cos(refLat), Math.cos(distanceToRadius) - Math.sin(refLat) * Math.sin(bx)));

	    distanceToRadius = ((envelopeEndLength * 0.0003048) / (2 * 6378.1));
	    
	    cx = (Math.asin(Math.sin(maxRangeLat) * Math.cos(distanceToRadius) + Math.cos(maxRangeLat) * Math.sin(distanceToRadius) * Math.cos(refCourse + 1.57)));
	    cy = (maxRangeLon + Math.atan2(Math.sin(refCourse + 1.57) * Math.sin(distanceToRadius) * Math.cos(maxRangeLat), Math.cos(distanceToRadius) - Math.sin(maxRangeLat) * Math.sin(cx)));

		dx = (Math.asin(Math.sin(maxRangeLat) * Math.cos(distanceToRadius) + Math.cos(maxRangeLat) * Math.sin(distanceToRadius) * Math.cos(refCourse - 1.57)));
	    dy = (maxRangeLon + Math.atan2(Math.sin(refCourse - 1.57) * Math.sin(distanceToRadius) * Math.cos(maxRangeLat), Math.cos(distanceToRadius) - Math.sin(maxRangeLat) * Math.sin(dx)));

		latitudes = new double[] { ax * 180 / Math.PI, bx * 180 / Math.PI, cx * 180 / Math.PI, dx * 180 / Math.PI };
		longitudes = new double[] { ay * 180 / Math.PI, by * 180 / Math.PI, cy * 180 / Math.PI, dy * 180 / Math.PI };
		Path2D path = new Path2D.Double();
		path.moveTo(latitudes[0], longitudes[0]);
		for (int i = 1; i < latitudes.length; ++i) {
			path.lineTo(latitudes[i], longitudes[i]);
		}
		path.closePath();
			
		
		for (int i = 0; i < aircraftIds.length; i++) {
			
			aircraft = cEngine.select_aircraft(sessionId, aircraftIds[i]);
			tempLat = aircraft.getLatitude_deg();
			tempLon = aircraft.getLongitude_deg();
			tempAlt = aircraft.getAltitude_ft();
			tempSpeed = aircraft.getTas_knots();
			tempFpa = aircraft.getFpa_rad() * 180/Math.PI;
			tempCourse = aircraft.getCourse_rad() * 180/Math.PI;
			tempLatMap = (refLatDeg / vortexSlope + refLonDeg + vortexSlope * tempLat - tempLon) / (1/vortexSlope + vortexSlope);
			tempLonMap = (-1 / vortexSlope) * tempLatMap + (1 / vortexSlope) * refLatDeg + refLonDeg;
			
			//The altitude, (lat,lon) position, and aircraft ID are used as filters to select flights within safety hazard area.
			if (path.contains(tempLat, tempLon) && !aircraftIds[i].equals(refAircraftId) && !flightsScanned.contains(aircraftIds[i])) {
				
				percentageDecline = calculateDistance(tempLatMap, tempLonMap, tempAlt, refLatDeg, refLonDeg, refAlt) / envelopeRangeLength * 100;
			    
				maxAlt = refAlt + (envelopeStartBreadth/2 - (percentageDecline / 100 * envelopeDecline));
				minAlt = refAlt - (envelopeStartBreadth/2 + (percentageDecline / 100 * envelopeDecline));
				
				if (tempAlt < maxAlt && tempAlt > minAlt) {
					tempList.add(aircraftIds[i]);
					tempList.add(calculateRelativeVelocity(refSpeed, refCourse * 180/Math.PI, refFpa, tempSpeed, tempCourse, tempFpa));
					tempList.add(tempAlt - refAlt);
					tempList.add(calculateBearing(refLatDeg, refLonDeg, tempLat, tempLon));
					tempList.add(calculateDistance(refLatDeg, refLonDeg, refAlt, tempLat, tempLon, tempAlt));
					tempListClone =  (ArrayList) tempList.clone();
					qualifiedAircrafts.add(tempListClone);
					flightsScanned.add(aircraftIds[i]);
					tempList.clear();
				}
			}
		}

		return (Object)qualifiedAircrafts;
	}
	
	/**
	 * Set aircraft book value in USD
	 * @param aircraftId Aircraft Callsign
	 * @param aircraftBookValue The aircraft book value in USD
	 */
	public int setAircraftBookValue(int sessionId, String aircraftId, double aircraftBookValue) {
		int retVal = 1;
		String[] aircraftIds;
		Aircraft aircraft = null;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retVal;
		}
		
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		if(aircraftBookValue > 0) {
			cEngine.setAircraftBookValue(aircraftId, aircraftBookValue);
			retVal = 0;
		}
		return retVal;
	}

	/**
	 * Get aircraft book value in USD
	 * @param aircraftId Aircraft Callsign
	 */
	public double getAircraftBookValue(int sessionId, String aircraftId) {
		
		Aircraft aircraft = null;
		double retValue = Double.NaN;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		retValue = cEngine.getAircraftBookValue(aircraftId);
		
		return retValue;	
	}
	
	/**
	 * Set cargo worth in millions USD
	 * @param aircraftId Aircraft Callsign
	 * @param cargoWorth The cargo worth in millions USD
	 */
	public int setCargoWorth(int sessionId, String aircraftId, double cargoWorth) {
		int retVal = 1;
		Aircraft aircraft = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retVal;
		}
		
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		if(cargoWorth > 0) {
			cEngine.setCargoWorth(aircraftId, cargoWorth);
			retVal = 0;
		}
		return retVal;
	}

	/**
	 * Get cargo worth in USD
	 * @param aircraftId Aircraft Callsign
	 */
	public double getCargoWorth(int sessionId, String aircraftId) {
		Aircraft aircraft = null;
		double retValue = Double.NaN;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		retValue = cEngine.getCargoWorth(aircraftId);
		
		return retValue;
	}
	
	/**
	 * Set passenger load factor
	 * @param aircraftId Aircraft Callsign
	 * @param paxLoadFactor The passenger load factor (ranging from 0.0 to 1.0)
	 */
	public int setPassengerLoadFactor(int sessionId, String aircraftId, double paxLoadFactor) {
		int retVal = 1;
		Aircraft aircraft = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retVal;
		}
		
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		if(paxLoadFactor >= 0.0 && paxLoadFactor <= 1.0) {
			cEngine.setPassengerLoadFactor(aircraftId, paxLoadFactor);
			retVal = 0;
		}
		return retVal;
	}
		
	/**
	 * Get passenger load factor
	 * @param aircraftId Aircraft Callsign
	 */
	public double getPassengerLoadFactor(int sessionId, String aircraftId) {
		Aircraft aircraft = null;
		double retValue = Double.NaN;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		retValue = cEngine.getPassengerLoadFactor(aircraftId);
		
		return retValue;
	}
	
	public double calculateWaypointDistance(float lat1, float lng1, float lat2, float lng2) {
		double earthRadius = 6371000; //meters
	    lat1 *= Math.PI / 180.0;
	    lat2 *= Math.PI / 180.0;
	    lng1 *= Math.PI / 180.0;
	    lng2 *= Math.PI / 180.0;
	    double dLat = lat2-lat1;
	    double dLng = lng2-lng1;
	    double a = Math.sin(dLat/2) * Math.sin(dLat/2) +
	               Math.cos(lat1) * Math.cos(lat2) *
	               Math.sin(dLng/2) * Math.sin(dLng/2);
	    double c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));
	    float dist = (float) (earthRadius * c);

	    return dist * 3.2808;
	}
	
	
	/**
	 * Set aircraft touch down point on runway for landing
	 * @param aircraftId Aircraft Callsign
	 * @param latitude The latitude of touchdown point
	 * @param longitude The longitude of touchdown point
	 */
	public int setTouchdownPointOnRunway(int sessionId, String aircraftId, double latitude, double longitude) {
		int retVal = 1;
		Aircraft aircraft = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retVal;
		}
		
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		cEngine.setTouchdownPointOnRunway(aircraftId, latitude, longitude);

		return retVal;
	}
		
	/**
	 * Get aircraft touch down point on runway for landing
	 * @param aircraftId Aircraft Callsign
	 */
	public double[] getTouchdownPointOnRunway(int sessionId, String aircraftId) {
		Aircraft aircraft = null;
		double[] retValue = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		retValue = cEngine.getTouchdownPointOnRunway(aircraftId);
		
		return retValue;
	}
	
	
	/**
	 * Set aircraft take off point on runway for liftoff
	 * @param aircraftId Aircraft Callsign
	 * @param latitude The latitude of liftoff point
	 * @param longitude The longitude of liftoff point
	 */
	public int setTakeOffPointOnRunway(int sessionId, String aircraftId, double latitude, double longitude) {
		int retVal = 1;
		Aircraft aircraft = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retVal;
		}
		
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		cEngine.setTakeOffPointOnRunway(aircraftId, latitude, longitude);

		return retVal;
	}
		
	/**
	 * Get aircraft take off point on runway for liftoff
	 * @param aircraftId Aircraft Callsign
	 */
	public double[] getTakeOffPointOnRunway(int sessionId, String aircraftId) {
		Aircraft aircraft = null;
		double[] retValue = null;
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		retValue = cEngine.getTakeOffPointOnRunway(aircraftId);
		
		return retValue;
	}
	
	/**
	 * Get L1 distance between two aircraft.
	 * @param airportId The ICAO code of airport where aircrafts are in taxi
	 * @param aircraftId1 The callsign of first aircraft
	 * @param aircraftId2 The callsign of second aircraft
	 * @return
	 */
	public double getL1Distance(int sessionId, String airportId, String aircraftId1, String aircraftId2) {
		Aircraft aircraft1 = null, aircraft2 = null;
		String aircraft1TaxiPlan[] = null, aircraft2TaxiPlan[] = null;
		float aircraft1Distance = -1.0f, aircraft2Distance = -1.0f, latLon[][] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
		int aircraft1target, aircraft2target, pointOfAircraftContact[] = {0, 0};
		double l1distance = -1.0;

		// Get Aircraft Instances
		aircraft1 = cEngine.select_aircraft(sessionId, aircraftId1);
		aircraft2 = cEngine.select_aircraft(sessionId, aircraftId2);
		
		// Check if aircraft exist
		if ((aircraft1 != null && aircraft2 != null)) {
			// Get Taxi plan waypoint names and (latitude, longitude) positions
			aircraft1TaxiPlan = cEngine.getSurface_taxi_plan(aircraftId1, airportId);
			aircraft2TaxiPlan = cEngine.getSurface_taxi_plan(aircraftId2, airportId);
			
			// Detect index of target waypoint in taxi plan
			aircraft1target = Arrays.asList(aircraft1TaxiPlan).indexOf(aircraft1.getTarget_waypoint_name());
			aircraft2target = Arrays.asList(aircraft2TaxiPlan).indexOf(aircraft2.getTarget_waypoint_name());

			// L1 Distance only if aircraft are currently on the ground at specified airport
			if((aircraft1target < aircraft1TaxiPlan.length) && (aircraft2target < aircraft2TaxiPlan.length)) {
				
				// Check if aircraft are in head-on collision route
				if ((aircraft1target > 0) && aircraft1TaxiPlan[aircraft1target - 1].equals(aircraft2TaxiPlan[aircraft2target]))
						l1distance = (double) calculateWaypointDistance(aircraft1.getLatitude_deg(), aircraft1.getLongitude_deg(), aircraft2.getLatitude_deg(), aircraft2.getLongitude_deg());
				else if ((aircraft2target > 0) && aircraft2TaxiPlan[aircraft2target - 1].equals(aircraft1TaxiPlan[aircraft1target]))
						l1distance = (double) calculateWaypointDistance(aircraft1.getLatitude_deg(), aircraft1.getLongitude_deg(), aircraft2.getLatitude_deg(), aircraft2.getLongitude_deg());
				else if ((aircraft1target > -1) && ((aircraft2target > -1))) {
					// Find common point for aircraft potential aircraft collision
					for (int i = aircraft1target; i < aircraft1TaxiPlan.length; i++) {
			            for (int j = aircraft1target; j < aircraft2TaxiPlan.length; j++)
			                if (aircraft1TaxiPlan[i].equals(aircraft2TaxiPlan[j])) {
			                	pointOfAircraftContact[0] = i;
			                	pointOfAircraftContact[1] = j;
		                		break;
			                }
			            if ((pointOfAircraftContact[0] != 0) && (pointOfAircraftContact[1] != 0))
			            	break;
					}
					
					// Check if point of contact exists
					if((pointOfAircraftContact[0] != 0) && (pointOfAircraftContact[1] != 0)) {
						
						// Add distance from first aircraft to it's target waypoint
						latLon[0] = cEngine.getGroundWaypointLocation(airportId, aircraft1TaxiPlan[aircraft1target]);
						l1distance = calculateWaypointDistance(aircraft1.getLatitude_deg(), aircraft1.getLongitude_deg(), latLon[0][0], latLon[0][1]);

						
						// Calculate cumulative distance from each aircraft to the potential collision point
						for (int i = aircraft1target; i < pointOfAircraftContact.length - 1; i++) {
							latLon[0] = cEngine.getGroundWaypointLocation(airportId, aircraft1TaxiPlan[i]);
							latLon[1] = cEngine.getGroundWaypointLocation(airportId, aircraft1TaxiPlan[i+1]);
							l1distance += calculateWaypointDistance(latLon[0][0], latLon[0][1], latLon[1][0], latLon[1][1]);
						}
						
						// Add distance from second aircraft to it's target waypoint
						latLon[0] = cEngine.getGroundWaypointLocation(airportId, aircraft2TaxiPlan[aircraft2target]);
						l1distance += calculateWaypointDistance(aircraft2.getLatitude_deg(), aircraft2.getLongitude_deg(), latLon[0][0], latLon[0][1]);

						for (int i = aircraft2target; i < pointOfAircraftContact.length - 1; i++) {
							latLon[0] = cEngine.getGroundWaypointLocation(airportId, aircraft2TaxiPlan[i]);
							latLon[1] = cEngine.getGroundWaypointLocation(airportId, aircraft2TaxiPlan[i+1]);
							l1distance += calculateWaypointDistance(latLon[0][0], latLon[0][1], latLon[1][0], latLon[1][1]);
						}
					}
				}
			}
		}
		
		//Return L1 distance rounded to 1 decimal place
		return (double)Math.round(l1distance * 10d) / 10d;
	}
	
	/**
	 * Get distance between aircraft current position and pavement end in direction of course.
	 * @param airportId The ICAO code of airport where aircrafts are in taxi
	 * @param aircraftId Callsign of the aircraft
	 * @return
	 */
	public double getDistanceToPavementEdge(int sessionId, String airportId, String aircraftId) {
		double retValue = -1;
		Aircraft aircraft = null;
		String[] aircraftIds;
		String targetWaypoint = "";
		int currentFlightPhase;
		double currentCourse, correctCourse, buffDistance;
		float[] targetWaypointLocation, tmpWaypointLocation;
		Object[] airportNodeMap;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return retValue;
		}
		
		// Get aircraft instance
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
				
		// Get current course
		currentCourse = (int)(aircraft.getCourse_rad()  * 180.0 / Math.PI);

		// Get target waypoint name
		targetWaypoint = aircraft.getTarget_waypoint_name();
				
		// Get target waypoint latitude and longitude
		targetWaypointLocation = cEngine.getGroundWaypointLocation(airportId, targetWaypoint);
		
		// Calculate bearing from current position to target point
		correctCourse = (int)(calculateBearing(aircraft.getLatitude_deg(), aircraft.getLongitude_deg(), targetWaypointLocation[0], targetWaypointLocation[1]));

		if (currentCourse != correctCourse) {
			
			currentFlightPhase = aircraft.getFlight_phase();
			
			// Aircraft is on ramp
			if(currentFlightPhase == 3 || currentFlightPhase == 23) {
				// Ramp width 60ft
				retValue = Math.sin((correctCourse - currentCourse) * Math.PI / 180.0) * (60 / 2);
			}
			// Aircraft is on Taxiway
			else if (currentFlightPhase == 4 || currentFlightPhase == 21) {
				// Taxiway width 60ft
				retValue = Math.sin((correctCourse - currentCourse) * Math.PI / 180.0) * (60 / 2);
			}
			// Aircraft is on Runway
			else if (currentFlightPhase == 5 || currentFlightPhase == 6) {
				// Runway width 100ft
				retValue = Math.sin((correctCourse - currentCourse) * Math.PI / 180.0) * (100 / 2);
			}
		}
		else {
			
			// Aircraft is on correct course, so pavement end would be the end of runway/taxiway/ramp
			
			int position = targetWaypoint.lastIndexOf("_");
	        airportNodeMap = cEngine.getLayout_node_map(airportId);
	        buffDistance = 0.0;
			for (int i = 0; i < airportNodeMap.length; i++) {
				if (((Object[]) airportNodeMap[i])[0].toString().startsWith(targetWaypoint.substring(0, position))) {
					tmpWaypointLocation = cEngine.getGroundWaypointLocation(airportId, ((Object[]) airportNodeMap[i])[0].toString());
					buffDistance = calculateWaypointDistance(aircraft.getLatitude_deg(), aircraft.getLongitude_deg(), tmpWaypointLocation[0], tmpWaypointLocation[1]);
					if((currentCourse == (int)(calculateBearing(aircraft.getLatitude_deg(), aircraft.getLongitude_deg(), tmpWaypointLocation[0], tmpWaypointLocation[1]))) && (buffDistance > retValue)) {
						retValue = buffDistance;
					}
				}
			}
		}
		
		return (double)Math.round(retValue * 10d) / 10d;
	}
	
	public int load_FlightPhaseSequence(String filename) {
		return cEngine.load_FlightPhaseSequence(filename);
	}
	
	public void clear_FlightPhaseSequence() {
		cEngine.clear_FlightPhaseSequence();
	}
	
	public int load_aviationOccurenceProfile(String dirPath) throws RemoteException {
		return cEngine.load_aviationOccurenceProfile(dirPath);
	}
}
