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
 * tg_sectors.h
 *
 *  Created on: Sep 18, 2013
 *      Author: jason
 */

#ifndef TG_SECTORS_H_
#define TG_SECTORS_H_

#include <string>
#include <vector>
#include "NatsSector.h"
#include "real_t.h"

using std::string;
using std::vector;

#define SECTOR_POLYGON_MAX_LENGTH   100

// sector grid cell definitions
#define SECTOR_GRID_CELL_LAT_SIZE    5   /* degrees longitude */
#define SECTOR_GRID_CELL_LON_SIZE    5   /* degrees latitude */
#define SECTOR_GRID_CELL_ALT_SIZE  1000   /* ft */
#define SECTOR_GRID_CELL_MAX_COUNT  500
#define SECTOR_GRID_LAT_MIN           0
#define SECTOR_GRID_LAT_MAX          90
#define SECTOR_GRID_LON_MIN        -360
#define SECTOR_GRID_LON_MAX           0
#define SECTOR_GRID_ALT_MIN           0
#define SECTOR_GRID_ALT_MAX       51000   /* max ADB altitude, ft */

#define SECTOR_GRID_LAT_SIZE ((SECTOR_GRID_LAT_MAX-SECTOR_GRID_LAT_MIN)/SECTOR_GRID_CELL_LAT_SIZE)
#define SECTOR_GRID_LON_SIZE ((SECTOR_GRID_LON_MAX-SECTOR_GRID_LON_MIN)/SECTOR_GRID_CELL_LON_SIZE)
#define SECTOR_GRID_ALT_SIZE ((SECTOR_GRID_ALT_MAX-SECTOR_GRID_ALT_MIN)/SECTOR_GRID_CELL_ALT_SIZE)

// Size of variable "c_sector_grid_cell_counts"
#define VAR_SECTOR_GRID_CELL_COUNTS_SIZE (SECTOR_GRID_LAT_SIZE * SECTOR_GRID_LON_SIZE * SECTOR_GRID_ALT_SIZE)
// Size of variable "c_sector_grid"
#define VAR_SECTOR_GRID_SIZE (SECTOR_GRID_LAT_SIZE * SECTOR_GRID_LON_SIZE * SECTOR_GRID_ALT_SIZE * SECTOR_GRID_CELL_MAX_COUNT)

typedef struct _sector_t {
	// polygon vertices
	int num_vertices;
	real_t latitude[SECTOR_POLYGON_MAX_LENGTH];
	real_t longitude[SECTOR_POLYGON_MAX_LENGTH];
	// bounding volume
	real_t alt_min;
	real_t alt_max;
	real_t lat_min;
	real_t lat_max;
	real_t lon_min;
	real_t lon_max;
} sector_t;

int load_sectors(const string& data_dir);
int destroy_sectors();
int get_sector_polygon_array_length();

extern vector<NatsSector> g_sectors;

extern sector_t* h_sector_array;
extern sector_t* d_sector_array;

extern vector<NatsSector> g_sectors;
extern int* h_sector_grid;
extern int*  h_sector_grid_cell_counts;
extern int* d_sector_grid;
extern int*  d_sector_grid_cell_counts;

#endif /* TG_SECTORS_H_ */
