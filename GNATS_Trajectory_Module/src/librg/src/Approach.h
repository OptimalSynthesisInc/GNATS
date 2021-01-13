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
 * Approach.h
 *
 *  Created on: May 19, 2017
 *      Author: pdutta
 */

#ifndef SRC_LIBRG_SRC_APPROACH_H_
#define SRC_LIBRG_SRC_APPROACH_H_

#include "ProcKey.h"

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;

namespace osi {

class Approach;

extern map<ProcKey, Approach*> rg_approach;

class Approach{
public:
	Approach();
	Approach(const string& name,
	     const string& airport,const string& runway,
	     const map< string, vector<string> >& waypoints,
		 const map<string,pair<string,string > >& rt_to_trans_rttype);
	Approach(const Approach& that);
	virtual ~Approach();

	Approach& operator=(const Approach& that);

	void getEntryPoints(vector<string>* const entryNames);

private:
	void buildConnectivity();
	void destroyConnectivity();

public:
	string name;
	string airport;
	string runway;
	map< string, vector<string> > waypoints;
	map<string,pair<string,string > > rt_to_trans_rttype;
	map<string, vector<pair<string,string> > > path_term; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,PATH AND TERMINATION) )
	map<string, vector<pair<string,string> > > alt_desc; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,ALTITUDE CONSTRAINT DESCRIPTION) )
	map<string, vector<pair<string,double> > > alt_1; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,LOW ALT CONSTRAINT) )
	map<string, vector<pair<string,double> > > alt_2; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,HIGH ALT CONSTRAINT) )
	map<string, vector<pair<string,double> > > spd_limit; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,SPEED LIM CONSTRAINT) )
	//ADDITIONAL VARIABLES
	map<string, vector<pair<string,string> > > recco_nav; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,RECOMMENDED NAVAID) ) SEE SECTION 5.23 IN ARINC 424
	map<string, vector<pair<string,double> > > theta; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,THETA ) ) SEE SECTION 5.24 IN ARINC 424
	map<string, vector<pair<string,double> > > rho; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,RHO ) ) SEE SECTION 5.25 IN ARINC 424
	map<string, vector<pair<string,double> > > mag_course; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,MAGNETIC COURSE AT WPT) ) SEE SECTION 5.26 IN ARINC 424
	map<string, vector<pair<string,double> > > rt_dist; //ROUTE_IN_APPROACH--> VECTOR( PAIR(WAYPOINT,ROUTE DISTANCE ) ) SEE SECTION 5.27 IN ARINC 424

private:
	map<string, int> connectivityIndex;
	map<int, string> connectivityNames;
	size_t connectivitySize;
	int** connectivity;
};

} /* namespace osi */

#endif /* SRC_LIBRG_SRC_APPROACH_H_ */
