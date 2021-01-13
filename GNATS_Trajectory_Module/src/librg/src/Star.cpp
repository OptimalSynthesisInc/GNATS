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
 * Star.cpp
 *
 *  Created on: Oct 4, 2012
 *      Author: jason
 */

#include "Star.h"

#include "ProcKey.h"

#include <string>
#include <vector>
#include <map>

#include <cstdlib>

using std::string;
using std::vector;
using std::map;

namespace osi {

// global stars map declared extern in Star.h
map<ProcKey, Star*> rg_stars;

Star::Star() :
	name(""),
	airport(""),
	waypoints(map< string, vector<string> >()),
	rt_to_trans_rttype(map< string, pair<string,string > >()),
	path_term(map< string, vector<pair<string,string> > >()),
	alt_desc(map<string, vector<pair<string,string> > >()),
	alt_1(map<string, vector<pair<string,double> > >()),
	alt_2(map<string, vector<pair<string,double> > > ()),
	spd_limit(map<string, vector<pair<string,double> > >()),
	recco_nav(map<string, vector<pair<string,string> > >()),
	theta(map<string, vector<pair<string,double> > >()),
	rho(map<string, vector<pair<string,double> > >()),
	mag_course(map<string, vector<pair<string,double> > >()),
	rt_dist(map<string, vector<pair<string,double> > >()),
	connectivityIndex(map<string, int>()),
	connectivityNames(map<int, string>()),
	connectivitySize(0),
	connectivity(NULL) {
}

Star::Star(const string& name, const string& airport, const map< string, vector<string> >& waypoints,
		const map<string,pair<string,string > >& rt_to_trans_rttype) :
	name(name),
	airport(airport),
	waypoints(waypoints),
	rt_to_trans_rttype(rt_to_trans_rttype),
	path_term(map< string, vector<pair<string,string> > >()),
	alt_desc(map<string, vector<pair<string,string> > >()),
	alt_1(map<string, vector<pair<string,double> > >()),
	alt_2(map<string, vector<pair<string,double> > > ()),
	spd_limit(map<string, vector<pair<string,double> > >()),
	recco_nav(map<string, vector<pair<string,string> > >()),
	theta(map<string, vector<pair<string,double> > >()),
	rho(map<string, vector<pair<string,double> > >()),
	mag_course(map<string, vector<pair<string,double> > >()),
	rt_dist(map<string, vector<pair<string,double> > >()),
	connectivityIndex(map<string, int>()),
	connectivityNames(map<int, string>()),
	connectivitySize(0),
	connectivity(NULL) {

	buildConnectivity();
}

Star::Star(const Star& that) :
	name(that.name),
	airport(that.airport),
	waypoints(that.waypoints),
	rt_to_trans_rttype(that.rt_to_trans_rttype),
	path_term(map< string, vector<pair<string,string> > >(that.path_term)),
	alt_desc(map<string, vector<pair<string,string> > >(that.alt_desc)),
	alt_1(map<string, vector<pair<string,double> > >(that.alt_1)),
	alt_2(map<string, vector<pair<string,double> > >(that.alt_2)),
	spd_limit(map<string, vector<pair<string,double> > >(that.spd_limit)),
	recco_nav(map<string, vector<pair<string,string> > >(that.recco_nav)),
	theta(map<string, vector<pair<string,double> > >(that.theta)),
	rho(map<string, vector<pair<string,double> > >(that.rho)),
	mag_course(map<string, vector<pair<string,double> > >(that.mag_course)),
	rt_dist(map<string, vector<pair<string,double> > >(that.rt_dist)),
    connectivityIndex(map<string, int>()),
    connectivityNames(map<int, string>()),
    connectivitySize(0),
    connectivity(NULL) {

	buildConnectivity();
}

Star::~Star() {
	destroyConnectivity();
}

Star& Star::operator=(const Star& that) {
	if(this == &that) return *this;
	this->name = that.name;
	this->airport = that.airport;
	this->waypoints.clear();
	this->waypoints.insert(that.waypoints.begin(), that.waypoints.end());
	this->rt_to_trans_rttype.clear();
	this->rt_to_trans_rttype.insert(that.rt_to_trans_rttype.begin(), that.rt_to_trans_rttype.end());
	this->path_term.insert(that.path_term.begin(),that.path_term.end());
	this->alt_desc.insert(that.alt_desc.begin(),that.alt_desc.end());
	this->alt_1.insert(that.alt_1.begin(),that.alt_1.end());
	this->alt_2.insert(that.alt_2.begin(),that.alt_2.end());
	this->spd_limit.insert(that.spd_limit.begin(),that.spd_limit.end());
	this->recco_nav.insert(that.recco_nav.begin(),that.recco_nav.end());
	this->theta.insert(that.theta.begin(),that.theta.end());
	this->rho.insert(that.rho.begin(),that.rho.end());
	this->mag_course.insert(that.mag_course.begin(),that.mag_course.end());
	this->rt_dist.insert(that.rt_dist.begin(),that.rt_dist.end());

	buildConnectivity();

	return *this;
}

void Star::getEntryPoints(vector<string>* const entryNames) {
	if(!connectivity) {
		buildConnectivity();
	}
	// find procedure start points.  these are points with no
	// inputs, so the column sum of the connectivity matrix
	// is zero for an entry point.
	for(size_t j=0; j<connectivitySize; ++j) {
		int colsum = 0;
		for(size_t i=0; i<connectivitySize; ++i) {
			colsum += connectivity[i][j];
		}
		if(colsum == 0) {
			string entryname = connectivityNames.at(j);
			entryNames->push_back(entryname);
		}
	}
}

void Star::getExitPoints(vector<string>* const exitNames) {
	if(!connectivity) {
		buildConnectivity();
	}
	// find procedure end points.  these are points with no
	// outputs, so the row sum of the connectivity matrix
	// is zero for an entry point.
	for(size_t i=0; i<connectivitySize; ++i) {
		int rowsum = 0;
		for(size_t j=0; j<connectivitySize; ++j) {
			rowsum += connectivity[i][j];
		}
		if(rowsum == 0) {
			string exitname = connectivityNames.at(i);
			if (exitname == this->airport){
				for (size_t j=0; j < connectivitySize; ++j){
					if ( connectivity[j][i] == 1){
						exitname = connectivityNames.at(j);
						break;
					}
				}
			}
			exitNames->push_back(exitname);
		}
	}
}


void Star::destroyConnectivity() {
	if(connectivity) {
		// free connectivity matrix memory, clear the names vector,
		// and reset the size to 0.
		for(size_t i=0; i<connectivitySize; ++i) {
			free(connectivity[i]);
		}
		free(connectivity);
		connectivityIndex.clear();
		connectivitySize = 0;
	}
}

void Star::buildConnectivity() {
	destroyConnectivity();

	// form the unique waypoint names index
	map< string, vector<string> >::iterator iter;
	for(iter=waypoints.begin(); iter!=waypoints.end(); ++iter) {
		const vector<string>* wpvec = &(iter->second);
		for(unsigned int i=0; i<wpvec->size(); ++i) {
			string name = wpvec->at(i);
			map<string, int>::iterator found = connectivityIndex.find(name);
			if(found == connectivityIndex.end()) {
				int index = connectivityIndex.size();
				connectivityIndex.insert(std::pair<string, int>(name, index));
				connectivityNames.insert(std::pair<int, string>(index, name));
			}
		}
	}

	// allocate connectivity
	connectivitySize = connectivityIndex.size();
	connectivity = (int**)calloc(connectivitySize, sizeof(int*));
	for(size_t i=0; i<connectivitySize; ++i) {
		connectivity[i] = (int*)calloc(connectivitySize, sizeof(int));
	}

	// iterate over each procedure segment and add connections
	// between waypoints.
	for(iter=waypoints.begin(); iter!=waypoints.end(); ++iter) {
		const vector<string>* wpvec = &(iter->second);
		for(unsigned int i=1; i<wpvec->size(); ++i) {
			string fromName = wpvec->at(i-1);
			string toName = wpvec->at(i);
			int fromIndex = connectivityIndex.at(fromName);
			int toIndex = connectivityIndex.at(toName);
			connectivity[fromIndex][toIndex] = 1;
		}
	}
}

}
