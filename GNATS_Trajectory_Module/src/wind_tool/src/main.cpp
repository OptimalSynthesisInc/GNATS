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
 * main.cpp
 *
 * This program will extract the u-v wind components and the geopotential
 * height parameters from a set of RUC/RAP files in GRIB1 or GRIB2 format.
 * This program will not detect grid dimensions so it is up to the user
 * to ensure that the input data set uses a consistent grid, for example
 * 13km RUC/RAP grids.
 *
 * make sure HDF5 is installed on your system.
 * make sure GRIB API is installed on your system. (see third-party folder)
 *
 * compile with:
 * g++ -g -O3 -fopenmp -I../include -L../lib -o wind_tool rap.cpp main.cpp -lgrib_api -lhdf5
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#include <dirent.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <omp.h>

#include "wind_api.h"

#include "rap.h"
#include "grib_api.h"

#include "hdf5.h"

using namespace std;

typedef struct _grib_filename_t {
  string filename;
  int    hour;
  int    forecast_hour;
} grib_filename_t;

static string g_config_file = "wind_tool.conf";

static string g_outdir = ".";

static map<int, string> g_grib_files;

static double g_lat_min;
static double g_lat_max;
static double g_lat_step;
static double g_lon_min;
static double g_lon_max;
static double g_lon_step;
static double g_alt_min;
static double g_alt_max;
static double g_alt_step;
static string g_grid_definition_file;


/**
 * Print usage
 */
static void print_usage(char* progname) {
  printf("\n");
  printf("Usage: %s [options]\n\n", progname);
  printf("  Option:\n");
  printf("    --config-file=<file>    -f <file>   Configuration file\n");
  printf("    --out-folder=<dir>      -o <dir>    Output folder\n");
  printf("    --help                  -h          Print this message\n");
  printf("\n");
}

/**
 * Parse arguments
 */
