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
********************************************************************************
 COMPUTATIONAL APPLIANCE FOR RAPID PREDICTION OF AIRCRAFT TRAJECTORIES (CARPAT)
          Copyright 2010 by Optimal Synthesis Inc. All rights reserved



SBIR Rights (FAR 52.227-20) Notice: Contract No. NNX11CA08C, Dated June 1, 2011.
                       Contract End Date May 31, 2013
                   Software Release Date August 16, 2011

For a period of 4 years after acceptance of all items to be delivered under this
contract, the Government agrees to use these data for Government purposes only,
and they shall not be disclosed outside the Government (including disclosure for
procurement purposes) during such period without permission of the Contractor,
except that, subject to the foregoing use and disclosure prohibitions, such data
may be disclosed for use by support Contractors.  After the aforesaid 4-year
period the Government has a royalty-free license to use, and to authorize others
to use on its behalf, these data for Government purposes, but is relieved of all
disclosure prohibitions and assumes no liability for unauthorized use of these
data by third parties.  This Notice shall be affixed to any reproductions of
these data, in whole or in part.
********************************************************************************
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>

#include "rap.h"
#include "util_string.h"

#define alat1 16.281
#define elon1 -126.138

//#define dx 13545.087
#define dx 13545.000000
#define elonv 265
#define alatan 25

// wind rotation constant, = 1 for polar stereo and sin(alatan) for lambert conformal
#define rotConst sin(alatan*DegToRad)

#define rEarth 6371229
#define rEarthByDx rEarth/dx
#define R0 6356766 //for geopotential to geometric conversion


using namespace std;

#if 0
static double uWindArrayMeterPerSec[NumHybridLevels][NiOfWindComp*NjOfWindComp];
static double vWindArrayMeterPerSec[NumHybridLevels][NiOfWindComp*NjOfWindComp];
static double geometricAltitudeMeter[NumHybridLevels][NiOfWindComp*NjOfWindComp];
static double temperatureArrayK[NumHybridLevels][NiOfWindComp*NjOfWindComp];
static double pressureArrayPa[NumHybridLevels][NiOfWindComp*NjOfWindComp];

static int hemisphere;
static double an;
static double elonvr;
static double cosltn;
static double poleI;
static double poleJ;
#endif

////////////////////////////////////////////////////////////////////////////////
// Thread-safe versions of the functions
static int initializeGribData(GribData* const grib) {

  if(!grib) return -1;
  double alatn1 = alatan*DegToRad;
  double elon1l;
  double ala1 = alat1*DegToRad;
  double rmll;
  double elo1;
  double arg;

  // hemisphere = 1 for northern hemisphere; = -1 for southern
  if (alatan > 0)
    grib->hemisphere = 1;
  else
    grib->hemisphere = -1;
  
  grib->an = grib->hemisphere * sin(alatn1);
  grib->cosltn = cos(alatn1);
  
  grib->elonvr = elonv*DegToRad;
  
  elon1l = elon1;
  if ((elon1 - elonv) > 180)
    elon1l = elon1 - 360;
  if ((elon1 - elonv) < 180)
    elon1l = elon1 + 360;
  
  rmll = rEarthByDx * (pow(grib->cosltn, 1-grib->an) * 
		       pow(1+grib->an, grib->an)) * 
    pow((cos(ala1)/(1+grib->hemisphere*sin(ala1))), grib->an) / grib->an;
  
  elo1 = elon1l * DegToRad;
  arg = grib->an * (elo1 - grib->elonvr);
  grib->poleI = 1 - grib->hemisphere * rmll * sin(arg);
  grib->poleJ = 1 + rmll * cos(arg);   
  
  return 0;
}

// Converts lat and lon to its corresponding grid coordinate (xI and xJ)
// where i represents latitude and j represents longitude
static int convertGribCoordinate(const GribData& grib, double alat, double elon,
			  double* xI, double* xJ) {

  if(!xI) return -1;
  if(!xJ) return -1;

  double elonl;
  double ala;
  double rm;
  double elo;
  double arg;
  
  elonl = elon;
  if ((elon - elonv) > 180)
    elonl = elon - 360;
  if ((elon - elonv) < 180)
    elonl = elon + 360;
  
  ala = alat * DegToRad;
  rm = rEarthByDx * (pow(grib.cosltn, 1-grib.an) * pow(1+grib.an, grib.an)) * 
    pow((cos(ala)/(1+grib.hemisphere*sin(ala))), grib.an) / grib.an;
  
  elo = elonl * DegToRad;
  
  arg = grib.an * (elo - grib.elonvr);
  
  *xI = grib.poleI + grib.hemisphere * rm * sin(arg);
  *xJ = grib.poleJ - rm * cos(arg);
  
  if (*xI < 1) {
    *xI = 1;
    //printf("xI is out of the border!!!\n");
  }
  if (*xJ < 1) {
    *xJ = 1;
    //printf("xJ is out of the border!!!\n");
  }
  if (*xJ > grib.grid_nj)
    *xJ = grib.grid_nj;
  //if (*xI > 451 && *xI < 452)
  if (*xI > grib.grid_ni)
    *xI = grib.grid_ni;
  
  return 0;
}


