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
 * tg_sectors.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: jason
 */

#include "tg_sectors.h"
#include "tg_simulation.h"

#include "NatsSector.h"
#include "NatsDataLoader.h"

#include "cuda_compat.h"

#include <string>
#include <vector>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <unistd.h>

using namespace std;

/*
 * host-side global variables
 */
vector<NatsSector> g_sectors;

bool flag_sector_available = false;

sector_t* h_sector_array;
sector_t* d_sector_array;

static int polygon_array_length = SECTOR_POLYGON_MAX_LENGTH;

int* h_sector_grid = NULL;
int* h_sector_grid_cell_counts = NULL;
int* d_sector_grid = NULL;
int* d_sector_grid_cell_counts = NULL;

static int load_sectors(const string& fpath,
		                NatsDataLoader& loader) {
	int err = loader.loadSectors(fpath, &g_sectors);
	if(err <= 0) {
		return err;
	}

	flag_sector_available = true;

	return 0;
}

static int get_max_boundary_length() {
	return SECTOR_POLYGON_MAX_LENGTH;
}

static void array_min_max(const vector<double>& array,
		                  real_t* const min_out, real_t* const max_out) {
	vector<double> sorted(array.begin(), array.end());
	sort(sorted.begin(), sorted.end());
	*min_out = sorted.front();
	*max_out = sorted.back();
}

static int get_index(int i, int j, int k, int l) {
	return SECTOR_GRID_CELL_MAX_COUNT * (SECTOR_GRID_ALT_SIZE *
			(i*SECTOR_GRID_LON_SIZE + j) + k) + l;
}

static int get_index(int i, int j, int k) {
	return SECTOR_GRID_ALT_SIZE * (i*SECTOR_GRID_LON_SIZE + j) + k;
}

static int load_sectors_gpu() {

	int num_sectors = g_sectors.size();

	// compute the polygon array length
	polygon_array_length = get_max_boundary_length();

	h_sector_array = (sector_t*)calloc(num_sectors, sizeof(sector_t));

	// populate the data buffer
	vector<NatsSector>::iterator iter;
	int i=0;
	for(iter=g_sectors.begin(); iter!=g_sectors.end(); ++iter) {
		const NatsSector* sector = &(*iter);
		h_sector_array[i].num_vertices = sector->numVertices;
		h_sector_array[i].alt_min = sector->minAltitude;
		h_sector_array[i].alt_max = sector->maxAltitude;

		array_min_max(sector->latitudes, &h_sector_array[i].lat_min,
				      &h_sector_array[i].lat_max);
		array_min_max(sector->longitudes, &h_sector_array[i].lon_min,
				      &h_sector_array[i].lon_max);

		for(int j=0; j<sector->numVertices; ++j) {
			h_sector_array[i].latitude[j] = sector->latitudes.at(j);
			h_sector_array[i].longitude[j] = sector->longitudes.at(j);
		}
		++i;
	}

	size_t sector_grid_bytes = SECTOR_GRID_LAT_SIZE * SECTOR_GRID_LON_SIZE *
			SECTOR_GRID_ALT_SIZE * SECTOR_GRID_CELL_MAX_COUNT * sizeof(int);
	size_t sector_grid_cell_count_bytes = SECTOR_GRID_LAT_SIZE *
			SECTOR_GRID_LON_SIZE * SECTOR_GRID_ALT_SIZE * sizeof(int);

	// allocate host lookup table memory
	h_sector_grid = (int*)malloc(sector_grid_bytes);
	memset(h_sector_grid, 0, sector_grid_bytes);

	h_sector_grid_cell_counts = (int*)malloc(sector_grid_cell_count_bytes);
	memset(h_sector_grid_cell_counts, 0, sector_grid_cell_count_bytes);

	// build the lookup grid
	for(int i=0; i<SECTOR_GRID_LAT_SIZE; ++i) {
		real_t lat_min = i*SECTOR_GRID_CELL_LAT_SIZE;
		real_t lat_max = (i+1)*SECTOR_GRID_CELL_LAT_SIZE;

		for(int j=0; j<SECTOR_GRID_LON_SIZE; ++j) {
			real_t lon_min = j*SECTOR_GRID_CELL_LON_SIZE - 360.;
			real_t lon_max = (j+1)*SECTOR_GRID_CELL_LON_SIZE - 360.;

			for(int k=0; k<SECTOR_GRID_ALT_SIZE; ++k) {
				real_t alt_min = k*SECTOR_GRID_CELL_ALT_SIZE;
				real_t alt_max = (k+1)*SECTOR_GRID_CELL_ALT_SIZE;

				for(int s=0; s<num_sectors; ++s) {

					if(lat_max >= h_sector_array[s].lat_min && lat_min <= h_sector_array[s].lat_max &&
					   lon_max >= h_sector_array[s].lon_min && lon_min <= h_sector_array[s].lon_max &&
					   alt_max >= h_sector_array[s].alt_min && alt_min <= h_sector_array[s].alt_max) {
						int count_index = get_index(i,j,k);
						int l = h_sector_grid_cell_counts[count_index];
						int grid_index = get_index(i,j,k,l);
						h_sector_grid[grid_index] = s;
						h_sector_grid_cell_counts[count_index]++;
					}
				}
			}
		}
	}

	// if we are using a GPU then allocate device global memory and
	// copy from host to device.  otherwise, just assign the host pointers
	// to the 'device' pointers.
#if USE_GPU

	// device allocate the lookup grid and cell counts
	cuda_calloc((void**)&d_sector_grid, sector_grid_bytes);
	cuda_calloc((void**)&d_sector_grid_cell_counts,
			    sector_grid_cell_count_bytes);
	cuda_calloc((void**)&d_sector_array, num_sectors*sizeof(sector_t));

	// copy from host to device
	cuda_memcpy(d_sector_grid, h_sector_grid, sector_grid_bytes,
			    cudaMemcpyHostToDevice);
	cuda_memcpy(d_sector_grid_cell_counts, h_sector_grid_cell_counts,
			    sector_grid_cell_count_bytes, cudaMemcpyHostToDevice);
	cuda_memcpy(d_sector_array, h_sector_array, num_sectors*sizeof(sector_t),
			    cudaMemcpyHostToDevice);
#else
	// assign host pointers to device pointers
	//d_sector_data = h_sector_data;
	d_sector_grid = h_sector_grid;
	d_sector_grid_cell_counts = h_sector_grid_cell_counts;
	d_sector_array = h_sector_array;
#endif

	set_device_sector_pointers();

	return 0;
}

