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
 * tg_adb.h
 *
 *  Created on: Sep 17, 2013
 *      Author: jason
 */

#ifndef TG_ADB_H_
#define TG_ADB_H_

#include "real_t.h"
#include "AdbOPFModel.h"
#include "AdbPTFModel.h"

#include <string>
#include <map>
#include <vector>
#include <set>

using std::string;
using std::map;
using std::vector;
using std::set;

using osi::AdbOPFModel;
using osi::AdbPTFModel;

#define ADB_MAX_ROWS         102
#define ADB_PTF_ROW_SPACING  5

#define KNOTS_TO_FPS          1.68781

typedef struct _adb_synonym_t {
	char actype[10];
	char ptf_file[10];
} adb_synonym_t;

typedef enum _adb_ptf_column_e {
	LO=0,
	NOM,
	HI
} adb_ptf_column_e;

extern map<string, int> g_adb_indices;
extern map<string, string> g_adb_synonyms;
extern set<string> g_adb_model_names;
extern vector<AdbOPFModel> g_adb_opf_models;
extern vector<AdbPTFModel> g_adb_ptf_models;

extern int adb_ptf_total_row_num;
extern int adb_ptf_type_num;
extern int adb_ptf_lower_bound_num;

extern short* h_adb_num_rows;
extern short* h_adb_table_start_index;
extern short* h_adb_lower_bound_row;
extern short* h_adb_fl;
extern short* h_adb_vtas_climb_knots;
extern short* h_adb_vtas_descent_knots;
extern short* h_adb_roc_fpm;
extern short* h_adb_rod_fpm;

int load_adb_performance_tables(const string& adb_ptf_dir,
		                         adb_ptf_column_e col=NOM);
//void* get_adb_performance_data();
int destroy_adb_performance_tables();

string get_adb_synonym(const string& actype, const string& default_type="B733");

int get_adb_table_index(const string& actype);
#endif /* TG_ADB_H_ */