/* This function uses bilinear interpolation to find the actual wind component of the point
 *
 *           D               C
 *        i2  #---------------#
 *            |       |       |
 *            |       |       |
 *         i  |-------#-------|
 *            |       |       |
 *            |       |       |
 *            |       |       |
 *            |       |       |
 *        i1  #---------------#
 *           A        j        B
 *            j1              j2
 *
 * The values for the wind components are a 1D arrray.
 * To convert from the grid coordinate to the array index:
 *    - Make sure to decrement the grid coordinate by 1 because the coordinate
 *      starts from 1, while the array index starts from 0
 *    - index = j * NiOfWindComp + i
 */
static double bilinearInterpolation(double i, double j, int ni, const double* arr) {
    int i1, i2, j1, j2;
    double Va = 0;
    double Vb = 0;
    double Vc = 0;
    double Vd = 0;

    double Vj_i1 = 0;
    double Vj_i2 = 0;
    double Vj_i = 0;

    i = i - 1; //minus 1 because the index of the array starts from 0
    j = j - 1;

    i1 = (int)floor(i);
    i2 = (int)ceil(i);
    j1 = (int)floor(j);
    j2 = (int)ceil(j);

    if(i1 == i2 && j1 == j2)
        return arr[j1 * ni + i1];
    if(j1 == j2) {
        Vj_i1 = arr[j1 * ni + i1];
        Vj_i2 = arr[j1 * ni + i2];
    }
    else {
        Va = arr[j1 * ni + i1];
        Vb = arr[j2 * ni + i1];

        Vj_i1 = Va + (Vb-Va)/(j2-j1)*(j-j1);

        if(i1 == i2)
            return Vj_i1;

        Vc = arr[j2 * ni + i2];
        Vd = arr[j1 * ni + i2];

        Vj_i2 = Vd + (Vc-Vd)/(j2-j1)*(j-j1);

    }

    Vj_i = Vj_i1 + (Vj_i2-Vj_i1)/(i2-i1)*(i-i1);

    return Vj_i;

}

static void convertToTrueNorth (double elon, double* uWind, double* vWind)
{
    double angle;

    if(elonv > 180)
        angle = rotConst * (elon - (elonv-360)) * DegToRad;
    else
        angle = rotConst * (elon - elonv) * DegToRad;

    double sinx2 = sin(angle);
    double cosx2 = cos(angle);

    double uTrueNorth = cosx2 * (*uWind) + sinx2 * (*vWind);
    double vTrueNorth = -sinx2 * (*uWind) + cosx2 * (*vWind);

    *uWind = uTrueNorth;
    *vWind = vTrueNorth;
}

static int handle_u_data(grib_handle* h, GribData* grib_data, long level) {

  map< long, vector<double> >::iterator u_iter;

  // handle u data
  u_iter = grib_data->u_data.find(level);
  if(u_iter == grib_data->u_data.end()) {
    grib_data->u_data.insert(pair< long, vector<double> >(level,
							  vector<double>()));
  }
  
  // extract the number of values, allocate the values buffer
  // copy the values to the level vector in grib_data, and
  // free the values buffer.
  size_t num_values = 0;
  GRIB_CHECK(grib_get_size(h, "values", &num_values), 0);
  double* values = (double*)calloc(num_values, sizeof(double));
  GRIB_CHECK(grib_get_double_array(h, "values", values, &num_values), 0);
  grib_data->u_data[level].assign(values, values+num_values);  
  free(values);

  return 0;
}

static int handle_u_data_win(GribData* grib_data, long level, long num_values, string grib_file_path_name) {
	map< long, vector<double> >::iterator u_iter;

	// handle u data
	u_iter = grib_data->u_data.find(level);
	if (u_iter == grib_data->u_data.end()) {
	  grib_data->u_data.insert(pair< long, vector<double> >(level, vector<double>()));
	}

    stringstream ss_command;
    ss_command.str("");
    ss_command.clear();

    ss_command << "wgrib2_windows_x64\\wgrib2 ";
    ss_command << grib_file_path_name;
    ss_command << " -match \"UGRD:";
    ss_command << level;
    ss_command << " mb\" -text tmp_rap_u.txt";

    FILE* wgrib2_input = popen(ss_command.str().c_str(), "r");
    if (wgrib2_input == NULL) exit(2);

    std::ifstream in;
    in.open("tmp_rap_u.txt");

    double* values = (double*)calloc(num_values, sizeof(double));

    string read_line;

	int i = -1;
	while ((i < num_values) && (in.good())) {
		read_line = "";
		getline(in, read_line);

		read_line = trim(read_line);

		if ((-1 < i) && (0 < read_line.length())) {
			values[i] = atof(read_line.c_str());
		}

		i++;
	}

	in.close();

	remove("tmp_rap_u.txt");

    /* close the wgrib2 program */
    int err = pclose(wgrib2_input);

    grib_data->u_data[level].assign(values, values+num_values);

    free(values);

    return 0;
}

static int handle_v_data(grib_handle* h, GribData* grib_data, long level) {

  map< long, vector<double> >::iterator v_iter;

  // handle v data
  v_iter = grib_data->v_data.find(level);
  if(v_iter == grib_data->v_data.end()) {
    grib_data->v_data.insert(pair< long, vector<double> >(level,
							  vector<double>()));
  }
  
  // extract the number of values, allocate the values buffer
  // copy the values to the level vector in grib_data, and
  // free the values buffer.
  size_t num_values = 0;
  GRIB_CHECK(grib_get_size(h, "values", &num_values), 0);
  double* values = (double*)calloc(num_values, sizeof(double));
  GRIB_CHECK(grib_get_double_array(h, "values", values, &num_values), 0);
  grib_data->v_data[level].assign(values, values+num_values); 
  free(values);

  return 0;
}

