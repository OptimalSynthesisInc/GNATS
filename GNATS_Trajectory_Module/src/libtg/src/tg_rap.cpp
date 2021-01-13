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
 * tg_rap.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: jason
 */

#include "tg_rap.h"
#include "tg_api.h"
#include "cuda_compat.h"
#include "wind_api.h"

#include "hdf5.h"

#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <omp.h>
#include <dirent.h>
#include <unistd.h>

using namespace std;

bool flag_rap_available = false;

real_t* h_wind_north = NULL;
real_t* h_wind_east = NULL;
real_t* h_wind_north_unc = NULL;
real_t* h_wind_east_unc = NULL;

real_t* d_wind_north = NULL;
real_t* d_wind_east = NULL;
real_t* d_wind_north_unc = NULL;
real_t* d_wind_east_unc = NULL;

real_t g_lat_min;
real_t g_lat_max;
real_t g_lat_step;

real_t g_lon_min;
real_t g_lon_max;
real_t g_lon_step;

real_t g_alt_min;
real_t g_alt_max;
real_t g_alt_step;

#if 0
/*
 * Compute the 1D linear index for 3D subscripts
 */
static int get_ruc_index(const int& i, const int& j, const int& k) {
	return RUC_GRID_CELL_ALT_SIZE * (i*RUC_GRID_CELL_LON_SIZE + j) + k;
}

/*
 * Load a single grib file containing uv wind components into the specified
 * arrays.  we use the libgrib2c library to parse the uv wind records
 * from the grib file.
 */
static int load_ruc_file(const string& fname,
		                 real_t** const uwind,
		                 real_t** const vwind) {

	if(!*uwind) return -1;
	if(!*vwind) return -1;

	// open the grib file for reading
	FILE* f = fopen(fname.c_str(), "r");
	if(!f) {
		fprintf(stderr, "ERROR: failed to open grib file %s\n", fname.c_str());
		return -1;
	}

	// create temporary arrays to hold raw data
	real_t uWindArray[NUM_HYBRID_LEVELS][NUM_WIND_POINTS_I*NUM_WIND_POINTS_J];
	real_t vWindArray[NUM_HYBRID_LEVELS][NUM_WIND_POINTS_I*NUM_WIND_POINTS_J];
	real_t hgtArray[NUM_HYBRID_LEVELS][NUM_WIND_POINTS_I*NUM_WIND_POINTS_J];

	int record = 0;
	int level = 1;
	int geolevel = 1;

	long lskip;  // bytes to skip from start of file to start of message
	long lgrib;  // length of grib message in bytes
	long iseek;  // number of bytes to skip before search

	g2int listsec0[3];
	g2int listsec1[13];
	g2int num_local;
	g2int num_fields;

	gribfield* gfld;

	int unpack = 1;
	int expand = 1;

	unsigned char* cgrib; // buffer to hold the grib message

	// read the file until EOF or problem
	for(;;) {
		++record;

		// find the next grib message
		seekgb(f, iseek, 32000, &lskip, &lgrib);
		if(lgrib == 0) break; // eof or problem.

		// allocate the buffer to be large enough to hold the
		// grib message
		cgrib = (unsigned char*)malloc(lgrib);

		// set the file pointer to the beginning of the record
		// so that we jump here on next file read.
		fseek(f, lskip, SEEK_SET);

		// read the grib message into the buffer
		fread(cgrib, sizeof(unsigned char), lgrib, f);

		// update the file position to the end of the message
		iseek = lskip + lgrib;

		// get information about the record
		g2_info(cgrib, listsec0, listsec1, &num_fields, &num_local);

		printf("variable: %d\n", listsec1[12]);
		// iterate over the number of fields in the record
		for(int n=0; n<num_fields; ++n) {
			// unpack/expand data into a gribfield struct.
			// note: this function call malloc's gfld and so
			// we must free it when we're done.
			g2_getfld(cgrib, n+1, unpack, expand, &gfld);
			g2_free(gfld);
		}
		free(cgrib);
	}
	fclose(f);
	return 0;
}
#endif

