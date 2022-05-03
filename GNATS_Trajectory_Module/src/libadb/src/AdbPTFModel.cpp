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
 * AdbPTFModel.cpp
 *
 *  Created on: Jan 24, 2013
 *      Author: jason
 *
 *  Update: 02.03, 2020
 *          Oliver Chen
 */

#include "AdbPTFModel.h"

#include <map>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <stdio.h>
#include "AdbIntegration.h"

using std::map;
using std::pair;
using std::cout;
using std::endl;
using std::lower_bound;

namespace osi {

  const double AdbPTFModel::KNOTS_TO_FEET_PER_MIN = 1.68780986*60.;
  const double AdbPTFModel::UNSET_VALUE = -9999999;

  AdbPTFModel::AdbPTFModel() :
    actype(""),
    climbCasLow(0),
    climbCasHi(0),
    climbMach(0),
    cruiseCasLow(0),
    cruiseCasHi(0),
    cruiseMach(0),
    descentCasLow(0),
    descentCasHi(0),
    descentMach(0),
    massLow(0),
    massNom(0),
    massHi(0),
    maxAltitude(0),
    altitudes(vector<double>()),
    cruiseTas(map<double, double>()),
    cruiseFuelFlowLow(map<double, double>()),
    cruiseFuelFlowNom(map<double, double>()),
    cruiseFuelFlowHi(map<double, double>()),
    climbTas(map<double, double>()),
    climbRateLow(map<double, double>()),
    climbRateNom(map<double, double>()),
    climbRateHi(map<double, double>()),
    climbFuelFlowNom(map<double, double>()),
    descentTas(map<double, double>()),
    descentRateNom(map<double, double>()),
    descentFuelFlowNom(map<double, double>()),
    descentDistNom(map<double, double>()),
    descentTimeNom(map<double, double>()),
    climbDistNom(map<double, double>()),
    climbTimeNom(map<double, double>()),
    cumulativeDescentDistNom(map<double, double>()),
    cumulativeDescentTimeNom(map<double, double>()),
    cumulativeClimbDistNom(map<double, double>()),
    cumulativeClimbTimeNom(map<double, double>()) {
  }

  AdbPTFModel::AdbPTFModel(const AdbPTFModel& that) :
    	    actype(that.actype),
    	    climbCasLow(that.climbCasLow),
    	    climbCasHi(that.climbCasHi),
    	    climbMach(that.climbMach),
    	    cruiseCasLow(that.cruiseCasLow),
    	    cruiseCasHi(that.cruiseCasHi),
    	    cruiseMach(that.cruiseMach),
    	    descentCasLow(that.descentCasLow),
    	    descentCasHi(that.descentCasHi),
    	    descentMach(that.descentMach),
    	    massLow(that.massLow),
    	    massNom(that.massNom),
    	    massHi(that.massHi),
    	    maxAltitude(that.maxAltitude),
    	    altitudes(vector<double>()),
    	    cruiseTas(map<double, double>()),
    	    cruiseFuelFlowLow(map<double, double>()),
    	    cruiseFuelFlowNom(map<double, double>()),
    	    cruiseFuelFlowHi(map<double, double>()),
    	    climbTas(map<double, double>()),
    	    climbRateLow(map<double, double>()),
    	    climbRateNom(map<double, double>()),
    	    climbRateHi(map<double, double>()),
    	    climbFuelFlowNom(map<double, double>()),
    	    descentTas(map<double, double>()),
    	    descentRateNom(map<double, double>()),
    	    descentFuelFlowNom(map<double, double>()),
    	    descentDistNom(map<double, double>()),
    	    descentTimeNom(map<double, double>()),
    	    climbDistNom(map<double, double>()),
    	    climbTimeNom(map<double, double>()),
    	    cumulativeDescentDistNom(map<double, double>()),
    	    cumulativeDescentTimeNom(map<double, double>()),
    	    cumulativeClimbDistNom(map<double, double>()),
    	    cumulativeClimbTimeNom(map<double, double>()) {

	  altitudes.insert(altitudes.begin(), that.altitudes.begin(), that.altitudes.end());

	  cruiseTas.insert(that.cruiseTas.begin(), that.cruiseTas.end());
	  cruiseFuelFlowLow.insert(that.cruiseFuelFlowLow.begin(), that.cruiseFuelFlowLow.end());
	  cruiseFuelFlowNom.insert(that.cruiseFuelFlowNom.begin(), that.cruiseFuelFlowNom.end());
	  cruiseFuelFlowHi.insert(that.cruiseFuelFlowHi.begin(), that.cruiseFuelFlowHi.end());

	  climbTas.insert(that.climbTas.begin(), that.climbTas.end());
	  climbRateLow.insert(that.climbRateLow.begin(), that.climbRateLow.end());
	  climbRateNom.insert(that.climbRateNom.begin(), that.climbRateNom.end());
	  climbRateHi.insert(that.climbRateHi.begin(), that.climbRateHi.end());
	  climbFuelFlowNom.insert(that.climbFuelFlowNom.begin(), that.climbFuelFlowNom.end());

	  descentTas.insert(that.descentTas.begin(), that.descentTas.end());
	  descentRateNom.insert(that.descentRateNom.begin(), that.descentRateNom.end());
	  descentFuelFlowNom.insert(that.descentFuelFlowNom.begin(), that.descentFuelFlowNom.end());

	  descentDistNom.insert(that.descentDistNom.begin(), that.descentDistNom.end());
	  descentTimeNom.insert(that.descentTimeNom.begin(), that.descentTimeNom.end());
	  climbDistNom.insert(that.climbDistNom.begin(), that.climbDistNom.end());
	  climbTimeNom.insert(that.climbTimeNom.begin(), that.climbTimeNom.end());
	  cumulativeDescentDistNom.insert(that.cumulativeDescentDistNom.begin(), that.cumulativeDescentDistNom.end());
	  cumulativeDescentTimeNom.insert(that.cumulativeDescentTimeNom.begin(), that.cumulativeDescentTimeNom.end());
	  cumulativeClimbDistNom.insert(that.cumulativeClimbDistNom.begin(), that.cumulativeClimbDistNom.end());
	  cumulativeClimbTimeNom.insert(that.cumulativeClimbTimeNom.begin(), that.cumulativeClimbTimeNom.end());
  }