static int handle_v_data_win(GribData* grib_data, long level, long num_values, string grib_file_path_name) {
	map< long, vector<double> >::iterator v_iter;

	// handle v data
	v_iter = grib_data->v_data.find(level);
	if (v_iter == grib_data->v_data.end()) {
	  grib_data->v_data.insert(pair< long, vector<double> >(level, vector<double>()));
	}

	stringstream ss_command;
	ss_command.str("");
	ss_command.clear();

	ss_command << "wgrib2_windows_x64\\wgrib2 ";
	ss_command << grib_file_path_name;
	ss_command << " -match \"VGRD:";
	ss_command << level;
	ss_command << " mb\" -text tmp_rap_v.txt";

	FILE* wgrib2_input = popen(ss_command.str().c_str(), "r");
	if (wgrib2_input == NULL) exit(2);

	std::ifstream in;
	in.open("tmp_rap_v.txt");

	double* values = (double*)calloc(num_values, sizeof(double));

	string read_line;

	int i = -1;
	while ((i < num_values) && (in.good())) {
		read_line = "";
		getline(in, read_line);

		read_line = trim(read_line);

		if ((-1 < i) && (0 < read_line.length())) {
			values[i] = atof(read_line.c_str());
		}

		i++;
	}

	in.close();

	remove("tmp_rap_v.txt");

	/* close the wgrib2 program */
	int err = pclose(wgrib2_input);

	grib_data->v_data[level].assign(values, values+num_values);

	free(values);

	return 0;
}

static int handle_h_data(grib_handle* h, GribData* grib_data, long level) {

  map< long, vector<double> >::iterator h_iter;

  // handle h data
  h_iter = grib_data->h_data.find(level);
  if(h_iter == grib_data->h_data.end()) {
    grib_data->h_data.insert(pair< long, vector<double> >(level,
							  vector<double>()));
  }
  
  // extract the number of values, allocate the values buffer
  // copy the values to the level vector in grib_data, and
  // free the values buffer.
  size_t num_values = 0;
  GRIB_CHECK(grib_get_size(h, "values", &num_values), 0);
  double* values = (double*)calloc(num_values, sizeof(double));
  GRIB_CHECK(grib_get_double_array(h, "values", values, &num_values), 0);
  grib_data->h_data[level].assign(values, values+num_values);  
  free(values);

  return 0;
}

static int handle_h_data_win(GribData* grib_data, long level, long num_values, string grib_file_path_name) {
	map< long, vector<double> >::iterator h_iter;

	// handle h data
	h_iter = grib_data->h_data.find(level);
	if (h_iter == grib_data->h_data.end()) {
	grib_data->h_data.insert(pair< long, vector<double> >(level, vector<double>()));
	}

	stringstream ss_command;
	ss_command.str("");
	ss_command.clear();

	ss_command << "wgrib2_windows_x64\\wgrib2 ";
	ss_command << grib_file_path_name;
	ss_command << " -match \"HGT:";
	ss_command << level;
	ss_command << " mb\" -text tmp_rap_h.txt";

	FILE* wgrib2_input = popen(ss_command.str().c_str(), "r");
	if (wgrib2_input == NULL) exit(2);

	std::ifstream in;
	in.open("tmp_rap_h.txt");

	double* values = (double*)calloc(num_values, sizeof(double));

	string read_line;

	int i = -1;
	while ((i < num_values) && (in.good())) {
		read_line = "";
		getline(in, read_line);

		read_line = trim(read_line);

		if ((-1 < i) && (0 < read_line.length())) {
			values[i] = atof(read_line.c_str());
		}

		i++;
	}

	in.close();

	remove("tmp_rap_h.txt");

	/* close the wgrib2 program */
	int err = pclose(wgrib2_input);

	grib_data->h_data[level].assign(values, values+num_values);

	free(values);

	return 0;
}

#if 0
static int handle_t_data(grib_handle* h, GribData* grib_data, long level) {

  map< long, vector<double> >::iterator t_iter;

  // handle t data
  t_iter = grib_data->t_data.find(level);
  if(t_iter == grib_data->t_data.end()) {
    grib_data->t_data.insert(pair< long, vector<double> >(level,
							  vector<double>()));
  }
  
  // extract the number of values, allocate the values buffer
  // copy the values to the level vector in grib_data, and
  // free the values buffer.
  size_t num_values = 0;
  GRIB_CHECK(grib_get_size(h, "values", &num_values), 0);
  double* values = (double*)calloc(num_values, sizeof(double));
  GRIB_CHECK(grib_get_double_array(h, "values", values, &num_values), 0);
  grib_data->t_data[level].assign(values, values+num_values);  
  free(values);

  return 0;
}

