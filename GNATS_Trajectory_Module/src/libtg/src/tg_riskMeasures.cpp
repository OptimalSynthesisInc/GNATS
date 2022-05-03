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

#include "tg_riskMeasures.h"

#include "util_string.h"

#include <fstream>
#include <dirent.h>

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

using namespace std;

bool flag_exist_ac_risk_measures_data = false;

aviation_occurence_profile_t aviation_occurence_profile;

// Template of risk measures metrics
ac_risk_measures_metrics_t ac_risk_measures_metrics_template;

// Risk measures data used during simulation
ac_risk_measures_metrics_t* ac_risk_measures_data;

int tmp_aviation_occurence_size = 50;

int tmp_idx = 0;
char** tmp_codes;
char** tmp_names;

char* getOccurenceName_by_code(const char* code) {
	char* retCharArray = NULL;

	for (int k = 0; k < aviation_occurence_profile.record_count; k++) {
		if (strcmp(code, aviation_occurence_profile.codes[k]) == 0) {
			retCharArray = aviation_occurence_profile.names[k];

			break;
		}
	}

	return retCharArray;
}

int load_aviationOccurenceProfile(const char* dirPath) {
	// Allocate memory space
	tmp_codes = (char**)calloc(tmp_aviation_occurence_size, sizeof(char*));
	tmp_names = (char**)calloc(tmp_aviation_occurence_size, sizeof(char*));

	// Initialization
	for (int i = 0; i < tmp_aviation_occurence_size; i++) {
		tmp_codes[i] = NULL;
		tmp_names[i] = NULL;
	}

	// ========================================================================

	DIR *dir = opendir(dirPath);
	string string_filenamePath;
	string read_line;
	ifstream in;
	char *entry;

	if (dir == NULL) {
		printf("Could not open directory %s", dirPath);
		return -1;
	}

	int idx = 0;

	string_filenamePath.assign(dirPath);
	string_filenamePath.append("/");
	string_filenamePath.append("RM_AviationOccurence.profile");

	in.open(string_filenamePath.c_str());

	while (in.good()) {
		read_line = "";
		getline(in, read_line);
		if (strlen(read_line.c_str()) == 0) continue;

		if (read_line.length() > 0) {
			char* tempCharArray = (char*)calloc((read_line.length()+1), sizeof(char));
			strcpy(tempCharArray, read_line.c_str());

			// Retrieve individual data split by "," sign
			// Get first entry
			entry = strtok(tempCharArray, ",");

			tmp_codes[idx] = (char*)calloc(strlen(entry)+1, sizeof(char));
			strcpy(tmp_codes[idx], entry);
			tmp_codes[idx][strlen(entry)] = '\0';

			entry = strtok(NULL, ",");
			entry = trimwhitespace(entry);

			tmp_names[idx] = (char*)calloc(strlen(entry)+1, sizeof(char));
			strcpy(tmp_names[idx], entry);
			tmp_names[idx][strlen(entry)] = '\0';

			if (tempCharArray != NULL) {
				free(tempCharArray);
				tempCharArray = NULL;
			}

			idx++;
		}
	}

	aviation_occurence_profile.record_count = idx;

	aviation_occurence_profile.codes = (char**)calloc(aviation_occurence_profile.record_count, sizeof(char*));
	aviation_occurence_profile.names = (char**)calloc(aviation_occurence_profile.record_count, sizeof(char*));

	for (int i = 0; i < aviation_occurence_profile.record_count; i++) {
		if (tmp_codes[i] != NULL) {
			aviation_occurence_profile.codes[i] = (char*)calloc(strlen(tmp_codes[i])+1, sizeof(char));
			strcpy(aviation_occurence_profile.codes[i], tmp_codes[i]);
			aviation_occurence_profile.codes[i][strlen(tmp_codes[i])] = '\0';
		}

		if (tmp_names[i] != NULL) {
			aviation_occurence_profile.names[i] = (char*)calloc(strlen(tmp_names[i])+1, sizeof(char));
			strcpy(aviation_occurence_profile.names[i], tmp_names[i]);
			aviation_occurence_profile.names[i][strlen(tmp_names[i])] = '\0';
		}
	}

	in.close();

	closedir(dir);

	// Free memory
	for (int i = 0; i < tmp_aviation_occurence_size; i++) {
		if (tmp_codes[i] != NULL) {
			free(tmp_codes[i]);
			tmp_codes[i] = NULL;
		}

		if (tmp_names[i] != NULL) {
			free(tmp_names[i]);
			tmp_names[i] = NULL;
		}
	}

	if (tmp_codes != NULL) {
		free(tmp_codes);
		tmp_codes = NULL;
	}

	if (tmp_names != NULL) {
		free(tmp_names);
		tmp_names = NULL;
	}

	return 0;
}