static int get_wind_grid_def_file(const string& dirname,
		                          string* const file) {
	int retValue = -1;

	if (!file) return retValue;

	DIR* dir;
	struct dirent* entry;
	dir = opendir(dirname.c_str());
	if(!dir) {
		fprintf(stderr, "      ERROR: could not open directory %s\n", dirname.c_str());
		return -1;
	}

	while((entry=readdir(dir)) != NULL) {
		string fname(entry->d_name);

		// TODO: filter files by extension
		// for now, we assume the directory contains only the correct files.
		unsigned int underscore = fname.find_last_of("_");
		string suffix = (underscore < fname.length() ? fname.substr(underscore) : "");
		if(suffix == "_grid.h5") {
			*file = fname;

			retValue = 0;

			break;
		}
	}

	closedir(dir);
	dir = NULL;

	return retValue;
}

static int get_wind_files(const string& dirname,
				          vector<string>* const files) {

	if(!files) return -1;

	DIR* dir;
	struct dirent* entry;
	dir = opendir(dirname.c_str());
	if(!dir) {
		fprintf(stderr, "ERROR: could not open directory %s\n", dirname.c_str());
		return -1;
	}

	while((entry=readdir(dir)) != NULL) {
		string fname(entry->d_name);

		// TODO: filter files by extension
		// for now, we assume the directory contains only the correct files.
		unsigned int underscore = fname.find_last_of("_");
		string suffix = (underscore < fname.length() ? fname.substr(underscore) : "");
		int dot = fname.find_last_of(".");
		string ext = fname.substr(dot);
		if(ext == ".h5" && suffix != "_grid.h5") {
			files->push_back(fname);
		}
	}

	closedir(dir);
	dir = NULL;

	return 0;
}

static int read_hdf5(const string& hdf5_file,
		double* const n_data,
		double* const e_data,
		const int table_size) {
    int retValue = -1;

	WindGrid grid_hdf5;

    // Read the HDF5 file
	if (read_wind_grid_hdf5(hdf5_file, &grid_hdf5) == 0) {
		for (size_t j = 0; j < table_size; ++j) {
			n_data[j] = grid_hdf5.n_data[j];
			e_data[j] = grid_hdf5.e_data[j];
		}

		retValue = 0;
	}

    return retValue;
}

static int read_grid_hdf5(const string& hdf5_file,
			  double* const lat_min,
			  double* const lat_max,
			  double* const lat_step,
			  double* const lon_min,
			  double* const lon_max,
			  double* const lon_step,
			  double* const alt_min,
			  double* const alt_max,
			  double* const alt_step) {
	int retValue = -1;

	WindGrid grid_hdf5;

	// Read the grid file
	if (read_wind_grid_hdf5(hdf5_file, &grid_hdf5) == 0) {
		*lat_min = grid_hdf5.lat_min;
		*lat_max = grid_hdf5.lat_max;
		*lat_step = grid_hdf5.lat_step;
		*lon_min = grid_hdf5.lon_min;
		*lon_max = grid_hdf5.lon_max;
		*lon_step = grid_hdf5.lon_step;
		*alt_min = grid_hdf5.alt_min;
		*alt_max = grid_hdf5.alt_max;
		*alt_step = grid_hdf5.alt_step;

		retValue = 0;
	}

	return retValue;
}

/* TODO: PARIKSHIT ADDER.
 * RAPID REFRESH SUITE NOT WORKING WITH THE EXISTING CODE
 * HENCE HAD TO CHANGE IMPLEMENTATION.
 * *****************************
 *
 */