static int handle_p_data(grib_handle* h, GribData* grib_data, long level) {

  map< long, vector<double> >::iterator p_iter;

  // handle h data
  p_iter = grib_data->p_data.find(level);
  if(p_iter == grib_data->p_data.end()) {
    grib_data->p_data.insert(pair< long, vector<double> >(level,
							  vector<double>()));
  }
  
  // extract the number of values, allocate the values buffer
  // copy the values to the level vector in grib_data, and
  // free the values buffer.
  size_t num_values = 0;
  GRIB_CHECK(grib_get_size(h, "values", &num_values), 0);
  double* values = (double*)calloc(num_values, sizeof(double));
  GRIB_CHECK(grib_get_double_array(h, "values", values, &num_values), 0);
  grib_data->p_data[level].assign(values, values+num_values);  
  free(values);

  return 0;
}
#endif

/**
 * Load a grib file into a GribData object
 */
int load_grib(const string& grib_file, GribData* const grib_data) {
	// turn on multi message support
	grib_multi_support_on(0);

	// open the file for reading
	FILE* f = fopen(grib_file.c_str(), "r");
	if (!f) {
	  fprintf(stderr, "ERROR: could not open GRIB file %s\n", grib_file.c_str());
	  return -1;
	}

	int err = 0;
	grib_handle* h = NULL;

	int grib_ed = 0;

	// grib1 keys
	long param_id = -1;
	long table_ver = -1;

	// grib2 keys
	//long discipline = -1;
	long param_cat = -1;
	long param_num = -1;
	long level = -1;

	// test for grib edition 1 or 2
	h = grib_handle_new_from_file(0, f, &err);

	if (h) {
		GRIB_CHECK(err, 0);

		// test for GRIB2 using parameterCategory
		grib_get_long(h, "parameterCategory", &param_cat);

		if (param_cat > -1) {
			grib_ed = 2;
		}

		// if we couldn't detect GRIB2, then test for GRIB1 using
		// inidicatorOfParameter
		if (grib_ed != 2) {
			grib_get_long(h, "indicatorOfParameter", &param_id);
			if(param_id > -1) {
				grib_ed = 1;
			}
		}

		// if we couldn't detect GRIB1 or GRIB2, then close the file and
		// return error.
		if ( !(grib_ed == 1 || grib_ed == 2) ) {
			fprintf(stderr, "ERROR: could not determine GRIB edition of file %s\n", grib_file.c_str());

			if (f) {
				fclose(f);
			}

			return -1;
		}
	} else {
		fprintf(stderr, "ERROR: could not obtain grib handle for file %s\n",
		grib_file.c_str());

		if (f) {
		  fclose(f);
		}

		return -1;
	}

	// set the grib edition in the output object
	grib_data->grib_edition = grib_ed;

	// determine the grid dimensions.  we'll need this to compute the
	// array index
	if (h) {
		long ni,nj;
		GRIB_CHECK(grib_get_long(h, "Ni", &ni), 0);
		GRIB_CHECK(grib_get_long(h, "Nj", &nj), 0);
		grib_data->grid_ni = ni;
		grib_data->grid_nj = nj;
		// done with the current handle, delete it.
		grib_handle_delete(h);
	} else {
		fprintf(stderr, "ERROR: could not determine grid dimensions for file %s\n",
				grib_file.c_str());
		if (f) {
			fclose(f);
		}

		return -1;
	}

	// close and re-open the file to reset the file pointer after having
	// performed the grib edition check.
	if (f) fclose(f);
	f = fopen(grib_file.c_str(), "r");
	if (!f) {
		fprintf(stderr, "ERROR: could not re-open GRIB file %s\n",
		grib_file.c_str());

		return -1;
	}

	while ((h = grib_handle_new_from_file(0, f, &err)) != NULL) {
		char levelType[255] = {0,};
		size_t sz =255;
		GRIB_CHECK(err, 0);

		// if the file is GRIB1, use param_id and table_ver keys.
		// if the file is GRIB2, use discipline, param_cat, and param_num
		if (grib_ed == 1) {
			GRIB_CHECK(grib_get_long(h, "indicatorOfParameter", &param_id), 0);
			GRIB_CHECK(grib_get_long(h, "table2Version", &table_ver), 0);
			GRIB_CHECK(grib_get_long(h, "level", &level), 0);

			if (param_id == 33 /*&& table_ver == 2*/) {
				// handle u data (GRIB1)
				handle_u_data(h, grib_data, level);
			} else if(param_id == 34 /*&& table_ver == 2*/) {
				// handle v data (GRIB1)
				handle_v_data(h, grib_data, level);
			} else if(param_id == 7 /*&& table_ver == 2*/) {
				// handle h data (GRIB1)
				handle_h_data(h, grib_data, level);
			}
		} else if (grib_ed == 2) {
			GRIB_CHECK(grib_get_long(h, "parameterCategory", &param_cat), 0);
			GRIB_CHECK(grib_get_long(h, "parameterNumber", &param_num), 0);
			GRIB_CHECK(grib_get_long(h, "level", &level), 0);
			//TODO: CORRECTED RAP FILE TYPE OF LEVEL
			GRIB_CHECK(grib_get_string(h, "typeOfLevel", levelType,&sz), 0 );

			// get the GribData object for the current level or create
			// a new one if it doesn't exist.
			if (param_cat == 2 && param_num == 2 && strncmp(levelType,"isobaricInhPa",sz)==0) {
				// handle u data (GRIB2)
				handle_u_data(h, grib_data, level);
			} else if(param_cat == 2 && param_num == 3 && strncmp(levelType,"isobaricInhPa",sz)==0) {
				// handle v data (GRIB2)
				handle_v_data(h, grib_data, level);
			} else if(param_cat == 3 && param_num == 5 && strncmp(levelType,"isobaricInhPa",sz)==0) {
				// handle h data (GRIB2)
				handle_h_data(h, grib_data, level);
			}
		} else {
			// whoops. invalid edition number
			fprintf(stderr, "ERROR: invalid GRIB edition %d for file %s\n",
			  grib_ed, grib_file.c_str());

			if (h) {
				grib_handle_delete(h);
			}

			if (f) {
			  fclose(f);
			}

			return -1;
		}

		grib_handle_delete(h);
	}

	fclose(f);
	initializeGribData(grib_data);

	return 0;
}

