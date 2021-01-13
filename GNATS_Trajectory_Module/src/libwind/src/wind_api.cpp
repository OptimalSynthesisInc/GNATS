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
 * wind_api.cpp
 *
 * This module provides functions for reading/writing wind files
 * stored in HDF5 format created using the wind_tool
 */

#include "wind_api.h"

#include <string>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include "hdf5.h"



using namespace std;



WindGrid::WindGrid() :
    lat_min(0),
    lat_max(0),
    lat_step(0),
    lon_min(0),
    lon_max(0),
    lon_step(0),
    alt_min(0),
    alt_max(0),
    alt_step(0),
    n_data(NULL),
    e_data(NULL),
    lat_size(0),
    lon_size(0),
    alt_size(0),
    table_size(0) {
}

WindGrid::WindGrid(const double& lat_min,
                   const double& lat_max,
                   const double& lat_step,
                   const double& lon_min,
                   const double& lon_max,
                   const double& lon_step,
                   const double& alt_min,
                   const double& alt_max,
                   const double& alt_step) :
    lat_min(lat_min),
    lat_max(lat_max),
    lat_step(lat_step),
    lon_min(lon_min),
    lon_max(lon_max),
    lon_step(lon_step),
    alt_min(alt_min),
    alt_max(alt_max),
    alt_step(alt_step),
    n_data(NULL),
    e_data(NULL),
    lat_size(0),
    lon_size(0),
    alt_size(0),
    table_size(0) {

    // compute the array sizes based on grid def. note: we add 1
    // because we want the range to be inclusive of max and min values.
    lat_size = (int)1+ceil((lat_max-lat_min) / lat_step);
    lon_size = (int)1+ceil((lon_max-lon_min) / lon_step);
    alt_size = (int)1+ceil((alt_max-alt_min) / alt_step);
    table_size = lat_size * lon_size * alt_size;

    // allocate the tables
    n_data = (double*)calloc(table_size, sizeof(double));
    e_data = (double*)calloc(table_size, sizeof(double));
}

WindGrid::WindGrid(const WindGrid& that) :
    lat_min(that.lat_min),
    lat_max(that.lat_max),
    lat_step(that.lat_step),
    lon_min(that.lon_min),
    lon_max(that.lon_max),
    lon_step(that.lon_step),
    alt_min(that.alt_min),
    alt_max(that.alt_max),
    alt_step(that.alt_step),
    n_data(NULL),
    e_data(NULL),
    lat_size(that.lat_size),
    lon_size(that.lon_size),
    alt_size(that.alt_size),
    table_size(that.table_size) {

    // allocate the tables
    n_data = (double*)calloc(table_size, sizeof(double));
    e_data = (double*)calloc(table_size, sizeof(double));

    // copy the tables
    memcpy(n_data, that.n_data, table_size*sizeof(double));
    memcpy(e_data, that.e_data, table_size*sizeof(double));
}

WindGrid::~WindGrid() {
    if(n_data) {
        free(n_data);
        n_data = NULL;
    }
    if(e_data) {
        free(e_data);
        e_data = NULL;
    }
}

int WindGrid::getIndex(const int& i, const int& j, const int& k) {
    return alt_size * (i*lon_size + j) + k;
}

void WindGrid::setGrid(const double& lat_min,
                       const double& lat_max,
                       const double& lat_step,
                       const double& lon_min,
                       const double& lon_max,
                       const double& lon_step,
                       const double& alt_min,
                       const double& alt_max,
                       const double& alt_step,
                       const double* const n_data,
                       const double* const e_data) {

    this->lat_min = lat_min;
    this->lat_max = lat_max;
    this->lat_step = lat_step;
    this->lon_min = lon_min;
    this->lon_max = lon_max;
    this->lon_step = lon_step;
    this->alt_min = alt_min;
    this->alt_max = alt_max;
    this->alt_step = alt_step;

    lat_size = (int)1+ceil((lat_max-lat_min) / lat_step);
    lon_size = (int)1+ceil((lon_max-lon_min) / lon_step);
    alt_size = (int)1+ceil((alt_max-alt_min) / alt_step);
    table_size = lat_size * lon_size * alt_size;

    if(this->n_data) free(this->n_data);
    if(this->e_data) free(this->e_data);

    // allocate the tables
    this->n_data = (double*)calloc(table_size, sizeof(double));
    this->e_data = (double*)calloc(table_size, sizeof(double));

    // copy the tables
    memcpy(this->n_data, n_data, table_size*sizeof(double));
    memcpy(this->e_data, e_data, table_size*sizeof(double));
}