int load_flightphase_aviationOccurence_mapping(const char* dirPath) {

	DIR *dir = opendir(dirPath);
	string string_filenamePath;
	string read_line;
	ifstream in;
	char *entry;

	if (dir == NULL) {
		printf("Could not open directory %s", dirPath);
		return -1;
	}

	ac_risk_measures_metrics_template.count_flight_phase = ENUM_Flight_Phase_Count;

	ac_risk_measures_metrics_template.flight_phases = (ENUM_Flight_Phase*)calloc(ac_risk_measures_metrics_template.count_flight_phase, sizeof(ENUM_Flight_Phase));

	// Insert data
	for (int i = 0; i < ac_risk_measures_metrics_template.count_flight_phase; i++) {
		ac_risk_measures_metrics_template.flight_phases[i] = ENUM_Flight_Phase(i);
	}

	ac_risk_measures_metrics_template.flight_phase_risk = (aviation_occurence_risk_value_t**)calloc(ac_risk_measures_metrics_template.count_flight_phase, sizeof(aviation_occurence_risk_value_t*));

	// Initialization
	for (int i = 0; i < ac_risk_measures_metrics_template.count_flight_phase; i++) {
		ac_risk_measures_metrics_template.flight_phase_risk[i] = (aviation_occurence_risk_value_t*)calloc(1, sizeof(aviation_occurence_risk_value_t));

		ac_risk_measures_metrics_template.flight_phase_risk[i]->record_count = 0; // Reset
	}

	// ========================================================================

	string_filenamePath.assign(dirPath);
	string_filenamePath.append("/");
	string_filenamePath.append("RM_Flightphase_AviationOccurence.mapping");

	in.open(string_filenamePath.c_str());

	string tmpAviationOccurenceCode;
	string tmpAviationOccurenceName;

	int tmp_flight_phase_idx = 0;

	while (in.good()) {
		read_line = "";
		getline(in, read_line);

		if (strlen(read_line.c_str()) == 0) continue;

		if (read_line.length() > 0) {
			char* tempCharArray = (char*)calloc(read_line.length()+1, sizeof(char));
			strcpy(tempCharArray, read_line.c_str());

			if (-1 < indexOf(tempCharArray, "FLIGHT_PHASE")) {
				tmp_flight_phase_idx = (int)getFlight_Phase(tempCharArray);

				free(tempCharArray);
				tempCharArray = NULL;

				continue;
			}

			// Allocate memory space
			tmp_codes = (char**)calloc(aviation_occurence_profile.record_count, sizeof(char*));

			// Initialization
			for (int i = 0; i < aviation_occurence_profile.record_count; i++) {
				tmp_codes[i] = NULL;
			}

			tmp_idx = 0;

			// Retrieve individual data split by "," sign
			// Get first entry
			entry = strtok(tempCharArray, ",");

			tmp_codes[tmp_idx] = (char*)calloc(strlen(entry)+1, sizeof(char));
			strcpy(tmp_codes[tmp_idx], entry);
			tmp_codes[tmp_idx][strlen(entry)] = '\0';

			tmp_idx++;

			entry = strtok(NULL, ",");
			while ((entry != NULL) && (0 < strlen(entry))) {
				entry = trimwhitespace(entry);

				tmp_codes[tmp_idx] = (char*)calloc(strlen(entry)+1, sizeof(char));
				strcpy(tmp_codes[tmp_idx], entry);
				tmp_codes[tmp_idx][strlen(entry)] = '\0';

				tmp_idx++;

				entry = strtok(NULL, ",");
			}

			if (tempCharArray != NULL) {
				free(tempCharArray);
				tempCharArray = NULL;
			}

			ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->record_count = aviation_occurence_profile.record_count;

			if (ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->record_count > 0) {
				ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->codes = (char**)calloc(ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->record_count, sizeof(char*));
				ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->risk_value = (double*)calloc(ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->record_count, sizeof(double));

				for (int j = 0; j < ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->record_count; j++) {
					if (tmp_codes[j] == NULL) {
						ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->codes[j] = NULL;
					} else {
						ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->codes[j] = (char*)calloc(strlen(tmp_codes[j])+1, sizeof(char));
						strcpy(ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->codes[j], tmp_codes[j]);
						ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->codes[j][strlen(tmp_codes[j])] = '\0';

						ac_risk_measures_metrics_template.flight_phase_risk[tmp_flight_phase_idx]->risk_value[j] = 0.0;
					}
				}
			}

			// Free memory
			for (int i = 0; i < aviation_occurence_profile.record_count; i++) {
				if (tmp_codes[i] != NULL) {
					free(tmp_codes[i]);
					tmp_codes[i] = NULL;
				}
			}

			if (tmp_codes != NULL) {
				free(tmp_codes);
			}
		}
	} // end - while

	in.close();

	closedir(dir);

	flag_exist_ac_risk_measures_data = true;

	return 0;
}