  AdbPTFModel::~AdbPTFModel() {
  }

  void AdbPTFModel::generateAuxiliaryTables() {
    generateDescentDistTable();
    generateClimbDistTable();
    generateDescentTimeTable();
    generateClimbTimeTable();
  }

  int AdbPTFModel::getNumRows() const {
    return altitudes.size();
  }

  double AdbPTFModel::getAltitude(const int row) const {
    return altitudes.at(row);
  }

  double AdbPTFModel::getCruiseTas(const double alt) const {
    return getTableValue(alt, cruiseTas);
  }

  double AdbPTFModel::getCruiseFuelFlow(const double alt,
					 const adb_mass_e mass) const {
    if(mass == LOW) {
      return getTableValue(alt, cruiseFuelFlowLow);
    } else if(mass == NOMINAL) {
      return getTableValue(alt, cruiseFuelFlowNom);
    } else {
      return getTableValue(alt, cruiseFuelFlowHi);
    }
  }

  double AdbPTFModel::getClimbTas(const double alt) const {
    return getTableValue(alt, climbTas);
  }

  double AdbPTFModel::getClimbRate(const double alt,
				    const adb_mass_e mass) const {
    if(mass == LOW) {
      return getTableValue(alt, climbRateLow);
    } else if(mass == NOMINAL) {
      return getTableValue(alt, climbRateNom);
    } else {
      return getTableValue(alt, climbRateHi);
    }
  }

  double AdbPTFModel::getClimbFuelFlow(const double alt) const {
    return getTableValue(alt, climbFuelFlowNom);
  }

  double AdbPTFModel::getDescentTas(const double alt) const {
    return getTableValue(alt, descentTas);
  }

  double AdbPTFModel::getDescentRate(const double alt,
				      const adb_mass_e mass) const {
    // only have nominal descent rate
    (void)mass;
    return getTableValue(alt, descentRateNom);
  }

  double AdbPTFModel::getDescentFuelFlow(const double alt) const {
    return getTableValue(alt, descentFuelFlowNom);
  }

  double AdbPTFModel::getDescentDistance(const double alt) const {
    return getTableValue(alt, descentDistNom);
  }

  double AdbPTFModel::getClimbDistance(const double alt) const {
    return getTableValue(alt, climbDistNom);
  }

  double AdbPTFModel::getDescentTime(const double alt) const {
    return getTableValue(alt, descentTimeNom);
  }

  double AdbPTFModel::getClimbTime(const double alt) const {
    return getTableValue(alt, climbTimeNom);
  }

  double AdbPTFModel::getCumulativeDescentDistance(const double alt) const {
	  return getTableValue(alt, cumulativeDescentDistNom);
  }

  double AdbPTFModel::getCumulativeDescentTime(const double alt) const {
	  return getTableValue(alt, cumulativeDescentTimeNom);
  }

  double AdbPTFModel::getCumulativeClimbDistance(const double alt) const {
	  return getTableValue(alt, cumulativeClimbDistNom);
  }

  double AdbPTFModel::getCumulativeClimbTime(const double alt) const {
	  return getTableValue(alt, cumulativeClimbTimeNom);
  }


