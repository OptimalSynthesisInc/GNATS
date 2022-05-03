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

package com.osi.gnats.api.equipment;

import java.rmi.RemoteException;

import com.osi.gnats.aircraft.Aircraft;
import com.osi.gnats.api.BaseInterface;

public interface CNSInterface extends BaseInterface {
	
	/**
	 * Computes line of sight between source and target, returns range, azimuth, and elevation along with potential terrain or earth curvature masking.
	 * @param observerLat Latitude of observer's position.
	 * @param observerLon Longitude of observer's position.
	 * @param observerAlt Altitude of observer's position.
	 * @param targetLat Latitude of target's position.
	 * @param targetLon Longitude of target's position.
	 * @param targetAlt Altitude of target's position.
	 * @return Array as (Range (ft), Azimuth (degree), Elevation(degree), Masking (boolean)) of observer with respect to target.
	 * The Masking boolean can assume following values:
	 * 0: No Masking.
	 * 1: Terrain Masking.
	 * 2: Masking due to curvature of Earth.
	 */
	public double[] getLineOfSight(double observerLat, double observerLon, double observerAlt, double targetLat, double targetLon, double targetAlt) throws RemoteException;
	
	/**
	 * Sets Latitude/Longitude navigation errors for aircraft CNS.
	 * 
	 * @param parameter String containing LATITUDE or LONGITUDE.
	 * @param bias Bias to be applied to original value.
	 * @param drift Drift to be applied to original value over flight time. 
	 * @param scaleFactor scale factor error that would lead to erroneous instrument values.
	 * @param noiseVariance Variance of noise to be applied, assuming normal distribution with zero mean.
	 * @param scope 0 for errors to reflect on flight deck systems only, 1 to include ADS-B transmission.
	 * @return
	 * @throws RemoteException
	 */
	public int setNavigationLocationError(String aircraftId, String parameter, double bias, double drift, double scaleFactor, double noiseVariance, int scope) throws RemoteException;

	/**
	 * Sets altitude navigation errors for aircraft CNS.
	 * 
	 * @param bias Bias to be applied to original value.
	 * @param noiseVariance Variance of noise to be applied, assuming normal distribution with zero mean.
	 * @param scope 0 for errors to reflect on flight deck systems only, 1 to include ADS-B transmission.
	 * @return
	 * @throws RemoteException
	 */
	public int setNavigationAltitudeError(String aircraftId, double bias, double noiseVariance, int scope) throws RemoteException;

	/**
	 * Sets range, elevation, or azimuth error to ground radar at an airport.
	 * 
	 * @param airportId ICAO code of airport
	 * @param parameter String containing RANGE, ELEVATION, or AZIMUTH
	 * @param originalValue The initial true value of the parameter
	 * @param bias Bias to be applied to original value.
	 * @param noiseVariance Variance of noise to be applied, assuming normal distribution with zero mean.
	 * @param scope 0 for errors to reflect on ground systems only, 1 to include transmission to aircraft.
	 * @return
	 * @throws RemoteException
	 */
	public int setRadarError(String airportId, String parameter, double originalValue, double bias, double noiseVariance, int scope) throws RemoteException;

}
