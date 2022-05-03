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

#ifndef TRXRECORD_GROUNDVEHICLE_H_
#define TRXRECORD_GROUNDVEHICLE_H_

#include "TrxRoute.h"

#include <string>

using std::string;

namespace osi {

class TrxRecord_GroundVehicle {
public:
	TrxRecord_GroundVehicle();

    TrxRecord_GroundVehicle(const long& timestamp,
          const string& vehicle_id,
		  const string& aircraft_id,
		  const string& airport_id,
          const double& latitude,
          const double& longitude,
          const double& speed,
          const double& altitude,
          const double& course,
          const string& route_str,
          const string& trx_str);

	TrxRecord_GroundVehicle(const TrxRecord_GroundVehicle& that);

	virtual ~TrxRecord_GroundVehicle();

	TrxRecord_GroundVehicle& operator=(const TrxRecord_GroundVehicle& that);

	bool operator==(const TrxRecord_GroundVehicle& that) const;

	int operator<(const TrxRecord_GroundVehicle& that) const;

    const string& getVehicle_id() const;
    const string& getAircraft_id() const;
    const string& getAirport_id() const;
    const double& getAltitude() const;
    const double& getCourse() const;
    const double& getLatitude() const;
    const double& getLongitude() const;
    const string& getRoute_str() const;
    const double& getSpeed() const;
    const long& getTimestamp() const;
    const string& getTrx_str() const;

    void setVehicle_id(string vehicle_id);
    void setAircraft_id(string aircraft_id);
    void setAirport_id(string airport_id);
    void setAltitude(double altitude);
    void setCourse(double course);
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    void setRoute_str(string route);
    void setSpeed(double speed);
    void setTimestamp(long  timestamp);
    void setTrx_str(string trx_str);

public:
	long timestamp;
	string vehicle_id;
	string aircraft_id;
	string airport_id;
	double latitude;
	double longitude;
	double speed;
	double altitude;
	double course;
	string route_str;
    string trx_str;

};

}

#endif