static void parse_args(int argc, char* argv[]) {
  int c;
  while(1) {
    int option_index = 2;

    // {name, has_arg, flag, val}
    // has_arg field: no arg==0,
    //                required arg==1
    //                optional arg==2
    static struct option opts[] = {
      {"config-file", 1, 0, 'f'},
      {"out-folder", 1, 0, 'o'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    string optstr = "f:o:h";

    c = getopt_long(argc, argv, optstr.c_str(), opts, &option_index);
    if (c == -1) {
      break;
    }

    switch(c) {
    case 'f':
      g_config_file = optarg;
      break;
    case 'o':
      g_outdir = optarg;
      break;
    case 'h':
    default:
      print_usage(argv[0]);
      exit(0);
    }
  }
}

/**
 * Trim leading and trailing whitespace from a string
 */
static void trim(string& line) {
  if(line.length() < 1) return;
  size_t start = line.find_first_not_of(" \t\n");
  size_t end = line.find_last_not_of(" \t\n")+1;
  if(end > line.length()) { 
    line = "";
    return;
  }

  line = line.substr(start, end-start);
}

/**
 * Parse a config file line
 */
static int parse_config_line(const string& line, 
			      int* const hour, 
			      string* const fname) {
  if(!hour) return -1;
  if(!fname) return -1;
  if(line.length() < 1) return -1;

  size_t comma_pos = line.find_first_of(",");

  string hour_str = line.substr(0, comma_pos);
  string fname_str = line.substr(comma_pos+1);

  trim(hour_str);
  trim(fname_str);

  *hour = atoi(hour_str.c_str());
  *fname = fname_str;

  return 0;
}

static int parse_grid_dim_line(const string& line,
			       double* const value) {
  if(!value) return -1;

  size_t equals_pos = line.find_first_of("=");
  string value_str = line.substr(equals_pos+1);
  trim(value_str);

  if(value_str.length() > 0) {
    *value = atof(value_str.c_str());
  } else {
    return -1;
  }

  return 0;
}

static int parse_grid_definition_name_line(const string& line,
					   string* const value) {
  if(!value) return -1;
  size_t equals_pos = line.find_first_of("=");
  string value_str = line.substr(equals_pos+1);
  trim(value_str);

  if(value_str.length() > 0) {
    *value = value_str;
  } else {
    return -1;
  }

  return 0;  
}

/**
 * Load the config file
 */
static int load_config(const string& config_file,
		       double* const lat_min,
		       double* const lat_max,
		       double* const lat_step,
		       double* const lon_min,
		       double* const lon_max,
		       double* const lon_step,
		       double* const alt_min,
		       double* const alt_max,
		       double* const alt_step,
		       string* const grid_def_file,
		       map<int, string>* const files) {
  if(!lat_min) return -1;
  if(!lat_max) return -1;
  if(!lat_step) return -1;
  if(!lon_min) return -1;
  if(!lon_max) return -1;
  if(!lon_step) return -1;
  if(!alt_min) return -1;
  if(!alt_max) return -1;
  if(!alt_step) return -1;
  if(!files) return -1;

  // open the config file for reading
  ifstream in;
  in.open(config_file.c_str());

  if (!in.is_open()) {
	  fprintf(stderr, "ERROR: could not open configuration file %s\n", config_file.c_str());
	  exit(-1);
  }

  // read the file
  string line = "";
  while(in.good()) {
    getline(in, line);

    // skip blank lines
    if(line.length() < 1) continue;

    // remove any leading or trailing whitespace
    trim(line);

    // skip comment lines starting with '#'
    if(line.at(0) == '#') continue;

    // determine if we are parsing the grid dimensions section
    int pos = line.find_first_of(" =");
    string first_word = line.substr(0, pos);
    trim(first_word);

    if(first_word == "LAT_MIN") {
      parse_grid_dim_line(line, lat_min);
    } else if(first_word == "LAT_MAX") {
      parse_grid_dim_line(line, lat_max);
    } else if(first_word == "LAT_STEP") {
      parse_grid_dim_line(line, lat_step);
    } else if(first_word == "LON_MIN") {
      parse_grid_dim_line(line, lon_min);
    } else if(first_word == "LON_MAX") {
      parse_grid_dim_line(line, lon_max);
    } else if(first_word == "LON_STEP") {
      parse_grid_dim_line(line, lon_step);
    } else if(first_word == "ALT_MIN") {
      parse_grid_dim_line(line, alt_min);
    } else if(first_word == "ALT_MAX") {
      parse_grid_dim_line(line, alt_max);
    } else if(first_word == "ALT_STEP") {
      parse_grid_dim_line(line, alt_step);
    } else if(first_word == "GRID_DEFINITION_FILE") {
      parse_grid_definition_name_line(line, grid_def_file);    
    } else {

      // parse the current line
      int hour;
      string fname;
      parse_config_line(line, &hour, &fname);
      files->insert(pair<int,string>(hour, fname));
    }
  }

    // close the file
    in.close();

	return 0;
}


/*
static int read_hdf5(const string& hdf5_file,
		     int* const rank,
		     hsize_t** const dims,
		     double** const n_data,
		     double** const e_data) {
  herr_t status;

  // open the file
  hid_t file_id = H5Fopen(hdf5_file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  // open the dataset
  hid_t n_dataset_id = H5Dopen2(file_id, "/wind_north", H5P_DEFAULT);
  hid_t e_dataset_id = H5Dopen2(file_id, "/wind_east", H5P_DEFAULT);
  
  // use n dataset to compute the dataspace rank and dimension
  hid_t dataspace_id = H5Dget_space(n_dataset_id);

  // query dataspace rank and dimension
  *rank = H5Sget_simple_extent_ndims(dataspace_id);
  *dims = (hsize_t*)calloc(*rank, sizeof(hsize_t));
  H5Sget_simple_extent_dims(dataspace_id, *dims, NULL);

  // allocate the data array
  size_t data_size = 1;
  for(int i=0; i<*rank; ++i) {
    data_size *= (*dims)[i];
  }
  *n_data = (double*)calloc(data_size, sizeof(double));
  *e_data = (double*)calloc(data_size, sizeof(double));

  // read the datasets
  status = H5Dread(n_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
		   H5P_DEFAULT, *n_data);
  status = H5Dread(e_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
		   H5P_DEFAULT, *e_data);

  // close the datasets
  status = H5Dclose(n_dataset_id);
  status = H5Dclose(e_dataset_id);

  // close the file
  status = H5Fclose(file_id);

  return status;
}
*/

/*
static int write_grid_hdf5(const string& hdf5_file,
			   const double& lat_min,
			   const double& lat_max,
			   const double& lat_step,
			   const double& lon_min,
			   const double& lon_max,
			   const double& lon_step,
			   const double& alt_min,
			   const double& alt_max,
			   const double& alt_step) {

  int scalar_rank = 1;
  hsize_t scalar_dims[1] = {1};

  hid_t file_id = H5Fcreate(hdf5_file.c_str(),
			    H5F_ACC_TRUNC,
			    H5P_DEFAULT,
			    H5P_DEFAULT);

  hid_t dataspace_id = H5Screate_simple(scalar_rank, scalar_dims, NULL);

  hid_t lat_min_id = H5Dcreate2(file_id, "/lat_min", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t lat_max_id = H5Dcreate2(file_id, "/lat_max", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t lat_step_id = H5Dcreate2(file_id, "/lat_step", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t lon_min_id = H5Dcreate2(file_id, "/lon_min", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t lon_max_id = H5Dcreate2(file_id, "/lon_max", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t lon_step_id = H5Dcreate2(file_id, "/lon_step", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t alt_min_id = H5Dcreate2(file_id, "/alt_min", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t alt_max_id = H5Dcreate2(file_id, "/alt_max", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  hid_t alt_step_id = H5Dcreate2(file_id, "/alt_step", H5T_NATIVE_DOUBLE,
				dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				H5P_DEFAULT);
  H5Dwrite(lat_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lat_min);
  H5Dwrite(lat_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lat_max);
  H5Dwrite(lat_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lat_step);
  H5Dwrite(lon_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lon_min);
  H5Dwrite(lon_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lon_max);
  H5Dwrite(lon_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &lon_step);
  H5Dwrite(alt_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &alt_min);
  H5Dwrite(alt_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &alt_max);
  H5Dwrite(alt_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT,
	   &alt_step);
  H5Dclose(lat_min_id);
  H5Dclose(lat_max_id);
  H5Dclose(lat_step_id);
  H5Dclose(lon_min_id);
  H5Dclose(lon_max_id);
  H5Dclose(lon_step_id);
  H5Dclose(alt_min_id);
  H5Dclose(alt_max_id);
  H5Dclose(alt_step_id);
  H5Fclose(file_id);
  return 0;
}
*/

int write_grid_hdf5(const string& hdf5_file,
		   const double& lat_min,
		   const double& lat_max,
		   const double& lat_step,
		   const double& lon_min,
		   const double& lon_max,
		   const double& lon_step,
		   const double& alt_min,
		   const double& alt_max,
		   const double& alt_step) {
    WindGrid grid(lat_min, lat_max, lat_step,
                  lon_min, lon_max, lon_step,
                  alt_min, alt_max, alt_step);

    return write_wind_grid_hdf5(hdf5_file, grid);
}

/*
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

  herr_t status;

  // open the file
  hid_t file_id = H5Fopen(hdf5_file.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

  // open the dataset
  hid_t lat_min_id = H5Dopen2(file_id, "/lat_min", H5P_DEFAULT);
  hid_t lat_max_id = H5Dopen2(file_id, "/lat_max", H5P_DEFAULT);
  hid_t lat_step_id = H5Dopen2(file_id, "/lat_step", H5P_DEFAULT);
  hid_t lon_min_id = H5Dopen2(file_id, "/lon_min", H5P_DEFAULT);
  hid_t lon_max_id = H5Dopen2(file_id, "/lon_max", H5P_DEFAULT);
  hid_t lon_step_id = H5Dopen2(file_id, "/lon_step", H5P_DEFAULT);
  hid_t alt_min_id = H5Dopen2(file_id, "/alt_min", H5P_DEFAULT);
  hid_t alt_max_id = H5Dopen2(file_id, "/alt_max", H5P_DEFAULT);
  hid_t alt_step_id = H5Dopen2(file_id, "/alt_step", H5P_DEFAULT);

  if(lat_min) {
    H5Dread(lat_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lat_min);
  }
  if(lat_max) {
    H5Dread(lat_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lat_max);
  }
  if(lat_step) {
    H5Dread(lat_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lat_step);
  }
  if(lon_min) {
    H5Dread(lon_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lon_min);
  }
  if(lon_max) {
    H5Dread(lon_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lon_max);
  }
  if(lon_step) {
    H5Dread(lon_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, lon_step);
  }
  if(alt_min) {
    H5Dread(alt_min_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, alt_min);
  }
  if(alt_max) {
    H5Dread(alt_max_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, alt_max);
  }
  if(alt_step) {
    H5Dread(alt_step_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, 
	    H5P_DEFAULT, alt_step);
  }

  H5Dclose(lat_min_id);
  H5Dclose(lat_max_id);
  H5Dclose(lat_step_id);
  H5Dclose(lon_min_id);
  H5Dclose(lon_max_id);
  H5Dclose(lon_step_id);
  H5Dclose(alt_min_id);
  H5Dclose(alt_max_id);
  H5Dclose(alt_step_id);

  H5Fclose(file_id);

  return 0;
}
*/

/*
 * Write a simple HDF5 dataset to the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
/*
static int write_hdf5(const string& hdf5_file, 
		      const int& rank,
		      const hsize_t* const dims,
		      const double* const n_data,
		      const double* const e_data) {

  // create a new HDF5 file using default properties
  hid_t file_id = H5Fcreate(hdf5_file.c_str(),
			    H5F_ACC_TRUNC,
			    H5P_DEFAULT,
			    H5P_DEFAULT);

  // create the HDF5 dataspace
  hid_t dataspace_id = H5Screate_simple(rank, dims, NULL);

  hid_t n_dataset_id = H5Dcreate2(file_id, "/wind_north", H5T_NATIVE_DOUBLE,
				  dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				  H5P_DEFAULT);
  hid_t e_dataset_id = H5Dcreate2(file_id, "/wind_east", H5T_NATIVE_DOUBLE,
				  dataspace_id, H5P_DEFAULT, H5P_DEFAULT,
				  H5P_DEFAULT);
  H5Dwrite(n_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, n_data);
  H5Dwrite(e_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,
		    H5P_DEFAULT, e_data);
  H5Dclose(n_dataset_id);
  H5Dclose(e_dataset_id);

  // close the file
  H5Fclose(file_id);

  return 0;
}
*/

static int build_wind_grid(const GribData& grib, WindGrid* const grid) {
    if(!grid) return -1;
    int index = 0;
    for(double lat=grid->lat_min; lat<=grid->lat_max; lat+=grid->lat_step) {
      for(double lon=grid->lon_min; lon<=grid->lon_max; lon+=grid->lon_step) {
        for(double alt=grid->alt_min; alt<=grid->alt_max; alt+=grid->alt_step) {
          get_wind_components(grib, lat, lon, alt,
                      &grid->n_data[index], &grid->e_data[index],
                      KNOTS);
          ++index;
        }
      }
    }
    return 0;
}

static double delta_timeval(struct timeval& tvend, struct timeval& tvstart) {
  double dsec = tvend.tv_sec - tvstart.tv_sec;
  double dusec = tvend.tv_usec - tvstart.tv_usec;
  return (dsec + dusec / 1000000.); // seconds
}


/**
 * Program entry
 */
int main(int argc, char* argv[]) {
    parse_args(argc, argv);

    printf("*****************************************************************************\n");
    printf("*                            GNATS Wind Utility                              *\n");
    printf("*****************************************************************************\n");

    printf("\n");
    printf("  Config file:    %s\n", g_config_file.c_str());
    printf("  Output folder:  %s\n", g_outdir.c_str());
    printf("\n");

    struct timeval t_start, t_end;
    gettimeofday(&t_start, NULL);

    // Load the config file
    load_config(g_config_file,
	      &g_lat_min, &g_lat_max, &g_lat_step,
	      &g_lon_min, &g_lon_max, &g_lon_step,
	      &g_alt_min, &g_alt_max, &g_alt_step,
	      &g_grid_definition_file, &g_grib_files);

    struct stat info;

    if ( stat( g_outdir.c_str(), &info ) != 0 ) {
  	    // make sure the output directory exists and create if needed
	    // we'll use the system's mkdir for this.  we may need a more
	    // cross platform way of doing this.
	    string mkdir_str = "mkdir -p " + g_outdir;
	    int err = system(mkdir_str.c_str());
	    if (err) {
		    fprintf(stderr, "ERROR: could not create output directory %s\n",
				  g_outdir.c_str());
		    exit(-1);
	    }
    }

    string out_dir = g_outdir;

    string grid_outfile = out_dir + "/" + g_grid_definition_file;

    printf("  Grid definition file: %s\n", grid_outfile.c_str());

	// Write the grid definition file
  	write_grid_hdf5(grid_outfile,
		  g_lat_min, g_lat_max, g_lat_step,
		  g_lon_min, g_lon_max, g_lon_step,
		  g_alt_min, g_alt_max, g_alt_step);

    // compute table sizes
    size_t lat_size = (int)1+ceil((g_lat_max-g_lat_min) / g_lat_step);
    size_t lon_size = (int)1+ceil((g_lon_max-g_lon_min) / g_lon_step);
    size_t alt_size = (int)1+ceil((g_alt_max-g_alt_min) / g_alt_step);

    // iterate over each entry and load the grib data
    map<int,string>::iterator iter;
//  map<int, GribData> grib_data;
  
    //TODO:PARIKSHIT ADDER
    unsigned int num_time_steps = g_grib_files.size();

    hsize_t t_dims[1] = {num_time_steps};
    hid_t def_dataspace_id = H5Screate_simple(1, t_dims, NULL);

    for (iter = g_grib_files.begin(); iter != g_grib_files.end(); ++iter) {
        printf("  hour: %d,  file: %s\n", iter->first, iter->second.c_str());

        GribData grib;

#ifndef _INC__MINGW_H
        load_grib(iter->second, &grib);
#else
	    load_grib_v2_win(iter->second, &grib);
#endif

        int num_levels = grib.h_data.size();

        printf("\n    GRIB edition: %d\n", grib.grib_edition);
        printf("\n    Num levels: %d\n", num_levels);
        printf("    Grid size: %d, %d\n", grib.grid_ni, grib.grid_nj);
        printf("\n");
        printf("    Output Table Bounds:\n");
        printf("      Latitude -  min: %f, max: %f, step: %f\n",
	       g_lat_min, g_lat_max, g_lat_step);
        printf("      Longitude - min: %f, max: %f, step: %f\n",
	       g_lon_min, g_lon_max, g_lon_step);
        printf("      Altitude -  min: %f, max: %f, step: %f\n",
	       g_alt_min, g_alt_max, g_alt_step);
        printf("      Latitude cells:  %ld\n", lat_size);
        printf("      Longitude cells: %ld\n", lon_size);
        printf("      Altitude cells:  %ld\n\n", alt_size);

        // Declare grid
        WindGrid grid(g_lat_min, g_lat_max, g_lat_step,
                  g_lon_min, g_lon_max, g_lon_step,
                  g_alt_min, g_alt_max, g_alt_step);

    // build the wind tables for the current file
        build_wind_grid(grib, &grid);

        int slash = iter->second.find_last_of("/");
        string basename = iter->second.substr(slash+1);

        // write the output for the current hour
        int dot = basename.find_last_of(".");
        string ofname = basename.substr(0, dot)+".h5";
        string ofpath = out_dir + "/" + ofname;

        write_wind_grid_hdf5(ofpath, grid);
    }

    gettimeofday(&t_end, NULL);

    double total_time = delta_timeval(t_end, t_start);
    printf("  Total time:  %f sec\n", total_time);

    printf("\nGood bye.\n");

    return 0;
}
