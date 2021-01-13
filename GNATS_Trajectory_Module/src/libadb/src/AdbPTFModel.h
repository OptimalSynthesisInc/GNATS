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
 * AdbPTFModel.h
 *
 *  Created on: Jan 24, 2013
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#ifndef ADBPTFMODEL_H_
#define ADBPTFMODEL_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "adb.h"

using std::string;
using std::map;
using std::vector;
using std::ostream;

namespace osi {

class AdbPTFModel {
public:
  AdbPTFModel();
  AdbPTFModel(const AdbPTFModel& that);
  virtual ~AdbPTFModel();

  string actype;

  double climbCasLow;
  double climbCasHi;
  double climbMach;

  double cruiseCasLow;
  double cruiseCasHi;
  double cruiseMach;

  double descentCasLow;
  double descentCasHi;
  double descentMach;

  double massLow;
  double massNom;
  double massHi;

  double maxAltitude;  // [ft]

  /**
   * List of altitudes from PTF table column
   */
  vector<double> altitudes; // [ft]

  /*
   * Note: STL maps are ordered ascending by key values
   */

  /**
   * Cruise data
   */
  map<double, double> cruiseTas;           // [knots]
  map<double, double> cruiseFuelFlowLow;   // [kg/min]
  map<double, double> cruiseFuelFlowNom;   // [kg/min]
  map<double, double> cruiseFuelFlowHi;    // [kg/min]

  /**
   * Climb data
   */
  map<double, double> climbTas;            // [knots]
  map<double, double> climbRateLow;        // [fpm]
  map<double, double> climbRateNom;        // [fpm]
  map<double, double> climbRateHi;         // [fpm]
  map<double, double> climbFuelFlowNom;    // [kg/min]
  
  /**
   * Descent data
   */
  map<double, double> descentTas;          // [knots]
  map<double, double> descentRateNom;      // [fpm]
  map<double, double> descentFuelFlowNom;  // [kg/min]

  /**
   * Auxiliary data not in the PTF file
   */
  map<double, double> descentDistNom;           // [ft]
  map<double, double> descentTimeNom;           // [min]
  map<double, double> climbDistNom;             // [ft]
  map<double, double> climbTimeNom;             // [min]
  map<double, double> cumulativeDescentDistNom; // [ft]
  map<double, double> cumulativeDescentTimeNom; // [min]
  map<double, double> cumulativeClimbDistNom;   // [ft]
  map<double, double> cumulativeClimbTimeNom;   // [min]

 public:
  void generateAuxiliaryTables();

  int getNumRows() const;
  double getAltitude(const int row) const;
  double getCruiseTas(const double alt) const;
  double getCruiseFuelFlow(const double alt, const adb_mass_e mass=NOMINAL) const;
  double getClimbTas(const double alt) const;
  double getClimbRate(const double alt, const adb_mass_e mass=NOMINAL) const;
  double getClimbFuelFlow(const double alt) const;
  double getDescentTas(const double alt) const;
  double getDescentRate(const double alt, const adb_mass_e mass=NOMINAL) const;
  double getDescentFuelFlow(const double alt) const;
  double getDescentDistance(const double alt) const;
  double getClimbDistance(const double alt) const;
  double getDescentTime(const double alt) const;
  double getClimbTime(const double alt) const;
  double getCumulativeDescentDistance(const double alt) const;
  double getCumulativeDescentTime(const double alt) const;
  double getCumulativeClimbDistance(const double alt) const;
  double getCumulativeClimbTime(const double alt) const;

  static const double UNSET_VALUE;
  static const double KNOTS_TO_FEET_PER_MIN;

  size_t size() const;

 private:
  void generateDescentDistTable();
  void generateClimbDistTable();
  void generateDescentTimeTable();
  void generateClimbTimeTable();

  double interpolate(const double& x,
             const double& xLo,
		     const double& xHi,
		     const double& yLo,
		     const double& yHi) const;

  double getTableValue(const double& h, 
		       const map<double, double>& table) const;
};

}

#endif  /* ADBPTFMODEL_H_ */