/**
 * Load a grib2 file into a GribData object
 */
int load_grib_v2_win(const string& grib_file, GribData* const grib_data) {
    // turn on multi message support
    grib_multi_support_on(0);

    // open the file for reading
    FILE* f = fopen(grib_file.c_str(), "r");
    if (!f) {
        fprintf(stderr, "ERROR: could not open GRIB file %s\n", grib_file.c_str());
        return -1;
    }

    int err = 0;

    int grib_ed = 0;

    long level = -1;

    grib_ed = 2;

    // set the grib edition in the output object
    grib_data->grib_edition = grib_ed;

    long num_values = 0;

    string str_command;

    str_command.assign("wgrib2_windows_x64\\wgrib2 ");
    str_command.append(grib_file);
    str_command.append(" -nxny");

    FILE* wgrib2_input = popen(str_command.c_str(), "r");
    if (wgrib2_input == NULL) exit(2);

    char chunk[256];

    while (fgets(chunk, sizeof(chunk), wgrib2_input) != NULL) {
		int idx_leftParen = indexOf(chunk, "(");
		int idx_rightParen = indexOf(chunk, ")");
		if ((-1 < idx_leftParen) && (-1 < idx_rightParen)) {
			char nx_ny[idx_rightParen - idx_leftParen];
			subString(nx_ny, chunk, idx_leftParen+1, idx_rightParen-idx_leftParen-1);
			nx_ny[idx_rightParen - idx_leftParen - 1] = '\0';

			int idx_signX = indexOf(nx_ny, "x");
			if (-1 < idx_signX) {
				char str_nx[idx_signX+1];
				char str_ny[strlen(nx_ny) - idx_signX];
				subString(str_nx, nx_ny, 0, idx_signX);
				str_nx[idx_signX] = '\0';
				subString(str_ny, nx_ny, idx_signX+1, strlen(nx_ny)-idx_signX-1);
				str_ny[strlen(nx_ny) - idx_signX] = '\0';

				grib_data->grid_ni = atol(str_nx);
				grib_data->grid_nj = atol(str_ny);

				num_values = grib_data->grid_ni * grib_data->grid_nj;

				break;
			}
		}
    }

    /* close the wgrib2 program */
    err = pclose(wgrib2_input);

    if (f) fclose(f);

    level = 100; // Initial value

    while (level <= 1000) {
        handle_u_data_win(grib_data, level, num_values, grib_file);

        handle_v_data_win(grib_data, level, num_values, grib_file);

        handle_h_data_win(grib_data, level, num_values, grib_file);

        level += 25;
    }

    initializeGribData(grib_data);

    return 0;
}


