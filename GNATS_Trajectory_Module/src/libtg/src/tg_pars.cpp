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
 * tg_pars.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: jason
 */

#include "tg_pars.h"
#include "NatsPar.h"
#include "NatsDataLoader.h"

#include <string>
#include <vector>
#include <vector>
#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <dirent.h>

using namespace std;

/*
 * host-side global variables
 */
vector<NatsPar> g_pars;

static int get_par_list(const string& data_dir, vector<string>* const files) {
	// open the par data directory and read the files in that directory.
	DIR* dir;
	struct dirent* ent;
	if( (dir=opendir(data_dir.c_str())) != NULL) {
		while((ent = readdir(dir)) != NULL) {
			// ignore the directories (".." and ".")
			string dname = ent->d_name;
			if(dname != ".." && dname != ".") {
				files->push_back(string(ent->d_name));
			}
		}
		closedir(dir);
	} else {
		printf("Error opening PAR directory\n");
		return -1;
	}
	return 0;
}

int load_pars(const string& data_dir) {

	printf("  Loading PAR data\n");

	string par_dir = data_dir + "/PAR";
	// get the list of all par files in the PAR directory
	vector<string> par_files;
	int err = get_par_list(par_dir, &par_files);
	if(err < 0) {
		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}
	g_pars.clear();

	// load each file
	NatsDataLoader loader;
	vector<NatsPar>::iterator iter;
	string fname;
	for(unsigned int i=0; i<par_files.size(); ++i) {
		fname = par_dir + "/" + par_files.at(i);
		err = loader.loadPars(fname, &g_pars);
	}

	// Detach the current thread
	pthread_detach(pthread_self());

	return 0;
}