  ////////////////////////////////////////////////////////////////////////////
  // Private Impl.

  void AdbPTFModel::generateDescentDistTable() {
    descentDistNom.insert(pair<double, double>(0., 0.));
    cumulativeDescentDistNom.insert(pair<double, double>(0., 0.));

    double dx = 0;
    if (altitudes.size() > 1) {
		for(unsigned int i = 1; i < altitudes.size(); ++i) {
		  double alt0 = altitudes.at(i-1);
		  double alt1 = altitudes.at(i);

		  // chop the interval between alt0 and alt1 into n=100 segments
		  int n = (int)(ceil(alt1-alt0)/100.);
		  double dh = (alt1-alt0)/(double)n;
		  double a=0.0, b=0.0, mid=0.0;
		  double f_a=0.0, f_b=0.0, f_mid=0.0;
		  double step_dx = 0.0;
		  for(int j=0; j<n; ++j) {
			a = alt0 + dh*j;     // ft
			b = alt0 + dh*(j+1); // ft
			mid = (a+b)/2;       // ft
			f_a = AdbIntegration::xdotVsRod(a, *this);
			f_b = AdbIntegration::xdotVsRod(b, *this);
			f_mid = AdbIntegration::xdotVsRod(mid, *this);
			step_dx += AdbIntegration::simpsons(a, b, f_a, f_mid, f_b);
		  }
		  dx += step_dx;
		  descentDistNom.insert(pair<double, double>(alt1, step_dx));
		  cumulativeDescentDistNom.insert(pair<double, double>(alt1, dx));
		}
    }
  }

  void AdbPTFModel::generateClimbDistTable() {
    climbDistNom.insert(pair<double, double>(0., 0.));
    cumulativeClimbDistNom.insert(pair<double, double>(0., 0.));

    double dx = 0.0;
    for(unsigned int i=1; i<altitudes.size(); ++i) {
      double alt0 = altitudes.at(i-1);
      double alt1 = altitudes.at(i);

      // chop thie interval between alt0 and alt1 into n=100 segments
      int n = (int)(ceil(alt1-alt0)/100);
      double dh = (alt1-alt0)/n;
      double a=0.0, b=0.0, mid=0.0;
      double f_a=0.0, f_b=0.0, f_mid=0.0;
      double step_dx = 0.0;
      for(int j=0; j<n; ++j) {
		a = alt0 + dh*j;     // ft
		b = alt0 + dh*(j+1); // ft
		mid = (a+b)/2;       // ft
		f_a = AdbIntegration::xdotVsRoc(a, *this);
		f_b = AdbIntegration::xdotVsRoc(b, *this);
		f_mid = AdbIntegration::xdotVsRoc(mid, *this);
		step_dx += AdbIntegration::simpsons(a, b, f_a, f_mid, f_b);
      }
      dx += step_dx;
      climbDistNom.insert(pair<double, double>(alt1, step_dx));
      cumulativeClimbDistNom.insert(pair<double, double>(alt1, dx));
    }    
  }

  void AdbPTFModel::generateDescentTimeTable() {
    descentTimeNom.insert(pair<double, double>(0., 0.));
    cumulativeDescentTimeNom.insert(pair<double, double>(0., 0.));

    double cum_dt = 0.0;
    for(unsigned int i=1; i<altitudes.size(); ++i) {
      double alt0 = altitudes.at(i-1);
      double alt1 = altitudes.at(i);

      // chop thie interval between alt0 and alt1 into n=100 segments
      int n = (int)(ceil(alt1-alt0)/100);
      double dh = (alt1-alt0)/n;
      double a=0.0, b=0.0, mid=0.0;
      double f_a=0.0, f_b=0.0, f_mid=0.0;
      double dt = 0.0;
      for(int j=0; j<n; ++j) {
		a = alt0 + dh*j;     // ft
		b = alt0 + dh*(j+1); // ft
		mid = (a+b)/2;       // ft
		f_a = AdbIntegration::invRod(a, *this);
		f_b = AdbIntegration::invRod(b, *this);
		f_mid = AdbIntegration::invRod(mid, *this);
		dt += AdbIntegration::simpsons(a, b, f_a, f_mid, f_b);
      }
      cum_dt += dt;
      descentTimeNom.insert(pair<double, double>(alt1, dt));
      cumulativeDescentTimeNom.insert(pair<double, double>(alt1, cum_dt));
    }
  }

