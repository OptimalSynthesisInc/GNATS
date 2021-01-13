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
 * tg_adb.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#include "tg_adb.h"

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "adb.h"
#include "AdbOPFParser.h"
#include "AdbPTFParser.h"
#include "AdbSynonymsParser.h"

using namespace std;
using namespace osi;


// host only globals
map<string, string> g_adb_synonyms;
map<string, int> g_adb_indices;
set<string> g_adb_model_names;
vector<AdbOPFModel> g_adb_opf_models;
vector<AdbPTFModel> g_adb_ptf_models;

bool flag_adb_available = false;

int adb_ptf_total_row_num = 0;
int adb_ptf_type_num = 0;
int adb_ptf_lower_bound_num = 0;

short* h_adb_num_rows;
short* h_adb_table_start_index;
short* h_adb_lower_bound_row;
short* h_adb_fl;
short* h_adb_vtas_climb_knots;
short* h_adb_vtas_descent_knots;
short* h_adb_roc_fpm;
short* h_adb_rod_fpm;

int read_adb_synonym_file(const string& adb_dir, map<string, string>& synonyms) {
	string fname = adb_dir + "/SYNONYM.NEW";
	AdbSynonymsParser parser(synonyms, fname);
	int err = parser.parse();
	return err;
}

// Read data of ADB performance files
int read_adb_performance_files(const string& adb_ptf_dir,
		                               const set<string>& model_names,
		                               adb_ptf_column_e& col) {
	adb_mass_e mass;
	if(col == HI) {
		mass = HIGH;
	} else if(col == LO) {
		mass = LOW;
	} else {
		mass = NOMINAL;
	}

	if (flag_adb_available) {
		set<string>::iterator it;

		// Looping over all aircraft modeled in Adb to build host tables
		int model_index = 0;
		for(it=model_names.begin(); it!=model_names.end(); it++) {
			stringstream ss;
			ss << adb_ptf_dir << "/" << (*it) << ".PTF";

			string model_name = it->substr(0, it->find_first_of('_'));

			// parse the PTF model file
			AdbPTFModel model;
			AdbPTFParser parser(model, ss.str());
			if (parser.parse() == 0) {
				g_adb_ptf_models.push_back(model);

				adb_ptf_total_row_num += model.getNumRows();

				g_adb_indices[model_name] = model_index;
				model_index++;
			}

		}
	}

	adb_ptf_type_num = g_adb_ptf_models.size();

	adb_ptf_lower_bound_num = adb_ptf_type_num * adb_ptf_total_row_num;

	// Allocate memory space
	h_adb_num_rows = (short*)calloc(adb_ptf_type_num, sizeof(short));
	h_adb_table_start_index = (short*)calloc(adb_ptf_type_num, sizeof(short));
	h_adb_lower_bound_row = (short*)calloc(adb_ptf_lower_bound_num, sizeof(short));
	h_adb_fl = (short*)calloc(adb_ptf_total_row_num, sizeof(short));
	h_adb_vtas_climb_knots = (short*)calloc(adb_ptf_total_row_num, sizeof(short));
	h_adb_vtas_descent_knots = (short*)calloc(adb_ptf_total_row_num, sizeof(short));
	h_adb_roc_fpm = (short*)calloc(adb_ptf_total_row_num, sizeof(short));
	h_adb_rod_fpm = (short*)calloc(adb_ptf_total_row_num, sizeof(short));

	for (int i = 0; i < adb_ptf_type_num; i++) {
		h_adb_num_rows[i] = 0;
	}

    // init the lower bounds array to -1
    for (int i=0; i<adb_ptf_lower_bound_num; ++i) {
    	h_adb_lower_bound_row[i] = -1;
    }

    int index = 0; // index into fl, vtas_climb, vtas_descent, roc, rod arrays
    int jndex = 0; // index into lower_bound row array

    for (unsigned int i=0; i<g_adb_ptf_models.size(); i++) {
    	AdbPTFModel* model = &(g_adb_ptf_models.at(i));

    	// set the starting index for this model
    	h_adb_table_start_index[i] = index;

    	double fl_max = model->getAltitude(model->getNumRows()-1)/100.;
    	short num_rows = (short)model->getNumRows();
    	short num_equispaced_rows = ceil(fl_max / (double)ADB_PTF_ROW_SPACING);
    	h_adb_num_rows[i] = num_rows;

    	// append the table data to the host arrays
    	for(int j=0; j<num_rows; ++j) {
    		double alt = model->getAltitude(j);
    		h_adb_fl[index] = (short)(alt/100.);
    		h_adb_vtas_climb_knots[index] = (short)model->getClimbTas(alt);
    		h_adb_vtas_descent_knots[index] = (short)model->getDescentTas(alt);
    		h_adb_roc_fpm[index] = (short)model->getClimbRate(alt, mass);
    		h_adb_rod_fpm[index] = (short)model->getDescentRate(alt, mass);

    		++index;
    	}

    	// build the lower bound row arrays.  the value at jndex is the
    	// index in the table arrays.  a value of -1 indicates that
    	// the altitude is beyond the max altitude for the ac type.
    	double alt = 0;
    	short fl = 0;
    	int lb_index = 0;
    	vector<double>::const_iterator lb_iter;
    	for(int j=0; j<ADB_MAX_ROWS; ++j) {
    		if(j <= num_equispaced_rows) {
				alt = fl*100.;
				lb_iter = lower_bound(model->altitudes.begin(),
									  model->altitudes.end(), alt);
				lb_index = lb_iter - model->altitudes.begin();
				h_adb_lower_bound_row[jndex] = lb_index;

				fl += ADB_PTF_ROW_SPACING;
    		} else {
    			h_adb_lower_bound_row[jndex] = -1;
    		}

    		++jndex;
    	}
    }

	return 0;
}

