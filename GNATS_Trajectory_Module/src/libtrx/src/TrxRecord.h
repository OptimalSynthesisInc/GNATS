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

/*
 * TrxRecord.h
 *
 *  Created on: May 4, 2012
 *      Author: jason
 */

#ifndef TRXRECORD_H_
#define TRXRECORD_H_

#include "TrxRoute.h"

#include <string>

using std::string;

namespace osi {

class TrxRecord {
public:
	TrxRecord();
	TrxRecord(const long& timestamp,
		  const string& acid,
		  const string& actype,
		  const double& latitude,
		  const double& longitude,
		  const double& tas,
		  const double& altitude,
		  const double& heading,
		  const string& center,
		  const string& sector,
		  const string& route,
		  const double& cruiseAltitude);
    TrxRecord(const long& timestamp,
          const string& acid,
          const string& actype,
          const double& latitude,
          const double& longitude,
          const double& tas,
          const double& altitude,
          const double& heading,
          const string& center,
          const string& sector,
          const string& route,
          const string& trx_str,
          const double& cruiseAltitude);
	TrxRecord(const TrxRecord& that);
	virtual ~TrxRecord();

	TrxRecord& operator=(const TrxRecord& that);
	bool operator==(const TrxRecord& that) const;
	int operator<(const TrxRecord& that) const;

    const string& getAcid() const;
    const string& getActype() const;
    const double& getAltitude() const;
    const string& getCenter() const;
    const double& getHeading() const;
    const double& getLatitude() const;
    const double& getLongitude() const;
    const string& getRoute_str() const;
    const string& getSector() const;
    const double& getTas() const;
    const long& getTimestamp() const;
    const TrxRoute& getRoute() const;
    const double& getCruiseAltitude() const;
    const string& getTrx_str() const;

    void setAcid(string acid);
    void setActype(string actype);
    void setAltitude(double altitude);
    void setCenter(string center);
    void setHeading(double heading);
    void setLatitude(double latitude);
    void setLongitude(double longitude);
    void setRoute_str(string route);
    void setSector(string sector);
    void setTas(double tas);
    void setTimestamp(long  timestamp);
    void setRoute(TrxRoute& route);
    void setCruiseAltitude(double cruiseAltitude);
    void setTrx_str(string trx_str);

public:
	long timestamp;
	string acid;
	string actype;
	double latitude;
	double longitude;
	double tas;
	double altitude;
	double heading;
	string center;
	string sector;
	string route_str;
    string trx_str;
	double cruiseAltitude;
	TrxRoute route;
	bool   flag_geoStyle;

private:
	void parseRouteString();
};

}

#endif /* TRXRECORD_H_ */