void WindGrid::getWind(const double& lat,
                       const double& lon,
                       const double& alt,
                       double* const wind_north,
                       double* const wind_east) {

    // alt is the inner dimension (fastest changing)
    // lon is the middle dimension
    // lat is the outer dimension (slowest changing)

    // clamp values between max and min
    double _lat = lat;
    double _lon = lon;
    double _alt = alt;
    if(_lat < lat_min) _lat = lat_min;
    if(_lat > lat_max) _lat = lat_max;
    if(_lon < lon_min) _lon = lon_min;
    if(_lon > lon_max) _lon = lon_max;
    if(_alt < alt_min) _alt = alt_min;
    if(_alt > alt_max) _alt = alt_max;

    // do trilinear interpolation to get the values at lat,lon,alt
    double i = (_lat - lat_min) / lat_step;
    double j = (_lon - lon_min) / lon_step;
    double k = (_alt - alt_min) / alt_step;
    double i_hi = ceil(i);
    double j_hi = ceil(j);
    double k_hi = ceil(k);
    double i_lo = floor(i);
    double j_lo = floor(j);
    double k_lo = floor(k);

    double lat_low  = lat_min+i_lo*lat_step, lat_interp = (_lat-lat_low)/lat_step;
    double lon_low  = lon_min+j_lo*lon_step, lon_interp = (_lon-lon_low)/lon_step;
    double alt_low  = alt_min+k_lo*alt_step, alt_interp = (_alt-alt_low)/alt_step;

    // array indices the eight vertices of a grid cell voxel
    int index_000 = getIndex(i_lo, j_lo, k_lo);
    int index_001 = getIndex(i_lo, j_lo, k_hi);
    int index_010 = getIndex(i_lo, j_hi, k_lo);
    int index_011 = getIndex(i_lo, j_hi, k_hi);
    int index_100 = getIndex(i_hi, j_lo, k_lo);
    int index_101 = getIndex(i_hi, j_lo, k_hi);
    int index_110 = getIndex(i_hi, j_hi, k_lo);
    int index_111 = getIndex(i_hi, j_hi, k_hi);

    double n_00 = n_data[index_000] + (n_data[index_100] - n_data[index_000])*lat_interp;
    double n_10 = n_data[index_010] + (n_data[index_110] - n_data[index_010])*lat_interp;
    double n_01 = n_data[index_001] + (n_data[index_101] - n_data[index_001])*lat_interp;
    double n_11 = n_data[index_011] + (n_data[index_111] - n_data[index_011])*lat_interp;
    double e_00 = e_data[index_000] + (e_data[index_100] - e_data[index_000])*lat_interp;
    double e_10 = e_data[index_010] + (e_data[index_110] - e_data[index_010])*lat_interp;
    double e_01 = e_data[index_001] + (e_data[index_101] - e_data[index_001])*lat_interp;
    double e_11 = e_data[index_011] + (e_data[index_111] - e_data[index_011])*lat_interp;

    double n_0 = n_00 + (n_10 - n_00)*lon_interp;
    double n_1 = n_01 + (n_11 - n_01)*lon_interp;
    double e_0 = e_00 + (e_10 - e_00)*lon_interp;
    double e_1 = e_01 + (e_11 - e_01)*lon_interp;

    double n = n_0 + (n_1-n_0)*alt_interp;
    double e = e_0 + (e_1-e_0)*alt_interp;


    *wind_north = n;
    *wind_east = e;
}



/*TODO:PARIKSHIT ADDER
 * Get maximum magnitude of wind in the entire windfield.
 */

double WindGrid::getMaxWind(){
	double maxwind = 0.0;
	for (unsigned int i=0;i<this->table_size;++i){
		double magw = sqrt(e_data[i]*e_data[i]+n_data[i]*n_data[i])*KnotsToFeetPerSec;
		if (magw>=maxwind)
			maxwind = magw;
	}
	return maxwind;
}

