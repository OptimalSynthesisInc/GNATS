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
 * tg_sidstars.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: jason
 */

#include "tg_sidstars.h"
#include "NatsSid.h"
#include "NatsStar.h"
#include "NatsApproach.h"
#include "NatsDataLoader.h"
#include "geometry_utils.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <algorithm>    // std::find
#include <limits>       // std::numeric_limits


using namespace std;
using osi::compute_distance_gc;

/*
 * host-side global variables
 */
vector<NatsSid> g_sids;
vector<NatsStar> g_stars;
vector<NatsApproach> g_approaches;
STARAPPmap g_star_app_map;

bool flag_SidStarApp_available = false;

int load_sidstars(const string& data_dir, const string& cifp_file) {

	printf("  Loading SID/STAR data\n");
	g_sids.clear();
	g_stars.clear();
	g_approaches.clear();

	string fname;

#if (!USE_CIFP)
	fname.assign(data_dir);
	fname.append("/Sids_Stars.crypt");
#else
	if (cifp_file != "") {
	    fname.assign(cifp_file);
	}
#endif

	NatsDataLoader loader;
	int err = loader.loadSidStars(fname, &g_sids, &g_stars, &g_approaches);
	if (err <= 0) {
		printf("Error loading SIDs/STARs\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	flag_SidStarApp_available = true;

	// Detach the current thread
	pthread_detach(pthread_self());

	return 0;
}
