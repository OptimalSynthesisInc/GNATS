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
 * Fix.h
 *
 *  Created on: May 10, 2012
 *      Author: jason
 */

#ifndef FIX_H_
#define FIX_H_

#include <string>
#include <map>

using std::map;
using std::string;

namespace osi {

class Fix;

// global hashmap of fixes for looking up a Fix instance by name
extern map<string, Fix*> rg_fixes;
extern map<int, Fix*> rg_fixes_by_id;
extern map<string, int> rg_fix_ids;
extern map<int, string> rg_fix_names;

/**
 * Interface for Navaid or Waypoint
 */
class Fix {
public:
	Fix();
	virtual ~Fix();

	virtual string& getName() = 0;
	virtual double& getLatitude() = 0;
	virtual double& getLongitude() = 0;
};

class Navaid : public Fix {
public:
	Navaid();
	Navaid(const string& name,
		   const string& description,
		   const double& latitude,
		   const double& longitude,
		   const double& frequency,
		   const double& dmeLatitude,
		   const double& dmeLongitude,
		   const double& dmeElevation);

	Navaid(const Navaid& that);

	virtual ~Navaid();

	Navaid& operator=(const Navaid& that);

	// TODO: making members public for convenience.
	// should make them private and provide getters and setters.
	string name;
	string description;
	double latitude;
	double longitude;
	double frequency;
	double dmeLatitude;
	double dmeLongitude;
	double dmeElevation;

	string& getName();
	double& getLatitude();
	double& getLongitude();
};

class Waypoint : public Fix {
public:
	Waypoint();
	Waypoint(const string& name,
			 const string& description,
			 const double& latitude,
			 const double& longitude);

	Waypoint(const Waypoint& that);

	virtual ~Waypoint();

	Waypoint& operator=(const Waypoint& that);

	// TODO: making members public for convenience.
	// should make them private and provide getters and setters.
	string name;
	string description;
	double latitude;
	double longitude;

	string& getName();
	double& getLatitude();
	double& getLongitude();
};

}

#endif /* FIX_H_ */