int load_ruc_alt(const string& wind_dir){
	g_wind_dir = wind_dir;

	// get the hdf5 files in the directory
	vector<string> hdf5_files;
	get_wind_files(wind_dir, &hdf5_files);

	if (hdf5_files.empty()){
		printf("Exiting no wind files specified...\n");
		exit(-1);
	}

	// sort the files by rap/ruc hour.
	sort(hdf5_files.begin(), hdf5_files.end());

	//Open first file
    hid_t file_id = H5Fopen((wind_dir + "/" + hdf5_files.at(0)).c_str(), H5F_ACC_RDONLY,
    		H5P_DEFAULT);
    if(file_id < 0) {
        fprintf(stderr, "ERROR: could not open wind HDF5 file %s\n",
        		hdf5_files.at(0).c_str());
        exit(-1);
    }

    // open the dataset
    hid_t def_dataset_id = H5Dopen2(file_id, "/grid_def", H5P_DEFAULT);
    if(def_dataset_id < 0) {
        fprintf(stderr, "ERROR: could not open grid_def dataset in %s\n",
                hdf5_files.at(0).c_str());
        exit(-1);
    }

    double grid_def[13];
    herr_t status = H5Dread(def_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, grid_def);
    if(status) {
        fprintf(stderr, "ERROR: could not read grid_def dataset in %s\n",
                hdf5_files.at(0).c_str());
        exit(-1);
    }

    g_lat_min = (real_t)grid_def[0];
    g_lat_max = (real_t)grid_def[1];
    g_lat_step = (real_t)grid_def[2];
    g_lon_min = (real_t)grid_def[3];
    g_lon_max = (real_t)grid_def[4];
    g_lon_step = (real_t)grid_def[5];
    g_alt_min = (real_t)grid_def[6];
    g_alt_max = (real_t)grid_def[7];
    g_alt_step = (real_t)grid_def[8];

	size_t lat_size = (int)1+ceil((g_lat_max-g_lat_min) / g_lat_step);
	size_t lon_size = (int)1+ceil((g_lon_max-g_lon_min) / g_lon_step);
	size_t alt_size = (int)1+ceil((g_alt_max-g_alt_min) / g_alt_step);
	size_t table_size = lat_size * lon_size * alt_size;

	// determine the number of hours of simulation. this is specified
	// by the global variable g_horizon, in seconds.

	int num_hours = ceil(g_horizon / 3600.);

	// allocate global wind arrays. each array must be large enough to
	// hold a wind table for each hour of simulation.
	size_t array_size = num_hours * table_size;
	h_wind_north = (real_t*)calloc(array_size, sizeof(real_t));
	h_wind_east = (real_t*)calloc(array_size, sizeof(real_t));
	h_wind_north_unc = (real_t*)calloc(array_size, sizeof(real_t));
	h_wind_east_unc = (real_t*)calloc(array_size, sizeof(real_t));

	// iterate over each file and load the data into
	// north and east wind arrays
	for(unsigned int i=0; i<hdf5_files.size(); ++i) {
		int offset = i*table_size;
		string hdf5_file = wind_dir + "/" + hdf5_files.at(i);

		double* north_buf = (double*)calloc(table_size, sizeof(double));
		double* east_buf = (double*)calloc(table_size, sizeof(double));

		read_hdf5(hdf5_file, north_buf, east_buf, table_size);
		// element wise copy of buffers into global arrays.
		// need to do this element wise because the global arrays might
		// be typedef'ed as floats instead of doubles.

		for(size_t j=0; j<table_size; ++j) {
			h_wind_north[offset+j] = (real_t)north_buf[j];
			h_wind_east[offset+j] = (real_t)east_buf[j];
		}

		free(north_buf);
		free(east_buf);
	}

#if USE_GPU
	// allocate device global mem
	cuda_calloc((void**)&d_wind_north, array_size);
	cuda_calloc((void**)&d_wind_east, array_size);
	cuda_calloc((void**)&d_wind_north_unc, array_size);
	cuda_calloc((void**)&d_wind_east_unc, array_size);

	// copy from device to host
	cuda_memcpy(d_wind_north, h_wind_north, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_east, h_wind_east, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_north_unc, h_wind_north_unc, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_east_unc, h_wind_east_unc, array_size, cuda_memcpy_HtoD);
#else
	// assign host pointers to device pointers
	d_wind_north = h_wind_north;
	d_wind_east = h_wind_east;
	d_wind_north_unc = h_wind_north_unc;
	d_wind_east_unc = h_wind_east_unc;
#endif
	return 0;
}