// Read ADB performance operational files
int read_adb_performance_operational_files(const string& adb_opf_dir,
		                               const set<string>& model_names) {
    set<string>::iterator it;

    // Looping over all aircraft modeled in ADB to build host tables
    for (it = model_names.begin(); it != model_names.end(); it++) {
    	stringstream ss;
    	ss << adb_opf_dir << "/" << (*it) << ".OPF";

    	// Parse the OPF model file
    	AdbOPFModel model;
    	AdbOPFParser parser(model, ss.str());

    	if (parser.parse() != -1) {
    		g_adb_opf_models.push_back(model);
    	}
    }

	return 0;
}

static void debug_adb_performance_operational_files() {
	map<string, int>::iterator ite_adb_index;
	for (ite_adb_index = g_adb_indices.begin(); ite_adb_index != g_adb_indices.end(); ite_adb_index++) {
		printf("Adb %s index %d\n", ite_adb_index->first.c_str(), ite_adb_index->second);
	}

	vector<AdbOPFModel>::iterator ite;
	for (ite = g_adb_opf_models.begin(); ite != g_adb_opf_models.end(); ite++) {
		AdbOPFModel tmpAdbOPFModel = *ite;

		printf("\nactype = %s\n", tmpAdbOPFModel.type.c_str());

		map<adb_flight_phase_e, double>::iterator ite_vstall;
		for (ite_vstall = tmpAdbOPFModel.vstall.begin(); ite_vstall != tmpAdbOPFModel.vstall.end(); ite_vstall++) {
			printf("	adb_flight_phase = %d, vstall = %f\n", ite_vstall->first, ite_vstall->second);
		}
	}
}

// Main function to load ADB data
int load_adb_performance_tables(const string& adb_ptf_dir,
		                         adb_ptf_column_e col) {
	int err;

	printf("  Loading ADB data\n");

	// load the synonyms
	err = read_adb_synonym_file(adb_ptf_dir, g_adb_synonyms);
	if (err) {
		flag_adb_available = false;
	} else {
		flag_adb_available = true;
	}

	// ADB file not available
	// Manually insert data of B737 and B773
	if (!flag_adb_available) {
		printf("      ADB data doesn't exist.\n");
		return -1;
	}

	// find the unique models by placing the synonym values into a
	// set.  sets allow only unique values.
	map<string, string>::iterator syn_iter;
	for (syn_iter = g_adb_synonyms.begin(); syn_iter != g_adb_synonyms.end(); ++syn_iter) {
		g_adb_model_names.insert(syn_iter->second);
	}

	err = read_adb_performance_files(adb_ptf_dir, g_adb_model_names, col);
	if ((flag_adb_available) && (err < 0)) {
		printf("Error reading ADB performance files\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return -3;
	}

	// =====================================================

	if (flag_adb_available) {
		err = read_adb_performance_operational_files(adb_ptf_dir, g_adb_model_names);
		if (err) {
			printf("Error reading ADB performance operational files\n");

			// Detach the current thread
			pthread_detach(pthread_self());

			return -3;
		}
	}

	//debug_adb_performance_operational_files();

	// Detach the current thread
	pthread_detach(pthread_self());

	return 0;
}

void* get_adb_performance_data() {
	return NULL;
}

string get_adb_synonym(const string& actype, const string& default_type) {
	map<string, string>::iterator it;
	it = g_adb_synonyms.find(actype);
	if (it == g_adb_synonyms.end()) {
		if (flag_adb_available) {
			return default_type;
		} else {
			return "N/A";
		}
	}
	// remove the trailing "_" characters
	return it->second.substr(0, it->second.find_first_of('_'));
}

int get_adb_table_index(const string& actype) {
	int retValue = -1;

	string synonym = get_adb_synonym(actype);
	if (g_adb_indices.find(synonym) != g_adb_indices.end()) {
		retValue = g_adb_indices.at(synonym);
	}

	return retValue;
}

/**
 * Clean up memory space of ADB-related variables
 */
int destroy_adb_performance_tables() {
    g_adb_indices.clear();
    g_adb_synonyms.clear();
    g_adb_model_names.clear();
    g_adb_ptf_models.clear();

    free(h_adb_num_rows);
    free(h_adb_table_start_index);
    free(h_adb_lower_bound_row);
    free(h_adb_fl);
    free(h_adb_vtas_climb_knots);
    free(h_adb_vtas_descent_knots);
    free(h_adb_roc_fpm);
    free(h_adb_rod_fpm);

	return 0;
}