int load_sectors(const string& data_dir) {

	printf("  Loading sector data\n");
	string fname;
	NatsDataLoader loader;
	int err;

	g_sectors.clear();

	// If we can't access the directory
	if ((data_dir.length() == 0) || ( access( data_dir.c_str(), F_OK ) == -1 )) {
		printf("      Failed to open directory %s\n", data_dir.c_str());

		return -1;
	}

	// load LO sectors
	fname = data_dir + "/Sectors_LO.crypt";
	err = load_sectors(fname, loader);
	if(err < 0) {
		printf("Error loading low sectors\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	// load HI sectors
	fname = data_dir + "/Sectors_HI.crypt";
	err = load_sectors(fname, loader);
	if(err < 0) {
		printf("Error loading high sectors\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	// load Super sectors
	fname = data_dir + "/Sectors_SUP.crypt";
	err = load_sectors(fname, loader);
	if(err < 0) {
		printf("Error loading super sectors\n");

		// Detach the current thread
		pthread_detach(pthread_self());

		return err;
	}

	// load sector data to device memory
	load_sectors_gpu();

	// Detach the current thread
	pthread_detach(pthread_self());

	return 0;
}

int destroy_sectors() {

    g_sectors.clear();

	if(h_sector_array) {
		free(h_sector_array);
		h_sector_array = NULL;
	}
	if(h_sector_grid) {
		free(h_sector_grid);
		h_sector_grid = NULL;
	}
	if(h_sector_grid_cell_counts) {
		free(h_sector_grid_cell_counts);
		h_sector_grid_cell_counts = NULL;
	}

#if USE_GPU
	if(d_sector_array) {
		cuda_free(d_sector_array);
		d_sector_array = NULL;
	}
	if(d_sector_grid) {
		cuda_free(d_sector_grid);
		d_sector_grid = NULL;
	}
	if(d_sector_grid_cell_counts) {
		cuda_free(d_sector_grid_cell_counts);
		d_sector_grid_cell_counts = NULL;
	}
#endif
	return 0;
}

int get_sector_polygon_array_length() {
	return polygon_array_length;
}