int get_wind_components(const GribData& grib,
			const double& latDeg, 
			const double& lonDeg,
			const double& altFeet,
			double* const windNorth,
			double* const windEast,
			wind_units_e units) {

  if(!windNorth) return -1;
  if(!windEast) return -1;

  double altMeter = altFeet * FeetToMeters;
  
  double xI, xJ;
  
  // Get grib coordinate j, j for the latitude and longitude
  convertGribCoordinate(grib, latDeg, lonDeg, &xI, &xJ);

  double ni = grib.grid_ni;

  int level=0;
  double height0Meter = 0;
  double height1Meter = 0;
  int NumHybridLevels = grib.h_data.size();
  long level_value = 0;
  long prev_level_value = 0;

  // find the level corresponding to this altitude, if altitude is higher
  // than the highest geometric altitude, then use the highest level
  map< long, vector<double> >::const_iterator liter;
  map< long, vector<double> >::const_iterator prev_liter;

  //for(level = 0; level < NumHybridLevels; level++) {
  for(liter=grib.h_data.begin(); liter!=grib.h_data.end(); ++liter) {
    const double* hdata = &(liter->second.at(0));
    double height1 = bilinearInterpolation(xI, xJ, ni, hdata);

    if(height1 < altMeter) {
      level_value = liter->first;
      height1Meter = height1;
      break;
    }
    height0Meter = height1;
    prev_level_value = liter->first;
    level++;
  }
  
  //if(level == NumHybridLevels) level = NumHybridLevels-1;
  if(level >= NumHybridLevels) level = NumHybridLevels-1;
  if(level <= 0) level = 0;
  
  double uWindMeterPerSec;
  double vWindMeterPerSec;

  if (level <= 0) {
	  if ((grib.u_data.find(level_value) == grib.u_data.end())
			  || (grib.v_data.find(level_value) == grib.v_data.end())) {
		  // Some data is missing
		  // Quit this function
		  return -1;
	  } else {
		  const double* udata = &(grib.u_data.at(level_value).at(0));
		  const double* vdata = &(grib.v_data.at(level_value).at(0));
		  uWindMeterPerSec = bilinearInterpolation(xI, xJ, ni, udata);
		  vWindMeterPerSec = bilinearInterpolation(xI, xJ, ni, vdata);
	  }
  } else if (level >= NumHybridLevels) {
	  if ((grib.u_data.find(level_value) == grib.u_data.end())
			  || (grib.v_data.find(level_value) == grib.v_data.end())) {
		  // Some data is missing
		  // Quit this function
		  return -1;
	  } else {
		  const double* udata = &(grib.u_data.at(level_value).at(0));
		  const double* vdata = &(grib.v_data.at(level_value).at(0));
		  uWindMeterPerSec = bilinearInterpolation(xI, xJ, ni, udata);
		  vWindMeterPerSec = bilinearInterpolation(xI, xJ, ni, vdata);
	  }
  } else {
	  if ((grib.u_data.find(level_value) == grib.u_data.end())
			  || (grib.v_data.find(level_value) == grib.v_data.end())
			  || (grib.u_data.find(prev_level_value) == grib.u_data.end())
			  || (grib.v_data.find(prev_level_value) == grib.v_data.end())) {
		  // Some data is missing
		  // Quit this function
		  return -1;
	  } else {
		  const double* udata_lo = &(grib.u_data.at(level_value).at(0));
		  const double* vdata_lo = &(grib.v_data.at(level_value).at(0));
		  const double* udata_hi = &(grib.u_data.at(prev_level_value).at(0));
		  const double* vdata_hi = &(grib.v_data.at(prev_level_value).at(0));
		  double u_lo = bilinearInterpolation(xI, xJ, ni, udata_lo);
		  double v_lo = bilinearInterpolation(xI, xJ, ni, vdata_lo);
		  double u_hi = bilinearInterpolation(xI, xJ, ni, udata_hi);
		  double v_hi = bilinearInterpolation(xI, xJ, ni, vdata_hi);
		  uWindMeterPerSec = u_lo + (u_hi - u_lo) / (height0Meter - height1Meter) * (altMeter - height1Meter);
		  vWindMeterPerSec = v_lo + (v_hi - v_lo) / (height0Meter - height1Meter) * (altMeter - height1Meter);
	  }
  }
  
  convertToTrueNorth(lonDeg, &uWindMeterPerSec, &vWindMeterPerSec);
  
  // Set wind value on North and East direction
  if(units == FEET_PER_SEC) { // unit is feet per sec
    *windNorth = vWindMeterPerSec * MetersToFeet; // v component of the wind
    *windEast = uWindMeterPerSec * MetersToFeet; // u component of the wind
  } else if(units == KNOTS) {
    *windNorth = vWindMeterPerSec * MetersPerSecToKnots; // v component of the wind
    *windEast = uWindMeterPerSec * MetersPerSecToKnots; // u component of the wind
  } else { // unit is meter per sec
    *windNorth = vWindMeterPerSec; // v component of the wind
    *windEast = uWindMeterPerSec; // u component of the wind
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
// the following functions are NOT thread safe...
#if 0
// Read the grib2 file and get the u and v components of the wind
void getRUCWindValues(char rucFilename[MAX_FILENAME_LEN])
{

    printf("Reading RUC Wind Data\n");
    int err = 0;
    size_t values_len= 0;

    long parameterNumber=0, parameterCategory=0, level=0;

    FILE* grib_file = NULL;
    grib_handle *grib_h = NULL;

    // turn on support for multi fields messages
    grib_multi_support_on(0);

    grib_file = fopen(rucFilename,"r");
    if(!grib_file) {
        printf("ERROR: unable to open file %s\n",rucFilename);
        return;
    }
    
    printf("Opening RUC File: %s\n", rucFilename);

    while ((grib_h = grib_handle_new_from_file(0,grib_file,&err)) != NULL)
    {

        GRIB_CHECK(err,0);

        GRIB_CHECK(grib_get_long(grib_h,"parameterNumber",&parameterNumber),0);

        GRIB_CHECK(grib_get_long(grib_h,"parameterCategory",&parameterCategory),0);

        GRIB_CHECK(grib_get_long(grib_h,"level",&level),0);

        //U-COMPONENT OF THE WIND
        if(parameterCategory == 2 && parameterNumber == 2) {

            // *get the size of the values array* //
            GRIB_CHECK(grib_get_size(grib_h,"values",&values_len),0);

            // *get data values* //
            GRIB_CHECK(grib_get_double_array(grib_h,"values",uWindArrayMeterPerSec[level-1],&values_len),0);
            
        }

        //V-COMPONENT OF THE WIND
        if(parameterCategory == 2 && parameterNumber == 3) {

            // *get the size of the values array* //
            GRIB_CHECK(grib_get_size(grib_h,"values",&values_len),0);

            // *get data values* //
            GRIB_CHECK(grib_get_double_array(grib_h,"values",vWindArrayMeterPerSec[level-1],&values_len),0);

            if(level == 50) break;
        }

        //GEOPOTENTIAL HEIGHT
        if(parameterCategory == 3 && parameterNumber == 5) {

            // *get the size of the values array* //
            GRIB_CHECK(grib_get_size(grib_h,"values",&values_len),0);

            // *get data values* //
            GRIB_CHECK(grib_get_double_array(grib_h,"values",geometricAltitudeMeter[level-1],&values_len),0);
	

        }
        //TEMERATURE
        if(parameterCategory == 0 && parameterNumber == 0) {

            // *get the size of the values array* //
            GRIB_CHECK(grib_get_size(grib_h,"values",&values_len),0);

            // *get data values* //
            GRIB_CHECK(grib_get_double_array(grib_h,"values",temperatureArrayK[level-1],&values_len),0);
        }

        //PRESSURE
        if(parameterCategory == 3 && parameterNumber == 0) {

            // *get the size of the values array* //
            GRIB_CHECK(grib_get_size(grib_h,"values",&values_len),0);

            // *get data values* //
            GRIB_CHECK(grib_get_double_array(grib_h,"values",pressureArrayPa[level-1],&values_len),0);
        }


    }

    grib_handle_delete(grib_h);
    fclose(grib_file);
    initializeRUCData();
}

void getWindFieldComponents(double latDeg, double lonDeg, double altFeet, double *windNorth, double *windEast, bool feetPerSec) {
    
    double altMeter = altFeet * FeetToMeters;

    double xI, xJ;

    convertRucCoordinate (latDeg, lonDeg, &xI, &xJ);

    int level;
    double height0Meter = 0;
    double height1Meter;

	// find the level corresponding to this altitude, if altitude is higher
	// than the highest geometric altitude, then use the highest level
    for(level = 0; level < NumHybridLevels; level++) {
        double height1 = bilinearInterpolation(xI, xJ, geometricAltitudeMeter[level]);
        if(height1 > altMeter) {
	    height1Meter = height1;
            break;
	}
        height0Meter = height1;
    }

    //if(level == NumHybridLevels) level = NumHybridLevels-1;

    double uWindMeterPerSec;
    double vWindMeterPerSec;

    if(level == NumHybridLevels) {
        uWindMeterPerSec = bilinearInterpolation (xI, xJ, uWindArrayMeterPerSec[NumHybridLevels-1]);
        vWindMeterPerSec = bilinearInterpolation (xI, xJ, vWindArrayMeterPerSec[NumHybridLevels-1]);
    } else if(level==0){
	uWindMeterPerSec = bilinearInterpolation (xI, xJ, uWindArrayMeterPerSec[0]);
        vWindMeterPerSec = bilinearInterpolation (xI, xJ, vWindArrayMeterPerSec[0]);
    } else {
        double uWind0MeterPerSec = bilinearInterpolation (xI, xJ, uWindArrayMeterPerSec[level-1]);
        double vWind0MeterPerSec = bilinearInterpolation (xI, xJ, vWindArrayMeterPerSec[level-1]);

        double uWind1MeterPerSec = bilinearInterpolation (xI, xJ, uWindArrayMeterPerSec[level]);
        double vWind1MeterPerSec = bilinearInterpolation (xI, xJ, vWindArrayMeterPerSec[level]);

		// do bilinear interpolation between the 2 levels
        uWindMeterPerSec = uWind0MeterPerSec + (uWind1MeterPerSec - uWind0MeterPerSec) / (height1Meter - height0Meter) * (altMeter - height0Meter);
        vWindMeterPerSec = vWind0MeterPerSec + (vWind1MeterPerSec - vWind0MeterPerSec) / (height1Meter - height0Meter) * (altMeter - height0Meter);

    }
  
    convertToTrueNorth(lonDeg, &uWindMeterPerSec, &vWindMeterPerSec);

    if(feetPerSec) { // unit is feet per sec
        *windNorth = vWindMeterPerSec * MetersToFeet; // v component of the wind
        *windEast = uWindMeterPerSec * MetersToFeet; // u component of the wind
    } else { // unit is meter per sec
        *windNorth = vWindMeterPerSec; // v component of the wind
        *windEast = uWindMeterPerSec; // u component of the wind
    }
}

void getTemperatureComponents(double latDeg, double lonDeg, double altFeet, double *temperatureK) {
    
    double altMeter = altFeet * FeetToMeters;

    double xI, xJ;

    convertRucCoordinate (latDeg, lonDeg, &xI, &xJ);

    int level;
    double height0Meter = 0;
    double height1Meter;

	// find the level corresponding to this altitude, if altitude is higher
	// than the highest geometric altitude, then use the highest level
	/*   height1Meter
               :
 	     altMeter
	       :	
	     height0Meter
  	*/
    for(level = 0; level < NumHybridLevels; level++) {
        double height1 = bilinearInterpolation(xI, xJ, geometricAltitudeMeter[level]);
        if(height1 > altMeter) {
			height1Meter = height1;
            break;
		}
        height0Meter = height1;
    }

    //if(level == NumHybridLevels) level = NumHybridLevels-1;
    if(level == NumHybridLevels) {
        *temperatureK = bilinearInterpolation (xI, xJ, temperatureArrayK[NumHybridLevels-1]);
    } else if( level == 0 ) {
	*temperatureK = bilinearInterpolation (xI, xJ, temperatureArrayK[0]);  
    } else {
       double temperature0K = bilinearInterpolation (xI, xJ, temperatureArrayK[level-1]);
       double temperature1K = bilinearInterpolation (xI, xJ, temperatureArrayK[level]);

	// do bilinear interpolation between the 2 levels
        *temperatureK = temperature0K + (temperature1K - temperature0K) / (height1Meter - height0Meter) * (altMeter - height0Meter);
        

    }

}

void getPressureComponents(double latDeg, double lonDeg, double altFeet, double *pressurePa) {
    
    double altMeter = altFeet * FeetToMeters;

    double xI, xJ;

    convertRucCoordinate (latDeg, lonDeg, &xI, &xJ);

    int level;
    double height0Meter = 0;
    double height1Meter;

  // find the level corresponding to this altitude, if altitude is higher
  // than the highest geometric altitude, then use the highest level
  /*   height1Meter
           :
	altMeter
           :	
       height0Meter
  */
    for(level = 0; level < NumHybridLevels; level++) {
        double height1 = bilinearInterpolation(xI, xJ, geometricAltitudeMeter[level]);
        if(height1 > altMeter) {
			height1Meter = height1;
            break;
		}
        height0Meter = height1;
    }

    //if(level == NumHybridLevels) level = NumHybridLevels-1;
    if(level == NumHybridLevels) {
        *pressurePa = bilinearInterpolation (xI, xJ, pressureArrayPa[NumHybridLevels-1]);
    } else if( level == 0 ) {
	*pressurePa = bilinearInterpolation (xI, xJ, pressureArrayPa[0]);  
    } else {
       double pressure0Pa = bilinearInterpolation (xI, xJ, pressureArrayPa[level-1]);
       double pressure1Pa = bilinearInterpolation (xI, xJ, pressureArrayPa[level]);

	// do bilinear interpolation between the 2 levels
        *pressurePa = pressure0Pa + (pressure1Pa - pressure0Pa) / (height1Meter - height0Meter) * (altMeter - height0Meter);
        

    }

}


// Initialize the global variables -- need to only do it once
void initializeRUCData()
{
    double alatn1 = alatan*DegToRad;
    double elon1l;
    double ala1 = alat1*DegToRad;
    double rmll;
    double elo1;
    double arg;

    // hemisphere = 1 for northern hemisphere; = -1 for southern
    if (alatan > 0)
        hemisphere = 1;
    else
        hemisphere = -1;

    an = hemisphere * sin(alatn1);
    cosltn = cos(alatn1);

    elonvr = elonv*DegToRad;

    elon1l = elon1;
    if ((elon1 - elonv) > 180)
        elon1l = elon1 - 360;
    if ((elon1 - elonv) < 180)
        elon1l = elon1 + 360;

    rmll = rEarthByDx * (pow(cosltn, 1-an) * pow(1+an, an)) * pow((cos(ala1)/(1+hemisphere*sin(ala1))), an) / an;

    elo1 = elon1l * DegToRad;
    arg = an * (elo1 - elonvr);
    poleI = 1 - hemisphere * rmll * sin(arg);
    poleJ = 1 + rmll * cos(arg);   
}


// Converts lat and lon to its corresponding grid coordinate (xI and xJ)
// where i represents latitude and j represents longitude
void convertRucCoordinate (double alat, double elon, double* xI, double* xJ)
{
    double elonl;
    double ala;
    double rm;
    double elo;
    double arg;

    elonl = elon;
    if ((elon - elonv) > 180)
        elonl = elon - 360;
    if ((elon - elonv) < 180)
        elonl = elon + 360;

    ala = alat * DegToRad;
    rm = rEarthByDx * (pow(cosltn, 1-an) * pow(1+an, an)) * pow((cos(ala)/(1+hemisphere*sin(ala))), an) / an;

    elo = elonl * DegToRad;

    arg = an * (elo - elonvr);

    *xI = poleI + hemisphere * rm * sin(arg);
    *xJ = poleJ - rm * cos(arg);

    if (*xI < 1) {
        *xI = 1;
        //printf("xI is out of the border!!!\n");
    }
    if (*xJ < 1) {
        *xJ = 1;
        //printf("xJ is out of the border!!!\n");
    }
    if (*xJ > 337)
        *xJ = 337;
    //if (*xI > 451 && *xI < 452)
    if (*xI > 451)
        *xI = 451;
}
#endif /* 0 */


/*
********************************************************************************
 COMPUTATIONAL APPLIANCE FOR RAPID PREDICTION OF AIRCRAFT TRAJECTORIES (CARPAT)
          Copyright 2010 by Optimal Synthesis Inc. All rights reserved



SBIR Rights (FAR 52.227-20) Notice: Contract No. NNX11CA08C, Dated June 1, 2011.
                       Contract End Date May 31, 2013
                   Software Release Date August 16, 2011

For a period of 4 years after acceptance of all items to be delivered under this
contract, the Government agrees to use these data for Government purposes only,
and they shall not be disclosed outside the Government (including disclosure for
procurement purposes) during such period without permission of the Contractor,
except that, subject to the foregoing use and disclosure prohibitions, such data
may be disclosed for use by support Contractors.  After the aforesaid 4-year
period the Government has a royalty-free license to use, and to authorize others
to use on its behalf, these data for Government purposes, but is relieved of all
disclosure prohibitions and assumes no liability for unauthorized use of these
data by third parties.  This Notice shall be affixed to any reproductions of
these data, in whole or in part.
********************************************************************************
*/