int load_rap(const string& wind_dir) {
	flag_rap_available = false; // Reset

	if (wind_dir == "") {
		return -1;
	}

	g_wind_dir = wind_dir;

	printf("  Loading winds\n");

	// osi - jason: don't filter by 2017
	//TODO:PARIKSHIT ADDER FOR ADDING RAPID REFRESH SUITE FILES.
	//size_t found = wind_dir.find("2017");
	//if (found != string::npos)
		//return load_ruc_alt(wind_dir);

	// get the wind grid def file in the directory
	string grid_def_file;
	if (get_wind_grid_def_file(wind_dir, &grid_def_file) < 0) {
		return -1;
	}

	// If we can't access the directory
	if (grid_def_file.length() == 0) {
		printf("      Failed to find wind definition file\n");

		return -1;
	}

	// read the grid def file
	grid_def_file = wind_dir + "/" + grid_def_file;

	double lat_min = 0.0, lat_max = 0.0, lat_step = 0.0;
	double lon_min = 0.0, lon_max = 0.0, lon_step = 0.0;
	double alt_min = 0.0, alt_max = 0.0, alt_step = 0.0;

	// Read grid definition file
	if (read_grid_hdf5(grid_def_file,
				   &lat_min, &lat_max, &lat_step,
				   &lon_min, &lon_max, &lon_step,
				   &alt_min, &alt_max, &alt_step) == -1) {
		printf("      Wind definition data doesn't exist.\n");

		return -1;
	}

	// possible down-casting if real_t is typedef'ed to float.
	g_lat_min = (real_t)lat_min;
	g_lat_max = (real_t)lat_max;
	g_lat_step = (real_t)lat_step;
	g_lon_min = (real_t)lon_min;
	g_lon_max = (real_t)lon_max;
	g_lon_step = (real_t)lon_step;
	g_alt_min = (real_t)alt_min;
	g_alt_max = (real_t)alt_max;
	g_alt_step = (real_t)alt_step;

	// compute the array sizes based on grid def. note: we add 1
	// because we want the range to be inclusive of max and min values.
	size_t lat_size = (int)1+(int)ceil(((double)g_lat_max - (double)g_lat_min) / (double)g_lat_step);
	size_t lon_size = (int)1+(int)ceil(((double)g_lon_max - (double)g_lon_min) / (double)g_lon_step);
	size_t alt_size = (int)1+(int)ceil(((double)g_alt_max - (double)g_alt_min) / (double)g_alt_step);
	size_t table_size = lat_size * lon_size * alt_size;

	// get the hdf5 files in the directory
	vector<string> hdf5_files;
	get_wind_files(wind_dir, &hdf5_files);

	if (hdf5_files.size() ==0) {
		printf("      Wind detail data doesn't exist.\n");
	} else {
		// sort the files by rap/ruc hour.
		sort(hdf5_files.begin(), hdf5_files.end());

		// determine the number of hours of simulation. this is specified
		// by the global variable g_horizon, in seconds.
		int num_hours = ceil(g_horizon / 3600.);

		// allocate global wind arrays. each array must be large enough to
		// hold a wind table for each hour of simulation.

		size_t array_size = num_hours * table_size;
		h_wind_north = (real_t*)calloc(array_size, sizeof(real_t));
		h_wind_east = (real_t*)calloc(array_size, sizeof(real_t));
		h_wind_north_unc = (real_t*)calloc(array_size, sizeof(real_t));
		h_wind_east_unc = (real_t*)calloc(array_size, sizeof(real_t));

		// iterate over each file and load the data into
		// north and east wind arrays
		for (unsigned int i=0; i<hdf5_files.size(); ++i) {
			int offset = i*table_size;
			string hdf5_file = wind_dir + "/" + hdf5_files.at(i);

			double* north_buf = (double*)calloc(table_size, sizeof(double));
			double* east_buf = (double*)calloc(table_size, sizeof(double));

			// Read individual wind HDF5 file
			read_hdf5(hdf5_file, north_buf, east_buf, table_size);

			// element wise copy of buffers into global arrays.
			// need to do this element wise because the global arrays might
			// be typedef'ed as floats instead of doubles.

			for (size_t j=0; j<table_size; ++j) {
				h_wind_north[offset+j] = (real_t)north_buf[j];
				h_wind_east[offset+j] = (real_t)east_buf[j];
			}

			if (north_buf != NULL)
				free(north_buf);

			if (east_buf != NULL)
				free(east_buf);
		}

#if USE_GPU
	// allocate device global mem
	cuda_calloc((void**)&d_wind_north, array_size);
	cuda_calloc((void**)&d_wind_east, array_size);
	cuda_calloc((void**)&d_wind_north_unc, array_size);
	cuda_calloc((void**)&d_wind_east_unc, array_size);

	// copy from device to host
	cuda_memcpy(d_wind_north, h_wind_north, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_east, h_wind_east, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_north_unc, h_wind_north_unc, array_size, cuda_memcpy_HtoD);
	cuda_memcpy(d_wind_east_unc, h_wind_east_unc, array_size, cuda_memcpy_HtoD);
#else
	// assign host pointers to device pointers
	d_wind_north = h_wind_north;
	d_wind_east = h_wind_east;
	d_wind_north_unc = h_wind_north_unc;
	d_wind_east_unc = h_wind_east_unc;
#endif

#if 0
	// if librap globals have already been loaded then
	// destroy the existing memory
	if(librap_initialized) {
		destroyRUCData();
	}

	// load the grib file
	FILE* f = fopen(grib_file.c_str(), "r");
	if(!f) {
		printf("Error opening grib file: %s\n", grib_file.c_str());
		return -1;
	}
	loadRUCFile(f);
	fclose(f);

	// init the librap global data
	initializeRUCData();

	librap_initialized = true;

	// TODO: load RUC/RAP data to device memory
#if USE_GPU
	size_t nlat = (MAX_LAT-MIN_LAT+1)/LAT_STEP;
	size_t nlon = (MAX_LON-MIN_LON+1)/LON_STEP;
	size_t nalt = (MAX_ALT-MIN_ALT+1)/ALT_STEP;
	size_t table_bytes = nlat*nlon*nalt*sizeof(float);

	cuda_calloc((void**)&d_uWindTable, table_bytes);
	cuda_calloc((void**)&d_vWindTable, table_bytes);
	cuda_calloc((void**)&d_uWindPertTable, table_bytes);
	cuda_calloc((void**)&d_vWindPertTable, table_bytes);

	cuda_memcpy(d_uWindTable, uWindTable, table_bytes, cudaMemcpyHostToDevice);
	cuda_memcpy(d_vWindTable, vWindTable, table_bytes, cudaMemcpyHostToDevice);
	cuda_memcpy(d_uWindPertTable, uWindPertTable, table_bytes, cudaMemcpyHostToDevice);
	cuda_memcpy(d_vWindPertTable, vWindPertTable, table_bytes, cudaMemcpyHostToDevice);

#else
	d_uWindTable = &uWindTable[0][0][0];
	d_vWindTable = &vWindTable[0][0][0];
	d_uWindPertTable = &uWindPertTable[0][0][0];
	d_vWindPertTable = &vWindPertTable[0][0][0];

#endif

#endif

		if (!hdf5_files.empty()) {
			hdf5_files.clear();
		}

		flag_rap_available = true;
	}

	return 0;
}

int destroy_rap() {
	if(h_wind_north) {
		free(h_wind_north);
		h_wind_north = NULL;
	}
	if(h_wind_east) {
		free(h_wind_east);
		h_wind_east = NULL;
	}
	if(h_wind_north_unc) {
		free(h_wind_north_unc);
		h_wind_north_unc = NULL;
	}
	if(h_wind_east_unc) {
		free(h_wind_east_unc);
		h_wind_east_unc = NULL;
	}
#if USE_GPU
	if(d_wind_north) {
		cuda_free(d_wind_north);
		d_wind_north = NULL;
	}
	if(d_wind_east) {
		cuda_free(d_wind_east);
		d_wind_east = NULL;
	}
	if(d_wind_north_unc) {
		cuda_free(d_wind_north_unc);
		d_wind_north_unc = NULL;
	}
	if(d_wind_east_unc) {
		cuda_free(d_wind_east_unc);
		d_wind_east_unc = NULL;
	}
#endif

#if 0
	if(librap_initialized) {
		destroyRUCData();
		librap_initialized = false;
	} else {

	}
#endif
	return 0;
}
