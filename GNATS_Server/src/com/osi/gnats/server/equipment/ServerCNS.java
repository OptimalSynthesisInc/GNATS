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

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.rmi.equipment.RemoteCNS;
import com.osi.gnats.server.ServerClass;
import com.osi.gnats.server.ServerNATS;
import com.osi.gnats.server.environment.ServerTerrain;
import com.osi.gnats.user.UserManager;
import com.osi.gnats.user.UserPermission;

/**
 * Actual RMI class which implements RemoteCNS
 */
public class ServerCNS extends ServerClass implements RemoteCNS {
	private static ServerTerrain serverTerrain;
	
	public ServerCNS(ServerNATS serverNATS) throws RemoteException {
		super(serverNATS);
	    serverTerrain = new ServerTerrain(serverNATS);
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
	
	/**
	 * Computes line of sight between source and target, returns range, azimuth, and elevation along with potential terrain or earth curvature masking.
	 */
	public double[] getLineOfSight(double observerLat, double observerLon, double observerAlt, double targetLat, double targetLon, double targetAlt) {
		
		// Returns an array of (Range, Azimuth, Elevation, Masking)
		double retVal[] = {0, 0, 0, 0};
		
		// Radius of Earth in ft
		double R = 20902230.97;
		
		// Convert latitude/longitude to radian
		observerLat = Math.toRadians(observerLat);
		observerLon = Math.toRadians(observerLon);
		targetLat = Math.toRadians(targetLat);
		targetLon = Math.toRadians(targetLon);
		
		// Transform Lat, Lon, Alt positinos of observer and target to geoinertial frame
		double xt = (R + targetAlt) * Math.cos(targetLat) * Math.cos(targetLon);
		double yt = (R + targetAlt) * Math.cos(targetLat) * Math.sin(targetLon);
		double zt = (R + targetAlt) * Math.sin(targetLat);
		double xo = (R + observerAlt) * Math.cos(observerLat) * Math.cos(observerLon);
		double yo = (R + observerAlt) * Math.cos(observerLat) * Math.sin(observerLon);
		double zo = (R + observerAlt) * Math.sin(observerLat);

		// Get relative position vectors
		double dxo = xo - xt;
		double dyo = yo - yt;
		double dzo = zo - zt;
		
		// Get transformed relative vector to observer's topocentric frame
		double xl = dxo * Math.sin(observerLat) * Math.cos(observerLon) + dyo * Math.sin(observerLat) * Math.sin(observerLon) - dzo * Math.cos(observerLat);
		double yl = -dxo * Math.sin(observerLat) + dyo * Math.cos(observerLat);
		double zl = dxo * Math.cos(observerLat) * Math.cos(observerLon) + dyo * Math.cos(observerLat) * Math.sin(observerLon) + dzo * Math.sin(observerLat);
		
		
		// Calculate Range
		retVal[0] = Math.sqrt(xl * xl + yl * yl + zl * zl);
		
		// Calculate Azimuth
		retVal[1] = Math.atan2((Math.sin(targetLon - observerLon) * Math.cos(targetLat)), (Math.cos(observerLat) * Math.sin(targetLat) - Math.sin(observerLat) * Math.cos(targetLat) * Math.cos(targetLon - observerLon))) * 180.0 / Math.PI;
		
		// Calculate Elevation
		retVal[2] = zl / Math.sqrt(xl * xl + yl * yl) * 180.0 / Math.PI;
		
		double startLat = observerLat;
		double endLat = targetLat;
		double startLon = observerLon;
		double endLon = targetLon;

		if(startLat > targetLat) {
			startLat = targetLat;
			endLat = observerLat;
			startLon = targetLon;
			endLon = observerLon;
		}
		
		startLat *= 180.0 / Math.PI;
		endLat *= 180.0 / Math.PI;
		startLon *= 180.0 / Math.PI;
		endLon *= 180.0 / Math.PI;
		int altCount = (int)((endLat - startLat) / 0.01);
		
		double buffAlt = observerAlt;
		if(buffAlt > targetAlt)
			buffAlt = targetAlt;
				
		double slope = (endLon - startLon) / (endLat - startLat);
		while(startLat < endLat) {
			double newLon = slope * startLat - slope * observerLat * 180.0 / Math.PI+ observerLon * 180.0 / Math.PI;
			try {
				if(serverTerrain.getElevation(startLat, newLon) > buffAlt)
					retVal[3] = 1;
				buffAlt += Math.abs(observerAlt - targetAlt) / altCount;
			}
			catch (Exception e) {
				e.printStackTrace();
			}
			startLat += 0.01;
		}

		if (retVal[3] == 0) {
			if(retVal[2] < Math.asin(R / ((R + observerAlt)) * 180.0 / Math.PI))
				retVal[3] = 2;
		}
		
		return retVal;
	}
	
	/**
	 * Sets Latitude/Longitude navigation errors for aircraft CNS.
	 * 
	 */
	public int setNavigationLocationError(int sessionId, String aircraftId, String parameter, double bias, double drift, double scaleFactor, double noiseVariance, int scope) {
		int retVal = 0;
		
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return 1;
		}
		
		Aircraft aircraft;
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		if (parameter.equals("LATITUDE")) {
			aircraft.setLatitude_deg((float)bias + (float)drift * cEngine.get_curr_sim_time() + (float)scaleFactor * aircraft.getLatitude_deg() + (float)Math.sqrt(noiseVariance));
		}
		else if (parameter.equals("LONGITUDE")) {
			aircraft.setLongitude_deg((float)bias + (float)drift * cEngine.get_curr_sim_time() + (float)scaleFactor * aircraft.getLongitude_deg() + (float)Math.sqrt(noiseVariance));
		}
		else {
			retVal = 1;
		}
		
		return retVal;
	}

	/**
	 * Sets altitude navigation errors for aircraft CNS.
	 */
	public int setNavigationAltitudeError(int sessionId, String aircraftId, double bias, double noiseVariance, int scope) {
		int retVal = 0;
		
		String[] aircraftIds;
		aircraftIds = cEngine.getAllAircraftId();
		boolean validAircraft = contains(aircraftIds, aircraftId);
		
		if(!validAircraft) {
			System.out.println(aircraftId + " is an Invalid Flight.");
			return 1;
		}
		
		Aircraft aircraft;
		aircraft = cEngine.select_aircraft(sessionId, aircraftId);
		
		aircraft.setAltitude_ft((float)bias + aircraft.getAltitude_ft() + (float)Math.sqrt(noiseVariance));
		
		return 0;
	}

	/**
	 * Sets altitude navigation errors for ground radar at given airport.
	 */
	public int setRadarError(String airportId, String parameter, double originalValue, double bias, double noiseVariance, int scope) {
		return cEngine.setRadarError(airportId, parameter, originalValue, bias, noiseVariance, scope);
	}
}
