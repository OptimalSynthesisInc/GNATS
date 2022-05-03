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

/*
 * LookupGrid.cpp
 *
 *  Created on: Oct 5, 2012
 *      Author: jason
 */

#include "LookupGrid.h"

#include "Fix.h"

#include <vector>
#include <map>

using std::vector;
using std::map;

namespace osi {

GridKey::GridKey(int i, int j) :
	i(i),
	j(j) {
}

GridKey::GridKey(const GridKey& that) :
	i(that.i),
	j(that.j) {
}

GridKey::~GridKey() {
}

bool GridKey::operator<(const GridKey& that) const {
	if(this == &that) return false;
	if(this->i > that.i) return false;
	else if(this->i == that.i) {
		if(this->j >= that.j) return false;
		else return true;
	} else {
		return true;
	}
}

bool GridKey::operator==(const GridKey& that) const {
	if(this == &that) return true;
	return (this->i == that.i && this->j == that.j);
}


LookupGrid::LookupGrid(const map<string, Fix*>& fixes) :
	grid(map< GridKey, vector<Fix*> >()) {

	// build the lookup grid
	map<string, Fix*>::const_iterator fiter;
	for(fiter=fixes.begin(); fiter!=fixes.end(); ++fiter) {
		Fix* fix = fiter->second;
		GridKey key = getKey(fix->getLatitude(), fix->getLongitude());

		getFixes(key).push_back(fix);
	}
}

LookupGrid::~LookupGrid() {

}

GridKey LookupGrid::getKey(const double& latitude, const double& longitude) {
	double latCells = (GRID_LAT_MAX-GRID_LAT_MIN) / GRID_CELL_SIZE;
	double lonCells = (GRID_LON_MAX-GRID_LON_MIN) / GRID_CELL_SIZE;
	double j = latCells * (latitude-GRID_LAT_MIN) / (GRID_LAT_MAX-GRID_LAT_MIN);
	double i = lonCells * (longitude-GRID_LON_MIN) / (GRID_LON_MAX-GRID_LON_MIN);
	return GridKey(i,j);
}

vector<Fix*>& LookupGrid::getFixes(const GridKey& key) {
	map< GridKey, vector<Fix*> >::iterator giter;
	giter = grid.find(key);
	if(giter == grid.end()) {
		grid.insert(std::pair< GridKey, vector<Fix*> >(key, vector<Fix*>()));
	}
	return grid.at(key);
}

vector<Fix*>& LookupGrid::getFixes(const double& latitude, const double& longitude) {
	return getFixes(getKey(latitude, longitude));
}

}
