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

#ifndef __WIND_API_H__
#define __WIND_API_H__

/*
 * This module provides functions for reading/writing wind files
 * stored in HDF5 format created using the wind_tool
 */


#include <string>
#include <vector>

#include "hdf5.h"

using namespace std;

/*
 * TODO:PARIKSHIT ADDER
 */
#ifndef FeetToMeters
#define FeetToMeters 0.3048
#endif

#ifndef KnotsToFeetPerSec
#define KnotsToFeetPerSec 1.68780986
#endif

/*
 * TODO:PARIKSHIT ADDER ENDS
 */

class WindGrid {
public:
	WindGrid();
	WindGrid(const double& lat_min,
	         const double& lat_max,
	         const double& lat_step,
	         const double& lon_min,
	         const double& lon_max,
	         const double& lon_step,
	         const double& alt_min,
	         const double& alt_max,
	         const double& alt_step);
	WindGrid(const WindGrid& that);
	virtual ~WindGrid();

	double lat_min;
	double lat_max;
	double lat_step;
	double lon_min;
	double lon_max;
	double lon_step;
	double alt_min;
	double alt_max;
	double alt_step;
    double* n_data;
    double* e_data;
    size_t lat_size;
    size_t lon_size;
    size_t alt_size;
    size_t table_size;

	void setGrid(const double& lat_min,
                 const double& lat_max,
                 const double& lat_step,
                 const double& lon_min,
                 const double& lon_max,
                 const double& lon_step,
                 const double& alt_min,
                 const double& alt_max,
                 const double& alt_step,
                 const double* const n_data,
                 const double* const e_data);

	void getWind(const double& lat,
			     const double& lon,
			     const double& alt,
			     double* const wind_north,
			     double* const wind_east);


	int getIndex(const int& i, const int& j, const int& k);

	//TODO:PARIKSHIT ADDER FOR MAXIMUM WIND MAGNITUDE
	double getMaxWind();

};

/*
 * Write a simple HDF5 dataset to the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int write_wind_grid_hdf5(const string& hdf5_file,
                         const WindGrid& grid);

/*
 * TODO: PARIKSHIT'S ADDER FUNCTION.
 * FIXME: IMPLEMENTATION NOT COMPLETED.
 * This does the same thing as the previous function but
 * combines databases from all time steps to one integrated file.
 */
int write_wind_grid_all_hdf5(const hid_t& file_id,
                         const WindGrid& grid);

/*
 * Read a simple HDF5 dataset from the specified file.
 * data is stored C-style row-major.  needs to be transposed
 * if loading into MATLAB/Octave using the built in load() function.
 */
int read_wind_grid_hdf5(const string& hdf5_file,
                        WindGrid* const grid);

/*TODO: PARIKSHIT ADDER
 * Same thing as before but reads from a directory and
 * for all time steps.
 */
int read_wind_grid_all_hdf5(const vector<string>& hdf5_file, vector<WindGrid>* const grid);

#endif  /* __WIND_API_H__ */
