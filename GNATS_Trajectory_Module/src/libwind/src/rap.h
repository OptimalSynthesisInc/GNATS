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

#ifndef _RAP_H
#define	_RAP_H

#include <map>
#include <vector>
#include <string>

using namespace std;

typedef enum _wind_units_e {
  METERS_PER_SEC=0,
  FEET_PER_SEC,
  KNOTS
} wind_units_e;

/*
 * struct to hold raw grib data
 */
class GribData {
public:
  GribData() :
    grib_edition(0),
    grid_ni(0),
    grid_nj(0),
    h_data(map< long, vector<double> >()),
    u_data(map< long, vector<double> >()),
    v_data(map< long, vector<double> >()),
    t_data(map< long, vector<double> >()),
    p_data(map< long, vector<double> >()),
    hemisphere(0),
    an(0),
    elonvr(0),
    cosltn(0),
    poleI(0),
    poleJ(0),
    wind_north(NULL),
    wind_east(NULL) {
  }
  GribData(const GribData& that) :
    grib_edition(that.grib_edition),
    grid_ni(that.grid_ni),
    grid_nj(that.grid_nj),
    h_data(that.h_data),
    u_data(that.u_data),
    v_data(that.v_data),
    t_data(that.t_data),
    p_data(that.p_data),
    hemisphere(that.hemisphere),
    an(that.an),
    elonvr(that.elonvr),
    cosltn(that.cosltn),
    poleI(that.poleI),
    poleJ(that.poleJ),
    wind_north(that.wind_north),
    wind_east(that.wind_east) {
  }
  virtual ~GribData() {
  }

  // grib edition
  int grib_edition;

  // grid dimensions
  int grid_ni;
  int grid_nj;

  // key: hybrid level, value: array of values at level
  map< long, vector<double> > h_data; // geopotential height [m]
  map< long, vector<double> > u_data; // u-wind data [m/s]
  map< long, vector<double> > v_data; // v-wind data [m/s]
  map< long, vector<double> > t_data; // temp data [K]
  map< long, vector<double> > p_data; // pressure data [Pa]

  // grid data for coordinate conversion
  int hemisphere;
  double an;
  double elonvr;
  double cosltn;
  double poleI;
  double poleJ;

  double* wind_north;
  double* wind_east;
};

#include "conversionFactors.h"
#include "grib_api.h"

int load_grib(const string& grib_file, GribData* const grib_data);

int load_grib_v2_win(const string& grib_file, GribData* const grib_data);

int get_wind_components(const GribData& grib,
			const double& latDeg, 
			const double& lonDeg,
			const double& altFeet,
			double* const windNorth,
			double* const windEast,
			wind_units_e units=METERS_PER_SEC);






// deprecated
/*
#if 0
    void initializeRUCData(); // Initialize the global variables and read the grib2 file
    void getRUCWindValues(char rucFilename[MAX_FILENAME_LEN]);

    // Get the wind field components provided latitude(degree), longitude(degree), and altitude(Feet)
    // the boolean is to indicate whether the expected value is in feet/sec (true) or meter/sec (false).
    void getWindFieldComponents(double latDeg, double lonDeg, double altFeet, double *windNorth, double *windEast, bool feetPerSec);

    // Converts lat and lon to its corresponding grid coordinate (xI and xJ)
    // where i represents latitude and j represents longitude
    void convertRucCoordinate (double alat, double elon, double* xI, double* xJ);

    void convertToTrueNorth (double elon, double* uWind, double* vWind);
    void getTemperatureComponents(double latDeg, double lonDeg, double altFeet, double *temperatureK);
    void getPressureComponents(double latDeg, double lonDeg, double altFeet, double *pressurePa);

    double bilinearInterpolation (double i, double j, double* arr);
#endif
*/


#endif	/* _RAP_H */

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