/*
 * Write a simple HDF5 dataset to the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int write_wind_grid_hdf5(const string& hdf5_file,
                         const WindGrid& grid) {
    // create a new HDF5 file using default properties
    hid_t file_id = H5Fcreate(hdf5_file.c_str(),
            H5F_ACC_TRUNC,
            H5P_DEFAULT,
            H5P_DEFAULT);

    // create the HDF5 dataspaces
    hsize_t dims[3] = {grid.lat_size, grid.lon_size, grid.alt_size};
    hid_t dataspace_id = H5Screate_simple(3, dims, NULL);

    hsize_t def_dims[1] = {13};
    hid_t def_dataspace_id = H5Screate_simple(1, def_dims, NULL);

    // Define wind grid definition data
    // The contents of data are latitude, logitude, altitude and grid size
    double def_data[13] = {grid.lat_min, grid.lat_max, grid.lat_step,
                           grid.lon_min, grid.lon_max, grid.lon_step,
                           grid.alt_min, grid.alt_max, grid.alt_step,
                           (double)grid.lat_size, (double)grid.lon_size, (double)grid.alt_size,
                           (double)grid.table_size};

    hid_t def_dataset_id = H5Dcreate2(file_id, "/grid_def", H5T_NATIVE_DOUBLE,
            def_dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);

    hid_t n_dataset_id = H5Dcreate2(file_id, "/wind_north", H5T_NATIVE_DOUBLE,
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);

    hid_t e_dataset_id = H5Dcreate2(file_id, "/wind_east", H5T_NATIVE_DOUBLE,
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);

    H5Dwrite(def_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, def_data);
    H5Dwrite(n_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, grid.n_data);
    H5Dwrite(e_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, grid.e_data);

    H5Dclose(def_dataset_id);
    H5Dclose(n_dataset_id);
    H5Dclose(e_dataset_id);

    // close the file
    H5Fclose(file_id);

    return 0;
}

/*
 * TODO:PARIKSHIT ADDER
 * Write a simple HDF5 dataset to the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int write_wind_grid_all_hdf5(const hid_t& file_id,
                         const WindGrid& grid) {

	// create the HDF5 dataspaces
    hsize_t dims[3] = {grid.lat_size, grid.lon_size, grid.alt_size};
    hid_t dataspace_id = H5Screate_simple(3, dims, NULL);

    hsize_t def_dims[1] = {13};
    hid_t def_dataspace_id = H5Screate_simple(1, def_dims, NULL);
    double def_data[13] = {grid.lat_min, grid.lat_max, grid.lat_step,
                           grid.lon_min, grid.lon_max, grid.lon_step,
                           grid.alt_min, grid.alt_max, grid.alt_step,
                           (double)grid.lat_size, (double)grid.lon_size, (double)grid.alt_size,
                           (double)grid.table_size};

    hid_t def_dataset_id = H5Dcreate2(file_id, "/grid_def", H5T_NATIVE_DOUBLE,
            def_dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
    hid_t n_dataset_id = H5Dcreate2(file_id, "/wind_north", H5T_NATIVE_DOUBLE,
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
    hid_t e_dataset_id = H5Dcreate2(file_id, "/wind_east", H5T_NATIVE_DOUBLE,
            dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
            H5P_DEFAULT);
    H5Dwrite(def_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, def_data);
    H5Dwrite(n_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, grid.n_data);
    H5Dwrite(e_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
            H5P_DEFAULT, grid.e_data);
    H5Dclose(def_dataset_id);
    H5Dclose(n_dataset_id);
    H5Dclose(e_dataset_id);

    // close the file
    H5Fclose(file_id);

    return 0;
}

/*
 * Read a simple HDF5 dataset from the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int read_wind_grid_hdf5(const string& hdf5_file,
                        WindGrid* const grid) {

    if(!grid) return -1;

    herr_t status;



    // open the file
    hid_t file_id = H5Fopen(hdf5_file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
    if(file_id < 0) {
        fprintf(stderr, "ERROR: could not open wind HDF5 file %s\n",
                hdf5_file.c_str());
        exit(-1);
    }


    // open the dataset
    hid_t def_dataset_id = H5Dopen2(file_id, "/grid_def", H5P_DEFAULT);
    if(def_dataset_id < 0) {
        fprintf(stderr, "ERROR: could not open grid_def dataset in %s\n",
                hdf5_file.c_str());
        exit(-1);
    }



    hid_t n_dataset_id = H5Dopen2(file_id, "/wind_north", H5P_DEFAULT);
    if(n_dataset_id < 0) {
        fprintf(stderr, "ERROR: could not open wind_north dataset in %s\n",
                hdf5_file.c_str());
        exit(-1);
    }


    hid_t e_dataset_id = H5Dopen2(file_id, "/wind_east", H5P_DEFAULT);
    if(e_dataset_id < 0) {
        fprintf(stderr, "ERROR: could not open wind_east dataset in %s\n",
                hdf5_file.c_str());
        exit(-1);
    }


    // read the datasets
    double grid_def[13];
    status = H5Dread(def_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
                     H5P_DEFAULT, grid_def);

    double* n_data = (double*)calloc(grid_def[12], sizeof(double));
    double* e_data = (double*)calloc(grid_def[12], sizeof(double));


    status = H5Dread(n_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, n_data);
    if(status) {
        fprintf(stderr, "ERROR: could not read wind_north dataset in %s\n",
                hdf5_file.c_str());
        exit(-1);
    }


    status = H5Dread(e_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
             H5P_DEFAULT, e_data);
    if(status) {
        fprintf(stderr, "ERROR: could not read wind_east dataset in %s\n",
                hdf5_file.c_str());
        exit(-1);
    }


    // close the datasets
    H5Dclose(def_dataset_id);
    H5Dclose(n_dataset_id);
    H5Dclose(e_dataset_id);

    // close the file
    H5Fclose(file_id);

    // set the grid data
    grid->setGrid(grid_def[0], grid_def[1], grid_def[2],
                  grid_def[3], grid_def[4], grid_def[5],
                  grid_def[6], grid_def[7], grid_def[8],
                  n_data, e_data);

    free(n_data);
    free(e_data);

    return 0;
}

/*
 * Read a simple HDF5 dataset from the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int read_wind_grid_all_hdf5(const vector<string>& hdf5_paths,
                        vector<WindGrid>* const g_wind_vec) {


	g_wind_vec->clear();

	for (unsigned int i=0;i<hdf5_paths.size();++i){
		string rap_file = hdf5_paths.at(i);
		WindGrid g_wind;
		read_wind_grid_hdf5(rap_file,&g_wind);
		g_wind_vec->push_back(g_wind);
	}


    return 0;
}
