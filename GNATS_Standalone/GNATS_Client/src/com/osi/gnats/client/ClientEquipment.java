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

import java.rmi.Remote;
import java.rmi.RemoteException;

import com.osi.gnats.api.EquipmentInterface;
import com.osi.gnats.api.NATSInterface;
import com.osi.gnats.api.equipment.AircraftInterface;
import com.osi.gnats.api.equipment.GroundVehicleInterface;
import com.osi.gnats.api.equipment.ADBDataInterface;
import com.osi.gnats.api.equipment.CNSInterface;
import com.osi.gnats.rmi.RemoteEquipment;

public class ClientEquipment extends BaseClass implements EquipmentInterface {
	RemoteEquipment remoteEquipment;

	public ClientEquipment(NATSInterface natsInterface, RemoteEquipment re) throws RemoteException {
		super(natsInterface, "Equipment", "Equipment Interface", "Equipment Functions");
		this.remoteEquipment = re;
	}	
    
	public ClientEquipment(NATSInterface natsInterface, Remote re) throws RemoteException {
		super(natsInterface, "Equipment", "Equipment Interface", "Equipment Functions");
		this.remoteEquipment = (RemoteEquipment)re;
	}
	
	public AircraftInterface getAircraftInterface() {
		return (AircraftInterface)interfacesMap.get("Aircraft");
	}
	
	public ADBDataInterface getADBDataInterface() {
		return (ADBDataInterface)interfacesMap.get("ADBData");
	}
	
	public GroundVehicleInterface getGroundVehicleInterface() {
		return (GroundVehicleInterface)interfacesMap.get("GroundVehicle");
	}
	
	public CNSInterface getCNSInterface() {
		return (CNSInterface)interfacesMap.get("CNS");
	}
}