  void AdbPTFModel::generateClimbTimeTable() {
    climbTimeNom.insert(pair<double, double>(0., 0.));
    cumulativeClimbTimeNom.insert(pair<double, double>(0., 0.));

    double cum_dt = 0.0;
    for(unsigned int i=1; i<altitudes.size(); ++i) {
      double alt0 = altitudes.at(i-1);
      double alt1 = altitudes.at(i);

      // chop thie interval between alt0 and alt1 into n=100 segments
      int n = (int)(ceil(alt1-alt0)/100);
      double dh = (alt1-alt0)/n;
      double a=0.0, b=0.0, mid=0.0;
      double f_a=0.0, f_b=0.0, f_mid=0.0;
      double dt = 0.0;
      for(int j=0; j<n; ++j) {
		a = alt0 + dh*j;     // ft
		b = alt0 + dh*(j+1); // ft
		mid = (a+b)/2;       // ft
		f_a = AdbIntegration::invRoc(a, *this);
		f_b = AdbIntegration::invRoc(b, *this);
		f_mid = AdbIntegration::invRoc(mid, *this);
		dt += AdbIntegration::simpsons(a, b, f_a, f_mid, f_b);
      }
      cum_dt += dt;
      climbTimeNom.insert(pair<double, double>(alt1, dt));
      cumulativeClimbTimeNom.insert(pair<double, double>(alt1, cum_dt));
    }
  }

  double AdbPTFModel::interpolate(const double& x,
				   const double& xLo,
				   const double& xHi,
				   const double& yLo,
				   const double& yHi) const {
    double y = UNSET_VALUE;
    if((x != UNSET_VALUE) &&
       (xLo != UNSET_VALUE) &&
       (xHi != UNSET_VALUE) &&
       (yLo != UNSET_VALUE) &&
       (yHi != UNSET_VALUE)) {
      // point-slope equation of line:
      // y-y1 = m(x-x1), m=(y2-y1)/(x2-x1)
      // y = (x-x1)*(y2-y1)/(x2-x1) + y1
      y = (x-xLo)*(yHi-yLo)/(xHi-xLo) + yLo;
    }
    return y;
  }

double AdbPTFModel::getTableValue(const double& h,
		const map<double, double>& table) const {

	double alt = h;
	if (alt > maxAltitude) {
		alt = maxAltitude;
	}
	if (alt < 0) {
		// throw exception
		alt = 0;
	}
	double retval = UNSET_VALUE;

	map<double, double>::const_iterator iter;
	if (table.size() > 0) {
		iter = table.find(alt);
		if (iter != table.end()) {
			// table contains the exact value so return the value
			// at the given altitude row
			retval = iter->second;
		} else {
			// obtain non-const iterators to the altitude vector
			// because std::lower_bound won't work with const_iterator
			AdbPTFModel* ptr = const_cast<AdbPTFModel*>(this);
			vector<double>::iterator begin = ptr->altitudes.begin();
			vector<double>::iterator end = ptr->altitudes.end();

			// iterpolate.
			// std::lower_bound returns an iterator that points to the first
			// element in the sorted range [first,last) which does not
			// compare LESS THAN target value using the operator<.
			// this will point to first occurrance of the target value,
			// or the position it would be inserted.
			int rowUpper, rowLower;
			vector<double>::iterator riter;
			riter = lower_bound(begin, end, h);
			rowUpper = riter - begin;

			if (rowUpper >= (int) altitudes.size())
				rowUpper = altitudes.size() - 1;

			rowLower = (rowUpper == 0 ? 0 : rowUpper - 1);

			double xHi = 0.0, xLo = 0.0, yHi= 0.0, yLo = 0.0;
			xHi = altitudes.at(rowUpper);

			try {
				if (table.find(xHi) != table.end()) {
					yHi = const_cast<map<double, double>&>(table)[xHi];
				} else {
					cout << "ERROR (case A): could not find adb table value for altitude "
							<< xHi << endl;
					return retval;
				}
			} catch (int e) {
				cout << "ERROR (case B): could not find adb table value for altitude "
						<< xHi << endl;
				return retval;
			}

			xLo = altitudes.at(rowLower);
			try {
				if (table.find(xLo) != table.end()) {
					yLo = const_cast<map<double, double>&>(table)[xLo];
				} else {
					cout << "ERROR (case C): could not find adb table value for altitude "
							<< xLo << endl;
					return retval;
				}
			} catch (int e) {
				cout << "ERROR (case D): could not find adb table value for altitude "
						<< xLo << endl;
				return retval;
			}

			retval = interpolate(alt, xLo, xHi, yLo, yHi);
		}
	}
	return retval;
}

size_t AdbPTFModel::size() const {
	int n = getNumRows();
	int map_double_double_size = n*2*sizeof(double);
	int vector_double_size = n*sizeof(double);

	size_t bytes = sizeof(AdbPTFModel) + 20*map_double_double_size +
			vector_double_size;

	return bytes;
}

} /* namespace osi */
